#ifndef RTOS_SEMAPHORE_H
#define RTOS_SEMAPHORE_H

#include "rtos.h"

typedef struct {
    int             id;
    int             count;
    int             max_count;
    pthread_mutex_t internal_lock;
    pthread_cond_t  condition;
    int             wait_queue[MAX_TASKS];
    int             wait_count;
} rtos_sem_t;

int     rtos_sem_init(rtos_sem_t *sem, int initial_count, int max_count);
int     rtos_sem_wait(rtos_sem_t *sem);
int     rtos_sem_trywait(rtos_sem_t *sem);
int     rtos_sem_post(rtos_sem_t *sem);
int     rtos_sem_getcount(rtos_sem_t *sem);
void    rtos_sem_destroy(rtos_sem_t *sem);

#endif
