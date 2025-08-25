#ifndef _CLOCK_TIMER_H
#define _CLOCK_TIMER_H

#include <sys/time.h>

#define BILLION 1000000000L

#define GET_TIME(now) { \
    struct timespec time; \
    clock_gettime(CLOCK_MONOTONIC, &time); \
    now = time.tv_sec + time.tv_nsec/1000000000.0; \
}
#endif