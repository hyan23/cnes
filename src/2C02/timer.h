// timer.h
// Author: hyan23
// Date: 2018.04.02

#ifndef __TIMER_H__
#define __TIMER_H__

#include <time.h>
#include "header.h"

struct CNES_TIMER;

typedef void (*cnes_timer_callback)(struct CNES_TIMER* timer, void* userdata);

typedef struct CNES_TIMER
{
    cnes_timer_callback callback;
    void*   userdata;
    uint    interval;
    float   period; /* calculated */
    BOOL    shutdown;
}
cnes_timer_s;

extern void cnes_timer_init(cnes_timer_s* timer, 
    cnes_timer_callback callback, void* userdata, uint interval);
extern float cnes_timer_period(const cnes_timer_s* timer);
extern void cnes_timer_signal(cnes_timer_s* timer);
extern void cnes_timer_run(cnes_timer_s* timer);
// TODO: pause

#endif /* __TIMER_H__ */