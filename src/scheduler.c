#include "rtos.h"

static task_queue_t     ready_queues[MAX_PRIORITY + 1];
static task_tcb_t      *current_task = NULL;
static bool             running = false;
static pthread_mutex_t  sched_lock = PTHREAD_MUTEX_INITIALIZER;
static uint64_t         total_switches = 0;

void scheduler_init(void)
{
    for (int i = 0; i <= MAX_PRIORITY; i++)
        task_queue_init(&ready_queues[i]);

    current_task = NULL;
    running = false;
    total_switches = 0;

    printf("[SCHED] Scheduler initialized (%d priority levels)\n", MAX_PRIORITY + 1);
}

void scheduler_add_task(task_tcb_t *task)
{
    if (!task || task->priority > MAX_PRIORITY) return;

    pthread_mutex_lock(&sched_lock);
    task_queue_enqueue(&ready_queues[task->priority], task);
    pthread_mutex_unlock(&sched_lock);

    printf("[SCHED] Added '%s' to ready queue (Pri:%d)\n", task->name, task->priority);
}

void scheduler_remove_task(task_tcb_t *task)
{
    if (!task || task->priority > MAX_PRIORITY) return;

    pthread_mutex_lock(&sched_lock);
    task_queue_remove(&ready_queues[task->priority], task->id);
    pthread_mutex_unlock(&sched_lock);
}

task_tcb_t *scheduler_get_next_task(void)
{
    for (int pri = MAX_PRIORITY; pri >= 0; pri--) {
        if (!task_queue_is_empty(&ready_queues[pri])) {
            task_tcb_t *task = task_queue_dequeue(&ready_queues[pri]);
            if (task && task->state == TASK_READY)
                return task;
            if (task)
                task_queue_enqueue(&ready_queues[pri], task);
        }
    }
    return NULL;
}

task_tcb_t *scheduler_get_current_task(void)
{
    return current_task;
}

void scheduler_tick(void)
{
    pthread_mutex_lock(&sched_lock);

    if (!running) {
        pthread_mutex_unlock(&sched_lock);
        return;
    }

    task_tcb_t *next = NULL;
    for (int pri = MAX_PRIORITY; pri >= 0; pri--) {
        if (!task_queue_is_empty(&ready_queues[pri])) {
            next = task_queue_dequeue(&ready_queues[pri]);
            if (next && next->state == TASK_READY)
                break;
            if (next) {
                task_queue_enqueue(&ready_queues[pri], next);
                next = NULL;
            }
        }
    }

    if (!next) {
        pthread_mutex_unlock(&sched_lock);
        return;
    }

    task_tcb_t *prev = current_task;

    if (prev && prev->state == TASK_RUNNING) {
        prev->state = TASK_READY;
        context_save(&prev->context);
        task_queue_enqueue(&ready_queues[prev->priority], prev);
    }

    current_task = next;
    current_task->state = TASK_RUNNING;
    current_task->ticks_remaining = current_task->time_slice;
    total_switches++;

    if (prev) {
        printf("[SCHED] Preempt: '%s' (Pri:%d) -> '%s' (Pri:%d) [switch #%lu]\n",
               prev->name, prev->priority,
               current_task->name, current_task->priority,
               (unsigned long)total_switches);
        context_switch(&prev->context, &current_task->context);
    } else {
        printf("[SCHED] Dispatch: '%s' (Pri:%d) [switch #%lu]\n",
               current_task->name, current_task->priority,
               (unsigned long)total_switches);
        context_restore(&current_task->context);
    }

    pthread_mutex_unlock(&sched_lock);

    if (current_task->entry)
        current_task->entry(current_task->arg);
}

void scheduler_start(void)
{
    running = true;
    printf("[SCHED] Scheduler started\n");
    printf("========================================\n");
    scheduler_tick();
}

void scheduler_stop(void)
{
    running = false;
    printf("========================================\n");
    printf("[SCHED] Scheduler stopped (total switches: %lu)\n",
           (unsigned long)total_switches);
}

void scheduler_lock(void)
{
    pthread_mutex_lock(&sched_lock);
}

void scheduler_unlock(void)
{
    pthread_mutex_unlock(&sched_lock);
}

bool scheduler_is_running(void)
{
    return running;
}

void scheduler_print_ready_queue(void)
{
    printf("\n--- Ready Queues ---\n");
    for (int pri = MAX_PRIORITY; pri >= 0; pri--) {
        if (!task_queue_is_empty(&ready_queues[pri])) {
            printf("  Priority %d:\n", pri);
            task_queue_print(&ready_queues[pri]);
        }
    }
    printf("--- End Queues ---\n\n");
}
