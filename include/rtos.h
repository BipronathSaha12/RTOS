#ifndef RTOS_H
#define RTOS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_TASKS           16
#define MAX_PRIORITY        8
#define STACK_SIZE          4096
#define TIME_SLICE_MS       100
#define TASK_NAME_LEN       32

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_SUSPENDED,
    TASK_TERMINATED
} task_state_t;

typedef struct {
    uint64_t registers[16];
    uint64_t stack_pointer;
    uint64_t program_counter;
    uint64_t flags;
} cpu_context_t;

typedef void (*task_func_t)(void *arg);

typedef struct task_tcb {
    int             id;
    char            name[TASK_NAME_LEN];
    uint8_t         priority;
    task_state_t    state;
    task_func_t     entry;
    void           *arg;
    cpu_context_t   context;
    uint8_t         stack[STACK_SIZE];
    uint32_t        time_slice;
    uint32_t        ticks_remaining;
    uint64_t        total_ticks;
    struct task_tcb *next;
} task_tcb_t;

#include "task.h"
#include "scheduler.h"
#include "mutex.h"
#include "semaphore.h"
#include "task_queue.h"
#include "timer.h"
#include "context.h"

#endif
