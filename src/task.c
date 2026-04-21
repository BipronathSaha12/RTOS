#include "rtos.h"

static task_tcb_t   task_pool[MAX_TASKS];
static bool         task_slot_used[MAX_TASKS];
static int          current_task_id = -1;
static pthread_mutex_t task_pool_lock = PTHREAD_MUTEX_INITIALIZER;

void task_init_all(void)
{
    pthread_mutex_lock(&task_pool_lock);
    for (int i = 0; i < MAX_TASKS; i++) {
        memset(&task_pool[i], 0, sizeof(task_tcb_t));
        task_pool[i].id = -1;
        task_pool[i].state = TASK_TERMINATED;
        task_slot_used[i] = false;
    }
    current_task_id = -1;
    pthread_mutex_unlock(&task_pool_lock);
    printf("[TASK] Task pool initialized (%d slots)\n", MAX_TASKS);
}

int task_create(const char *name, uint8_t priority, task_func_t entry, void *arg)
{
    pthread_mutex_lock(&task_pool_lock);

    int slot = -1;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!task_slot_used[i]) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        pthread_mutex_unlock(&task_pool_lock);
        printf("[TASK] ERROR: No free task slots\n");
        return -1;
    }

    task_tcb_t *tcb = &task_pool[slot];
    memset(tcb, 0, sizeof(task_tcb_t));

    tcb->id = slot;
    strncpy(tcb->name, name, TASK_NAME_LEN - 1);
    tcb->name[TASK_NAME_LEN - 1] = '\0';
    tcb->priority = (priority > MAX_PRIORITY) ? MAX_PRIORITY : priority;
    tcb->state = TASK_READY;
    tcb->entry = entry;
    tcb->arg = arg;
    tcb->time_slice = TIME_SLICE_MS;
    tcb->ticks_remaining = TIME_SLICE_MS;
    tcb->total_ticks = 0;
    tcb->next = NULL;

    context_init(&tcb->context);
    tcb->context.stack_pointer = (uint64_t)(uintptr_t)&tcb->stack[STACK_SIZE - 1];
    tcb->context.program_counter = (uint64_t)(uintptr_t)entry;

    task_slot_used[slot] = true;

    pthread_mutex_unlock(&task_pool_lock);

    scheduler_add_task(tcb);

    printf("[TASK] Created '%s' (ID:%d, Pri:%d)\n", name, slot, priority);
    return slot;
}

void task_delete(int task_id)
{
    if (task_id < 0 || task_id >= MAX_TASKS) return;

    pthread_mutex_lock(&task_pool_lock);

    if (!task_slot_used[task_id]) {
        pthread_mutex_unlock(&task_pool_lock);
        return;
    }

    task_tcb_t *tcb = &task_pool[task_id];
    scheduler_remove_task(tcb);

    printf("[TASK] Deleted '%s' (ID:%d)\n", tcb->name, task_id);

    tcb->state = TASK_TERMINATED;
    task_slot_used[task_id] = false;

    pthread_mutex_unlock(&task_pool_lock);
}

void task_suspend(int task_id)
{
    if (task_id < 0 || task_id >= MAX_TASKS) return;

    pthread_mutex_lock(&task_pool_lock);
    if (task_slot_used[task_id] && task_pool[task_id].state == TASK_READY) {
        task_pool[task_id].state = TASK_SUSPENDED;
        printf("[TASK] Suspended '%s' (ID:%d)\n", task_pool[task_id].name, task_id);
    }
    pthread_mutex_unlock(&task_pool_lock);
}

void task_resume(int task_id)
{
    if (task_id < 0 || task_id >= MAX_TASKS) return;

    pthread_mutex_lock(&task_pool_lock);
    if (task_slot_used[task_id] && task_pool[task_id].state == TASK_SUSPENDED) {
        task_pool[task_id].state = TASK_READY;
        printf("[TASK] Resumed '%s' (ID:%d)\n", task_pool[task_id].name, task_id);
    }
    pthread_mutex_unlock(&task_pool_lock);
}

void task_yield(void)
{
    if (current_task_id >= 0) {
        printf("[TASK] '%s' yielding CPU\n", task_pool[current_task_id].name);
        scheduler_tick();
    }
}

void task_delay(uint32_t ticks)
{
    if (current_task_id >= 0) {
        printf("[TASK] '%s' delaying for %u ticks\n",
               task_pool[current_task_id].name, ticks);
        task_pool[current_task_id].state = TASK_BLOCKED;
        usleep(ticks * 1000);
        task_pool[current_task_id].state = TASK_READY;
    }
}

task_tcb_t *task_get_current(void)
{
    if (current_task_id >= 0 && current_task_id < MAX_TASKS)
        return &task_pool[current_task_id];
    return NULL;
}

task_tcb_t *task_get_by_id(int task_id)
{
    if (task_id >= 0 && task_id < MAX_TASKS && task_slot_used[task_id])
        return &task_pool[task_id];
    return NULL;
}

const char *task_state_str(task_state_t state)
{
    switch (state) {
    case TASK_READY:      return "READY";
    case TASK_RUNNING:    return "RUNNING";
    case TASK_BLOCKED:    return "BLOCKED";
    case TASK_SUSPENDED:  return "SUSPENDED";
    case TASK_TERMINATED: return "TERMINATED";
    default:              return "UNKNOWN";
    }
}
