#include "hash-table.h"
#include "hayes_checker.h"
#include "hayes_parser.h"
#include "queue.h"
#include "shim.h"
#include "stdint.h"

#ifdef UNITTEST
#include "mqueue.h"
#include "pthread.h"
#else
#include "FreeRTOS_POSIX/mqueue.h"
#include "FreeRTOS_POSIX/pthread.h"
#endif

#ifndef C_HAYES_H_
#define C_HAYES_H_

#define CHAYES_QUEUE_NAME "/test_queue"

typedef void (*urc_hook)(const char*);

typedef struct control_ctx {
    syscall_shim shim;

    pthread_rwlock_t mu;
    mqd_t resp_q;

    HashTable* urc_hooks;
    hayes_checker* checker;
    hayes_parser* parser;
    char inflight_tag[30];
} control_ctx;

control_ctx* NewControlCtx(syscall_shim shim, hayes_checker* checker);

parser_result* send_timeout(control_ctx* self, const char* command,
                            uint64_t timeout);
void register_urc_hook(control_ctx* self, const char* urc, urc_hook hook);
void unregister_urc_hook(control_ctx* self, const char* urc);
void feed(control_ctx* self, const char* buf);

#endif

