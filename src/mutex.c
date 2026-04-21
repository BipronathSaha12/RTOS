#include "rtos.h"

static int mutex_id_counter = 0;

int rtos_mutex_init(rtos_mutex_t *mutex)
{
    if (!mutex) return -1;

    mutex->id = mutex_id_counter++;
    mutex->locked = false;
    mutex->owner_task_id = -1;
    mutex->original_priority = 0;
    mutex->wait_count = 0;
    memset(mutex->wait_queue, -1, sizeof(mutex->wait_queue));
    pthread_mutex_init(&mutex->internal_lock, NULL);

    printf("[MUTEX] Initialized mutex #%d\n", mutex->id);
    return 0;
}

int rtos_mutex_lock(rtos_mutex_t *mutex)
{
    if (!mutex) return -1;

    pthread_mutex_lock(&mutex->internal_lock);

    task_tcb_t *current = scheduler_get_current_task();
    int task_id = current ? current->id : -1;

    if (mutex->locked) {
        if (mutex->owner_task_id == task_id) {
            pthread_mutex_unlock(&mutex->internal_lock);
            printf("[MUTEX] WARNING: Task %d already owns mutex #%d (deadlock avoided)\n",
                   task_id, mutex->id);
            return -1;
        }

        if (mutex->wait_count < MAX_TASKS) {
            mutex->wait_queue[mutex->wait_count++] = task_id;
        }

        printf("[MUTEX] Task %d blocked on mutex #%d (owner: Task %d)\n",
               task_id, mutex->id, mutex->owner_task_id);

        /* Priority inheritance */
        if (current) {
            task_tcb_t *owner = task_get_by_id(mutex->owner_task_id);
            if (owner && current->priority > owner->priority) {
                printf("[MUTEX] Priority inheritance: Task %d boosted %d -> %d\n",
                       owner->id, owner->priority, current->priority);
                mutex->original_priority = owner->priority;
                owner->priority = current->priority;
            }
        }

        if (current)
            current->state = TASK_BLOCKED;

        pthread_mutex_unlock(&mutex->internal_lock);
        usleep(10000);
        pthread_mutex_lock(&mutex->internal_lock);

        if (current)
            current->state = TASK_READY;
    }

    mutex->locked = true;
    mutex->owner_task_id = task_id;

    printf("[MUTEX] Task %d acquired mutex #%d\n", task_id, mutex->id);

    pthread_mutex_unlock(&mutex->internal_lock);
    return 0;
}

int rtos_mutex_trylock(rtos_mutex_t *mutex)
{
    if (!mutex) return -1;

    pthread_mutex_lock(&mutex->internal_lock);

    if (mutex->locked) {
        pthread_mutex_unlock(&mutex->internal_lock);
        return -1;
    }

    task_tcb_t *current = scheduler_get_current_task();
    mutex->locked = true;
    mutex->owner_task_id = current ? current->id : -1;

    printf("[MUTEX] Task %d trylock success on mutex #%d\n",
           mutex->owner_task_id, mutex->id);

    pthread_mutex_unlock(&mutex->internal_lock);
    return 0;
}

int rtos_mutex_unlock(rtos_mutex_t *mutex)
{
    if (!mutex) return -1;

    pthread_mutex_lock(&mutex->internal_lock);

    task_tcb_t *current = scheduler_get_current_task();
    int task_id = current ? current->id : -1;

    if (!mutex->locked || mutex->owner_task_id != task_id) {
        printf("[MUTEX] ERROR: Task %d cannot unlock mutex #%d (owner: %d)\n",
               task_id, mutex->id, mutex->owner_task_id);
        pthread_mutex_unlock(&mutex->internal_lock);
        return -1;
    }

    /* Restore original priority after inheritance */
    if (current && mutex->original_priority > 0) {
        printf("[MUTEX] Priority restored: Task %d back to %d\n",
               task_id, mutex->original_priority);
        current->priority = mutex->original_priority;
        mutex->original_priority = 0;
    }

    mutex->locked = false;
    mutex->owner_task_id = -1;

    /* Wake highest-priority waiter */
    if (mutex->wait_count > 0) {
        int best_idx = 0;
        uint8_t best_pri = 0;
        for (int i = 0; i < mutex->wait_count; i++) {
            task_tcb_t *t = task_get_by_id(mutex->wait_queue[i]);
            if (t && t->priority > best_pri) {
                best_pri = t->priority;
                best_idx = i;
            }
        }

        int woken_id = mutex->wait_queue[best_idx];
        for (int i = best_idx; i < mutex->wait_count - 1; i++)
            mutex->wait_queue[i] = mutex->wait_queue[i + 1];
        mutex->wait_count--;

        printf("[MUTEX] Waking Task %d from mutex #%d wait queue\n",
               woken_id, mutex->id);
    }

    printf("[MUTEX] Task %d released mutex #%d\n", task_id, mutex->id);

    pthread_mutex_unlock(&mutex->internal_lock);
    return 0;
}

void rtos_mutex_destroy(rtos_mutex_t *mutex)
{
    if (!mutex) return;
    printf("[MUTEX] Destroyed mutex #%d\n", mutex->id);
    pthread_mutex_destroy(&mutex->internal_lock);
}
