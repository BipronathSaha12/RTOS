#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include "rtos.h"

typedef struct queue_node {
    task_tcb_t         *task;
    struct queue_node  *next;
} queue_node_t;

typedef struct {
    queue_node_t   *head;
    queue_node_t   *tail;
    int             count;
    pthread_mutex_t lock;
} task_queue_t;

void            task_queue_init(task_queue_t *queue);
int             task_queue_enqueue(task_queue_t *queue, task_tcb_t *task);
task_tcb_t     *task_queue_dequeue(task_queue_t *queue);
task_tcb_t     *task_queue_peek(task_queue_t *queue);
task_tcb_t     *task_queue_dequeue_highest_priority(task_queue_t *queue);
int             task_queue_remove(task_queue_t *queue, int task_id);
bool            task_queue_is_empty(task_queue_t *queue);
int             task_queue_count(task_queue_t *queue);
void            task_queue_print(task_queue_t *queue);
void            task_queue_destroy(task_queue_t *queue);

#endif
