#include "chayes.h"

#ifdef UNITTEST
#include "fcntl.h"
#include "pthread.h"
#include "time.h"
#include "errno.h"
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
    if (init == 0) {
        singleton.shim = shim;

        singleton.urc_hooks = hash_table_new(string_hash, string_equal);
        singleton.parser = NewHayesParser(checker);

        struct mq_attr resp_q_qttr = {.mq_flags = 0,
                                      .mq_maxmsg = 20,
                                      .mq_msgsize = sizeof(parser_result *),
                                      .mq_curmsgs = 0};

        singleton.resp_q = mq_open("/respq",O_WRONLY|O_CREAT,0644, &resp_q_qttr);

        if(singleton.resp_q==-1){
            chayes_debug("mq_open failed, err: %s",strerror(errno));
        }

        if (checker == NULL)
            singleton.checker = &gDefaultChecker;
        else
            singleton.checker = checker;

        init = 1;
    }
    return &singleton;
}

parser_result *send_timeout(control_ctx *self, const char *command,
                            uint64_t timeout) {
    static parser_result *res;

    parser_result *cached_res;
    res = NewParseResult();

    // set inflight_tag
    self->parser->parse_at_req(self->parser, res, command);
    res_read_tag(res, self->inflight_tag, command);
    self->shim.send(command, timeout);

    // poll the resp queue
    struct timespec interval = {
        .tv_sec = timeout % 1000,
        .tv_nsec = (timeout - 1000 * (timeout % 1000)) * 1000000};
    while (1) {
        uint8_t status = mq_timedreceive(self->resp_q, (char *)&res,
                                         sizeof(parser_result *), 0, &interval);
        if (status != 1) {
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
    
    chayes_debug("receive feed: %s\n",buf);

    switch (res->type) {
        case HAYES_RES_STDRESP: {
            res_read_tag(res, tag_buf, buf);
            if (string_equal(tag_buf, self->inflight_tag)) {
                mq_send(self->resp_q, (const char *)&res,
                        sizeof(parser_result *), 0);

                break;
            } else {
                urc_hook hook = hash_table_lookup(self->urc_hooks, tag_buf);
                hook(buf);
                break;
            }
        }
        case HAYES_RES_RESP: {
            if (strlen(self->inflight_tag) > 0)
                mq_send(self->resp_q, (const char *)&res,
                        sizeof(parser_result *), 0);
            else
                break;
        }
        default: {
            int status = mq_send(self->resp_q, (const char *)&res, sizeof(parser_result *),
                    0);
            chayes_debug("send ok to mq, status: %d\n",status);
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

