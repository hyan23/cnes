// timer.c
// Author: hyan23
// Date: 2018.04.02

#include <unistd.h>
#include "header.h"
#include "common.h"
#include "timer.h"

void 
cnes_timer_init(cnes_timer_s* timer, 
cnes_timer_callback callback, void* userdata, uint interval)
{
    timer->shutdown = FALSE;
    assert(NOTNULL(callback));
    timer->callback = callback;
    timer->userdata = userdata;
    assert(interval > 0);
    timer->interval = interval;
    timer->period = (float) 1e6 / interval;
}

float cnes_timer_period(const cnes_timer_s* timer)
{
    return timer->period;
}

void cnes_timer_signal(cnes_timer_s* timer)
{
    timer->shutdown = TRUE;
}

void cnes_timer_run(cnes_timer_s* timer)
{
    struct timespec start, stop;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
    while (!timer->shutdown) {
        timer->callback(timer, timer->userdata);
        if (timer->shutdown) {
            break;
        }
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &stop);
        uint useconds = INTERVAL(start, stop);
        assert(useconds > 0);
        if (timer->interval > useconds) {
            uint delay = timer->interval - useconds;
            usleep(delay);
        }
        // TODO: calculate period
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
    }
}