#include "stdint.h"
#ifndef _TEST_SHIM_H
#define _TEST_SHIM_H
#define CHAYES_URC_SEMAPHORE_NAME "/chayes_test"
void send_null(const char *buf, uint32_t len);
void send_sync_urc(const char *buf, uint32_t len);
void wait_for_urc_semaphore();
#endif
