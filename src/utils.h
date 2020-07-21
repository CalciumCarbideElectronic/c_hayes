#ifndef CHAYES_UTILS_H_
#define CHAYES_UTILS_H_

#include "inttypes.h"
#ifdef UNITTEST
#include "mqueue.h"
#include "pthread.h"
#include "time.h"
#else
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_POSIX/mqueue.h"
#include "FreeRTOS_POSIX/pthread.h"
#include "FreeRTOS_POSIX/time.h"
#endif

void timeutil_addnsec(struct timespec * t1, uint64_t nano);
void timeutil_add(struct timespec * t1,const struct timespec *t2);
#endif

