#include "rtos.h"

void task_queue_init(task_queue_t *queue)
{
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
    pthread_mutex_init(&queue->lock, NULL);
}

int task_queue_enqueue(task_queue_t *queue, task_tcb_t *task)
{
    pthread_mutex_lock(&queue->lock);

    queue_node_t *node = (queue_node_t *)malloc(sizeof(queue_node_t));
    if (!node) {
        pthread_mutex_unlock(&queue->lock);
        return -1;
    }

    node->task = task;
    node->next = NULL;

    if (queue->tail) {
        queue->tail->next = node;
    } else {
        queue->head = node;
    }
    queue->tail = node;
    queue->count++;

    pthread_mutex_unlock(&queue->lock);
    return 0;
}

task_tcb_t *task_queue_dequeue(task_queue_t *queue)
{
    pthread_mutex_lock(&queue->lock);

    if (!queue->head) {
        pthread_mutex_unlock(&queue->lock);
        return NULL;
    }

    queue_node_t *node = queue->head;
    task_tcb_t *task = node->task;

    queue->head = node->next;
    if (!queue->head)
        queue->tail = NULL;

    queue->count--;
    free(node);

    pthread_mutex_unlock(&queue->lock);
    return task;
}

task_tcb_t *task_queue_peek(task_queue_t *queue)
{
    pthread_mutex_lock(&queue->lock);
    task_tcb_t *task = queue->head ? queue->head->task : NULL;
    pthread_mutex_unlock(&queue->lock);
    return task;
}

task_tcb_t *task_queue_dequeue_highest_priority(task_queue_t *queue)
{
    pthread_mutex_lock(&queue->lock);

    if (!queue->head) {
        pthread_mutex_unlock(&queue->lock);
        return NULL;
    }

    queue_node_t *best_prev = NULL;
    queue_node_t *best_node = queue->head;
    queue_node_t *prev = NULL;
    queue_node_t *curr = queue->head;

    while (curr) {
        if (curr->task->priority > best_node->task->priority) {
            best_prev = prev;
            best_node = curr;
        }
        prev = curr;
        curr = curr->next;
    }

    if (best_prev) {
        best_prev->next = best_node->next;
    } else {
        queue->head = best_node->next;
    }

    if (best_node == queue->tail)
        queue->tail = best_prev;

    queue->count--;
    task_tcb_t *task = best_node->task;
    free(best_node);

    pthread_mutex_unlock(&queue->lock);
    return task;
}

int task_queue_remove(task_queue_t *queue, int task_id)
{
    pthread_mutex_lock(&queue->lock);

    queue_node_t *prev = NULL;
    queue_node_t *curr = queue->head;

    while (curr) {
        if (curr->task->id == task_id) {
            if (prev) {
                prev->next = curr->next;
            } else {
                queue->head = curr->next;
            }
            if (curr == queue->tail)
                queue->tail = prev;

            queue->count--;
            free(curr);

            pthread_mutex_unlock(&queue->lock);
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }

    pthread_mutex_unlock(&queue->lock);
    return -1;
}

bool task_queue_is_empty(task_queue_t *queue)
{
    pthread_mutex_lock(&queue->lock);
    bool empty = (queue->count == 0);
    pthread_mutex_unlock(&queue->lock);
    return empty;
}

int task_queue_count(task_queue_t *queue)
{
    pthread_mutex_lock(&queue->lock);
    int count = queue->count;
    pthread_mutex_unlock(&queue->lock);
    return count;
}

void task_queue_print(task_queue_t *queue)
{
    pthread_mutex_lock(&queue->lock);

    printf("  Task Queue (%d tasks):\n", queue->count);
    queue_node_t *curr = queue->head;
    while (curr) {
        printf("    [ID:%d] %-16s  Pri:%d  State:%s\n",
               curr->task->id,
               curr->task->name,
               curr->task->priority,
               task_state_str(curr->task->state));
        curr = curr->next;
    }

    pthread_mutex_unlock(&queue->lock);
}

void task_queue_destroy(task_queue_t *queue)
{
    pthread_mutex_lock(&queue->lock);

    queue_node_t *curr = queue->head;
    while (curr) {
        queue_node_t *next = curr->next;
        free(curr);
        curr = next;
    }

    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;

    pthread_mutex_unlock(&queue->lock);
    pthread_mutex_destroy(&queue->lock);
}
