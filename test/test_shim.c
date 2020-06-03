#include "test_shim.h"
#include "errno.h"
#include "fcntl.h"
#include "semaphore.h"
#include "stdio.h"
#include "string.h"

void send_null(const char *buf, uint32_t len) {}

void send_sync_urc(const char *buf, uint32_t len) {
    sem_t *sem = sem_open(CHAYES_URC_SEMAPHORE_NAME, O_CREAT, 0777, 0);
    sem_post(sem);
    int value;
    sem_getvalue(sem, &value);
}

void wait_for_urc_semaphore() {
    sem_t *sem = sem_open(CHAYES_URC_SEMAPHORE_NAME, O_CREAT, 0777, 0);
    int value;
    sem_getvalue(sem, &value);
    if (sem == SEM_FAILED) {
        printf("open semaphore failed: %s\n", strerror(errno));
    }
    sem_wait(sem);
}

