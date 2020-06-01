#include "stdint.h"
#ifndef _SHIM_C_
#define _SHIM_C_
void chayes_debug(const char *fmt, ...) ;

typedef struct syscall_shim {
    void (*send)(const char*, uint32_t);
} syscall_shim;

#endif
