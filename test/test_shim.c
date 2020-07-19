#include "test_shim.h"
#include "errno.h"
#include "fcntl.h"
#include "semaphore.h"
#include "stdio.h"
#include "string.h"
#include "time.h"


void send_null(const char *buf, uint32_t len) {}

void send_sync_urc(const char *buf, uint32_t len) {
    sem_t *sem = sem_open(CHAYES_URC_SEMAPHORE_NAME, O_CREAT, 0777, 0);
    if (sem == SEM_FAILED)
        printf("open semaphore failed: %s\n", strerror(errno));
    sem_post(sem);
    int value;
    sem_getvalue(sem, &value);
}

void wait_for_urc_semaphore() {
    sem_t *sem = sem_open(CHAYES_URC_SEMAPHORE_NAME, O_CREAT, 0777, 0);
    if (sem == SEM_FAILED)
        printf("open semaphore failed: %s\n", strerror(errno));
    int value;
    sem_getvalue(sem, &value);
    struct timespec now;
    clock_gettime(clock(),&now);
    now.tv_nsec += 200000;
    if(now.tv_nsec>=1e9){
        now.tv_nsec-=1e9;
        now.tv_sec+=1;
    }
    
    sem_timedwait(sem,&now);
}

