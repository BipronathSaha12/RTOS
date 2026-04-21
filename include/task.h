#ifndef TASK_H
#define TASK_H

#include "rtos.h"

int     task_create(const char *name, uint8_t priority, task_func_t entry, void *arg);
void    task_delete(int task_id);
void    task_suspend(int task_id);
void    task_resume(int task_id);
void    task_yield(void);
void    task_delay(uint32_t ticks);

task_tcb_t *task_get_current(void);
task_tcb_t *task_get_by_id(int task_id);
const char *task_state_str(task_state_t state);

void    task_init_all(void);

#endif
