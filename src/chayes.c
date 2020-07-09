#include "chayes.h"

#ifdef UNITTEST
#include "errno.h"
#include "fcntl.h"
#include "inttypes.h"
#include "pthread.h"
#include "time.h"
#else
#include "FreeRTOS_POSIX/errno.h"
#include "FreeRTOS_POSIX/fcntl.h"
#include "FreeRTOS_POSIX/pthread.h"
#include "FreeRTOS_POSIX/time.h"
#endif

#include "c_core/src/compare-string.h"
#include "c_core/src/hash-string.h"
#include "string.h"

control_ctx *NewControlCtx(syscall_shim shim, hayes_checker *checker) {
    static control_ctx singleton;
    static uint8_t init = 0;
    init_default_checker();

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

void feed(control_ctx *self, const char *buf) {
    static char tag_buf[30];
    static parser_result *tres;
    tres = NewParseResult();
    self->parser->parse_resp(self->parser, tres, buf);

    switch (tres->type) {
        case HAYES_RES_STDRESP: {
            res_read_tag(tres, tag_buf);
            if (string_equal(tag_buf, self->inflight_tag)) {
                //receive response of command sent most recently.
                //It's downstream function, the send_timeout command, should free the res memory.
                mq_send(self->resp_q, (const char *)&tres,
                        sizeof(parser_result *), 0);

                break;
            } else {
                //response with standard structure but not match to the tag, may be an URC.
                urc_hook hook = hash_table_lookup(self->urc_hooks, tag_buf);
                if (hook != NULL) {
                    hook(buf);
                }
                //We do not know whether the hook will free the res or not, just free it again.
                ParseResultFree(tres);
                break;
            }
        }
        case HAYES_RES_RESP: {
            //plain response, use a special URC Hooks.
            urc_hook hook = hash_table_lookup(self->urc_hooks, "plain");
            if (hook != NULL) {
                hook(buf);
            }
            ParseResultFree(tres);
            break;
        }
        default: {
            //Unknown type, just forward to the downstream.
            int status = mq_send(self->resp_q, (const char *)&tres,
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

enum CHayesStatus try_until_ok(control_ctx *ctx, const char *command,
                               uint16_t maxtimes, struct timespec *interval) {
    enum result_type ttype;
    parser_result *tres;
    while (1) {
        if ((maxtimes--) == 0) return CHAYES_TIMEOUT;
        tres = send_timeout(ctx, command, 200);
        if (tres->type == HAYES_RES_OK) return CHAYES_OK;
        nanosleep(interval, NULL);
    }
}


enum CHayesStatus try_until_flag_set(control_ctx *ctx, const char *command,
                               uint16_t maxtimes, struct timespec *interval,uint16_t flag_idx) {
    char flag = '0';
    char tagbuf[10];
    parser_result *tres;
    
    while (1) {
        if ((maxtimes--) == 0) return CHAYES_TIMEOUT;

        tres = send_timeout(ctx, command, 200);
        if(res_read_nth(tres,tagbuf,flag_idx)!=-1)
            if(*tagbuf == '1')
                return CHAYES_OK;

        nanosleep(interval, NULL);
    }
}

void send_without_res(control_ctx *ctx, const char *command) {
    parser_result * res = send_timeout(ctx,command,1000);
    ParseResultFree(res);
}

parser_result *send_timeout(control_ctx *self, const char *command,
                            uint64_t timeout) {
    static mqd_t queue;
    static parser_result *tres;

    parser_result *cached_res = NULL;

    // set inflight_tag
    tres = NewParseResult();
    self->inflight_tag[0] = 0;
    self->parser->parse_at_req(self->parser, tres, command);
    if (res_read_tag(tres, self->inflight_tag) == -1) {
        ParseResultFree(tres);
        return NULL;
    }
    ParseResultFree(tres);

    // send AT command

    self->shim.send(command, timeout);

    // poll the resp queue
    // 1. set time out
    struct timespec ddl;
    clock_gettime(CLOCK_REALTIME, &ddl);
    ddl.tv_nsec += timeout * 1e6;
    if (ddl.tv_nsec >= 1e9 - 1) {
        long sec = ddl.tv_nsec / (long)1e9;
        ddl.tv_nsec -= sec * (long)1e9;
        ddl.tv_sec += sec;
    }

    // 2.poll

    while (1) {
        int rescode = mq_timedreceive(self->resp_q, (char *)&tres,
                                      sizeof(parser_result *), NULL, &ddl);
        if (rescode == -1) {
            chayes_debug("mq_timedreceive() failed: %s\n", strerror(errno));
            self->inflight_tag[0] = 0;
            return NULL;
        } else {
            switch (tres->type) {
                case HAYES_RES_OK: {
                    self->inflight_tag[0] = 0;
                    if (cached_res != NULL) {
                        //free OK response just received and return the real cached response.
                        ParseResultFree(tres);
                        return cached_res;
                    } else {
                        //The command we sent only have OK response.Return it.
                        return tres;
                    }
                }
                case HAYES_RES_ERROR: {
                    self->inflight_tag[0] = 0;
                    return tres;
                }
                case HAYES_RES_EMPTY:
                case HAYES_REQ: {
                    ParseResultFree(tres);
                    break;
                }
                // Actually, we can only get standard response here. 
                // After receiving standard response, we should wait for OK.
                case HAYES_RES_RESP:
                case HAYES_RES_STDRESP: {
                    cached_res = tres;
                    break;
                }
                //unknown response, just free it.
                default:{
                    ParseResultFree(tres);
                    break;
                }
            }
        }
    }
}
