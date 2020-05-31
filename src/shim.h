#include "stdint.h"
#ifndef _SHIM_C_
#define _SHIM_C_

typedef struct syscall_shim {
    void (*send)(const char*, uint32_t);
    void (*delay)(uint32_t msec);
    uint8_t (*getline)(const char* buf, uint32_t len, uint32_t timeout);
    void (*mutex_lock)(uint32_t timeout);
    void (*mutex_release)();
} syscall_shim;

#endif
