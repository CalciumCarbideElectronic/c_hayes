#ifdef UNITTEST
#include "stdarg.h"
#include "stdio.h"
#endif
void chayes_debug(const char *fmt, ...) {
#ifdef UNITTEST
    va_list args;
    va_start(args, fmt);
    vprintf(fmt,args);
    va_end(args);
#endif
}
