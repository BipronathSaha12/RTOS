#ifndef RTOS_MUTEX_H
#define RTOS_MUTEX_H

#include "rtos.h"

typedef struct {
    int             id;
    bool            locked;
    int             owner_task_id;
    uint8_t         original_priority;
    pthread_mutex_t internal_lock;
    int             wait_queue[MAX_TASKS];
    int             wait_count;
} rtos_mutex_t;

int     rtos_mutex_init(rtos_mutex_t *mutex);
int     rtos_mutex_lock(rtos_mutex_t *mutex);
int     rtos_mutex_trylock(rtos_mutex_t *mutex);
int     rtos_mutex_unlock(rtos_mutex_t *mutex);
void    rtos_mutex_destroy(rtos_mutex_t *mutex);

#endif
