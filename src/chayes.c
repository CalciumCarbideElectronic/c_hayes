#include "chayes.h"

#ifdef UNITTEST
#include "errno.h"
#include "fcntl.h"
#include "inttypes.h"
#include "pthread.h"
#include "time.h"
#else
#include "FreeRTOS_POSIX/fcntl.h"
#include "FreeRTOS_POSIX/pthread.h"
#include "FreeRTOS_POSIX/time.h"
#endif

#include "compare-string.h"
#include "hash-string.h"
#include "string.h"

control_ctx *NewControlCtx(syscall_shim shim, hayes_checker *checker) {
    static control_ctx singleton;
    static uint8_t init = 0;

    if (init != 0) ControlCtxFree(&singleton);

    singleton.shim = shim;

    singleton.urc_hooks = hash_table_new(string_hash, string_equal);
    singleton.parser = NewHayesParser(checker);

    struct mq_attr attr;
    attr.mq_curmsgs = 0;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 8;
    attr.mq_msgsize = sizeof(parser_result *);

    singleton.resp_q =
        mq_open(CHAYES_QUEUE_NAME, O_RDWR | O_CREAT, 0777, &attr);

    if (singleton.resp_q == -1) {
        chayes_debug("mq_open failed, err: %s\n", strerror(errno));
    }

    if (checker == NULL)
        singleton.checker = &gDefaultChecker;
    else
        singleton.checker = checker;

    init = 1;
    return &singleton;
}

void ControlCtxFree(control_ctx *self) {
    if (self->urc_hooks != NULL) hash_table_free(self->urc_hooks);
    mq_close(self->resp_q);
}

parser_result *send_timeout(control_ctx *self, const char *command,
                            uint64_t timeout) {
    static parser_result *res;
    static mqd_t queue;

    parser_result *cached_res;
    res = NewParseResult();

    // set inflight_tag
    self->inflight_tag[0] = 0;
    self->parser->parse_at_req(self->parser, res, command);
    if (res_read_tag(res, self->inflight_tag) == -1) {
        return NULL;
    }
    self->shim.send(command, timeout);

    // poll the resp queue
    struct timespec ddl;
#ifdef UNITTEST
    clock_gettime(CLOCK_REALTIME, &ddl);
    ddl.tv_nsec += timeout * 1e6;
    if (ddl.tv_nsec >= 1e9 - 1) {
        long sec = ddl.tv_nsec / (long)1e9;
        ddl.tv_nsec -= sec * (long)1e9;
        ddl.tv_sec += sec;
    }
#endif

    while (1) {
        int rescode = mq_timedreceive(self->resp_q, (char *)&res,
                                      sizeof(parser_result *), NULL, &ddl);
        if (rescode == -1) {
            chayes_debug("mq_timedreceive() failed: %s\n", strerror(errno));
            self->inflight_tag[0] = 0;
            return NULL;
        } else {
            switch (res->type) {
                case HAYES_RES_OK: {
                    self->inflight_tag[0] = 0;
                    if (cached_res != NULL) {
                        ParseResultFree(res);
                        return cached_res;
                    } else {
                        return res;
                    }
                }
                case HAYES_RES_ERROR: {
                    self->inflight_tag[0] = 0;
                    return res;
                }
                case HAYES_RES_EMPTY:
                case HAYES_REQ: {
                    break;
                }
                case HAYES_RES_RESP:
                case HAYES_RES_STDRESP: {
                    cached_res = res;
                    break;
                }
            }
        }
    }

    ParseResultFree(res);
    return NULL;
}

void feed(control_ctx *self, const char *buf) {
    static char tag_buf[30];
    static parser_result *res;
    res = NewParseResult();
    self->parser->parse_resp(self->parser, res, buf);

    switch (res->type) {
        case HAYES_RES_STDRESP: {
            res_read_tag(res, tag_buf);
            if (string_equal(tag_buf, self->inflight_tag)) {
                mq_send(self->resp_q, (const char *)&res,
                        sizeof(parser_result *), 0);

                break;
            } else {
                urc_hook hook = hash_table_lookup(self->urc_hooks, tag_buf);
                if (hook != NULL) {
                    hook(buf);
                }
                break;
            }
        }
        case HAYES_RES_RESP: {
            urc_hook hook = hash_table_lookup(self->urc_hooks, "plain");
            if (hook != NULL) {
                hook(buf);
            }
        }
        default: {
            int status = mq_send(self->resp_q, (const char *)&res,
                                 sizeof(parser_result *), 0);
            break;
        }
    }
}

void register_urc_hook(control_ctx *self, const char *urc, urc_hook hook) {
    hash_table_insert(self->urc_hooks, (void *)urc, (void *)hook);
}

void unregister_urc_hook(control_ctx *self, const char *urc) {
    hash_table_remove(self->urc_hooks, (void *)urc);
}

