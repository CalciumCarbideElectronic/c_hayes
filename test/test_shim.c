#include "test_shim.h"

#include "stdio.h"

void send_stdio(const char *buf, uint32_t len) { printf("%s", buf); }

