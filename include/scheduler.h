#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "rtos.h"

void    scheduler_init(void);
void    scheduler_start(void);
void    scheduler_stop(void);
void    scheduler_tick(void);

void    scheduler_add_task(task_tcb_t *task);
void    scheduler_remove_task(task_tcb_t *task);

task_tcb_t *scheduler_get_next_task(void);
task_tcb_t *scheduler_get_current_task(void);

void    scheduler_lock(void);
void    scheduler_unlock(void);
bool    scheduler_is_running(void);

void    scheduler_print_ready_queue(void);

#endif
