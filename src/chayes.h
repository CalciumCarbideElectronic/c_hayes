#include "c_core/src/hash-table.h"
#include "hayes_checker.h"
#include "hayes_parser.h"
#include "shim.h"
#include "stdint.h"

#ifdef UNITTEST
#include "mqueue.h"
#include "pthread.h"
#include "time.h"
#else
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_POSIX/mqueue.h"
#include "FreeRTOS_POSIX/pthread.h"
//#include "FreeRTOS_POSIX/time.h"
#endif

#ifndef C_HAYES_H_
#define C_HAYES_H_

#define CHAYES_QUEUE_NAME "/test_queue"

typedef void (*urc_hook)(const char*);
typedef void (*cmd_hook)(const char*);

enum CHayesStatus {
    CHAYES_OK,
    CHAYES_ERR,
    CHAYES_TIMEOUT,
};

typedef struct control_ctx {
    syscall_shim shim;
    mqd_t resp_q;
    pthread_mutex_t mu;
    HashTable* urc_hooks;
    HashTable* command_hooks;
    hayes_checker* checker;
    hayes_parser* parser;

    uint8_t sending;

    char inflight_tag[30];
} control_ctx;

typedef struct chayes_request{
	const char *raw_cmd;
	uint8_t  no_tag;
} chayes_request;

control_ctx* NewControlCtx(syscall_shim shim, hayes_checker* checker);

void ControlCtxFree(control_ctx* self);

void hayes_async_message(parser_result * res);

void register_command_hook(control_ctx* self, const char* cmd, cmd_hook hook);
void unregister_command_hook(control_ctx* self, const char* cmd);
void register_urc_hook(control_ctx* self, const char* urc, urc_hook hook);
void unregister_urc_hook(control_ctx* self, const char* urc);

void feed(control_ctx* self, const char* buf);

enum CHayesStatus try_until_ok(control_ctx* ctx, chayes_request req,
                               uint16_t maxtimes, struct timespec* interval);

enum CHayesStatus try_until_flag_set(control_ctx* ctx, chayes_request req,
                                     uint16_t maxtimes,
                                     struct timespec* interval,
                                     uint16_t flag_idx);

void send_without_res(control_ctx* ctx, chayes_request req);

parser_result* send_timeout(control_ctx* self, chayes_request req,
                            struct timespec * ddl);

#endif

