#include "rtos.h"

static int sem_id_counter = 0;

int rtos_sem_init(rtos_sem_t *sem, int initial_count, int max_count)
{
    if (!sem || initial_count < 0 || max_count < 1 || initial_count > max_count)
        return -1;

    sem->id = sem_id_counter++;
    sem->count = initial_count;
    sem->max_count = max_count;
    sem->wait_count = 0;
    memset(sem->wait_queue, -1, sizeof(sem->wait_queue));
    pthread_mutex_init(&sem->internal_lock, NULL);
    pthread_cond_init(&sem->condition, NULL);

    printf("[SEM] Initialized semaphore #%d (count: %d/%d)\n",
           sem->id, initial_count, max_count);
    return 0;
}

int rtos_sem_wait(rtos_sem_t *sem)
{
    if (!sem) return -1;

    pthread_mutex_lock(&sem->internal_lock);

    task_tcb_t *current = scheduler_get_current_task();
    int task_id = current ? current->id : -1;

    while (sem->count <= 0) {
        printf("[SEM] Task %d waiting on semaphore #%d (count: %d)\n",
               task_id, sem->id, sem->count);

        if (sem->wait_count < MAX_TASKS && task_id >= 0)
            sem->wait_queue[sem->wait_count++] = task_id;

        if (current)
            current->state = TASK_BLOCKED;

        pthread_cond_wait(&sem->condition, &sem->internal_lock);

        if (current)
            current->state = TASK_READY;
    }

    sem->count--;
    printf("[SEM] Task %d acquired semaphore #%d (count: %d/%d)\n",
           task_id, sem->id, sem->count, sem->max_count);

    pthread_mutex_unlock(&sem->internal_lock);
    return 0;
}

int rtos_sem_trywait(rtos_sem_t *sem)
{
    if (!sem) return -1;

    pthread_mutex_lock(&sem->internal_lock);

    if (sem->count <= 0) {
        pthread_mutex_unlock(&sem->internal_lock);
        return -1;
    }

    sem->count--;
    task_tcb_t *current = scheduler_get_current_task();
    printf("[SEM] Task %d trywait success on semaphore #%d (count: %d)\n",
           current ? current->id : -1, sem->id, sem->count);

    pthread_mutex_unlock(&sem->internal_lock);
    return 0;
}

int rtos_sem_post(rtos_sem_t *sem)
{
    if (!sem) return -1;

    pthread_mutex_lock(&sem->internal_lock);

    if (sem->count >= sem->max_count) {
        printf("[SEM] WARNING: Semaphore #%d at max count (%d)\n",
               sem->id, sem->max_count);
        pthread_mutex_unlock(&sem->internal_lock);
        return -1;
    }

    sem->count++;
    task_tcb_t *current = scheduler_get_current_task();
    printf("[SEM] Task %d posted semaphore #%d (count: %d/%d)\n",
           current ? current->id : -1, sem->id, sem->count, sem->max_count);

    if (sem->wait_count > 0) {
        /* Wake highest-priority waiter */
        int best_idx = 0;
        uint8_t best_pri = 0;
        for (int i = 0; i < sem->wait_count; i++) {
            task_tcb_t *t = task_get_by_id(sem->wait_queue[i]);
            if (t && t->priority > best_pri) {
                best_pri = t->priority;
                best_idx = i;
            }
        }

        int woken_id = sem->wait_queue[best_idx];
        for (int i = best_idx; i < sem->wait_count - 1; i++)
            sem->wait_queue[i] = sem->wait_queue[i + 1];
        sem->wait_count--;

        printf("[SEM] Waking Task %d from semaphore #%d\n", woken_id, sem->id);
        pthread_cond_signal(&sem->condition);
    }

    pthread_mutex_unlock(&sem->internal_lock);
    return 0;
}

int rtos_sem_getcount(rtos_sem_t *sem)
{
    if (!sem) return -1;

    pthread_mutex_lock(&sem->internal_lock);
    int count = sem->count;
    pthread_mutex_unlock(&sem->internal_lock);
    return count;
}

void rtos_sem_destroy(rtos_sem_t *sem)
{
    if (!sem) return;
    printf("[SEM] Destroyed semaphore #%d\n", sem->id);
    pthread_mutex_destroy(&sem->internal_lock);
    pthread_cond_destroy(&sem->condition);
}
