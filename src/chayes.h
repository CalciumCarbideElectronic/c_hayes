#include "hash-table.h"
#include "hayes_checker.h"
#include "stdint.h"

#ifndef C_HAYES_H_
#define C_HAYES_H_

typedef void (*func_send)(const char*, uint32_t);
typedef void (*func_delay)(uint32_t msec);
typedef int (*func_getline)(const char* buf, uint32_t len,uint32_t timeout);
typedef void (*urc_hook)(const char*);

typedef struct control_ctx {
    func_send sender;
    func_delay osDelay;
    func_getline getline;
    HashTable* urc_hooks;
    hayes_checker* checker;

} control_ctx;

void ctx_init(control_ctx* self, func_send sender, func_delay osDelay,
              func_getline getline, hayes_checker* checker);
int send_timeout(control_ctx* self, const char* command, uint64_t timeout);
void register_urc_hook(control_ctx* self, const char* urc, urc_hook hook);
void unregister_urc_hook(control_ctx* self, const char* urc);

#endif

