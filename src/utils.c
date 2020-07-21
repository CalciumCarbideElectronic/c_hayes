#include "utils.h"
void timeutil_add(struct timespec * t1,const struct timespec *t2){
    t1->tv_sec += t2->tv_sec;
    timeutil_addnsec(t1,t2->tv_nsec);

}

void timeutil_addnsec(struct timespec * t1, uint64_t nano){
    uint64_t  mid  = t1->tv_nsec += nano;
    t1 -> tv_sec += mid / 1000000000UL;
    t1 -> tv_nsec = mid % 1000000000UL;
}
 
