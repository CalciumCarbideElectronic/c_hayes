#include "hash-table.h"
#include "hayes_checker.h"
#include "hayes_parser.h"
#include "queue.h"
#include "shim.h"
#include "stdint.h"

#ifndef C_HAYES_H_
#define C_HAYES_H_

typedef void (*urc_hook)(const char*);

typedef struct control_ctx {
    syscall_shim shim;
    HashTable* urc_hooks;
    hayes_checker* checker;
    hayes_parser* parser;
    Queue* resp_queue;
    char inflight_tag[30];

} control_ctx;

control_ctx* NewControlCtx(syscall_shim shim, hayes_checker* checker);

int send_timeout(control_ctx* self, const char* command, uint64_t timeout);
void register_urc_hook(control_ctx* self, const char* urc, urc_hook hook);
void unregister_urc_hook(control_ctx* self, const char* urc);

#endif

