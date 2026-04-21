#ifndef RTOS_TIMER_H
#define RTOS_TIMER_H

#include "rtos.h"

typedef void (*timer_callback_t)(void *arg);

typedef struct {
    timer_t             posix_timer;
    bool                active;
    uint32_t            period_ms;
    bool                periodic;
    timer_callback_t    callback;
    void               *callback_arg;
} rtos_timer_t;

int     rtos_timer_init(void);
void    rtos_timer_stop_system(void);

int     rtos_timer_create(rtos_timer_t *timer, uint32_t period_ms,
                          bool periodic, timer_callback_t callback, void *arg);
int     rtos_timer_start(rtos_timer_t *timer);
int     rtos_timer_stop(rtos_timer_t *timer);
void    rtos_timer_destroy(rtos_timer_t *timer);

uint64_t rtos_timer_get_ticks(void);

#endif
