#include "rtos.h"

static rtos_mutex_t shared_mutex;
static rtos_sem_t   shared_sem;
static volatile int shared_resource = 0;

/* ── Demo Task Functions ─────────────────────────────────── */

static void task_high_priority(void *arg)
{
    (void)arg;
    printf("\n  >> [HIGH-PRI] Starting critical work (Pri:7)\n");

    rtos_mutex_lock(&shared_mutex);
    printf("  >> [HIGH-PRI] Acquired mutex, accessing shared resource\n");
    shared_resource += 100;
    printf("  >> [HIGH-PRI] shared_resource = %d\n", shared_resource);
    usleep(50000);
    rtos_mutex_unlock(&shared_mutex);

    printf("  >> [HIGH-PRI] Done\n\n");
}

static void task_medium_priority(void *arg)
{
    (void)arg;
    printf("\n  >> [MED-PRI] Starting computation (Pri:5)\n");

    rtos_sem_wait(&shared_sem);
    printf("  >> [MED-PRI] Acquired semaphore\n");
    shared_resource += 50;
    printf("  >> [MED-PRI] shared_resource = %d\n", shared_resource);
    usleep(30000);
    rtos_sem_post(&shared_sem);

    printf("  >> [MED-PRI] Done\n\n");
}

static void task_low_priority(void *arg)
{
    (void)arg;
    printf("\n  >> [LOW-PRI] Starting background work (Pri:2)\n");

    rtos_mutex_lock(&shared_mutex);
    printf("  >> [LOW-PRI] Acquired mutex (may trigger priority inheritance)\n");
    shared_resource += 10;
    printf("  >> [LOW-PRI] shared_resource = %d\n", shared_resource);
    usleep(80000);
    rtos_mutex_unlock(&shared_mutex);

    printf("  >> [LOW-PRI] Done\n\n");
}

static void task_periodic(void *arg)
{
    int *counter = (int *)arg;
    (*counter)++;
    printf("\n  >> [PERIODIC] Heartbeat #%d (Pri:6)\n", *counter);
    rtos_sem_post(&shared_sem);
    printf("  >> [PERIODIC] Semaphore posted\n\n");
}

static void task_queue_demo(void *arg)
{
    (void)arg;
    printf("\n  >> [QUEUE] Task queue demonstration (Pri:3)\n");

    task_queue_t demo_queue;
    task_queue_init(&demo_queue);

    task_tcb_t *t;

    t = task_get_by_id(0);
    if (t) task_queue_enqueue(&demo_queue, t);
    t = task_get_by_id(1);
    if (t) task_queue_enqueue(&demo_queue, t);
    t = task_get_by_id(2);
    if (t) task_queue_enqueue(&demo_queue, t);

    printf("  >> [QUEUE] After enqueue:\n");
    task_queue_print(&demo_queue);

    task_tcb_t *highest = task_queue_dequeue_highest_priority(&demo_queue);
    if (highest)
        printf("  >> [QUEUE] Dequeued highest priority: '%s' (Pri:%d)\n",
               highest->name, highest->priority);

    printf("  >> [QUEUE] Remaining:\n");
    task_queue_print(&demo_queue);

    task_queue_destroy(&demo_queue);
    printf("  >> [QUEUE] Done\n\n");
}

/* ── Main ────────────────────────────────────────────────── */

int main(void)
{
    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║         RTOS Simulator - Full Demo              ║\n");
    printf("╠══════════════════════════════════════════════════╣\n");
    printf("║  Features:                                      ║\n");
    printf("║    - Priority-based preemptive scheduling       ║\n");
    printf("║    - POSIX timer tick engine                    ║\n");
    printf("║    - Mutex with priority inheritance            ║\n");
    printf("║    - Counting semaphore                         ║\n");
    printf("║    - Priority task queue                        ║\n");
    printf("║    - Simulated context switching                ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");

    /* Phase 1: Initialize all subsystems */
    printf("──── Phase 1: Initialization ────\n\n");
    task_init_all();
    scheduler_init();
    rtos_mutex_init(&shared_mutex);
    rtos_sem_init(&shared_sem, 1, 3);

    /* Phase 2: Create tasks with varying priorities */
    printf("\n──── Phase 2: Task Creation ────\n\n");
    static int heartbeat_counter = 0;

    int id_low  = task_create("LowPriTask",   2, task_low_priority,    NULL);
    int id_queue = task_create("QueueDemo",    3, task_queue_demo,      NULL);
    int id_med  = task_create("MedPriTask",    5, task_medium_priority, NULL);
    int id_per  = task_create("Periodic",      6, task_periodic,        &heartbeat_counter);
    int id_high = task_create("HighPriTask",   7, task_high_priority,   NULL);

    (void)id_low;
    (void)id_queue;
    (void)id_med;
    (void)id_per;
    (void)id_high;

    /* Phase 3: Show initial state */
    printf("\n──── Phase 3: Initial Ready Queues ────\n");
    scheduler_print_ready_queue();

    /* Phase 4: Initialize POSIX timer */
    printf("──── Phase 4: Timer Setup ────\n\n");
    if (rtos_timer_init() != 0) {
        printf("WARNING: POSIX timer init failed, running without timer\n");
    }

    /* Phase 5: Run scheduler — executes tasks by priority */
    printf("\n──── Phase 5: Scheduler Execution ────\n\n");
    scheduler_start();

    /* The scheduler dispatches tasks; let the timer drive further preemption */
    usleep(500000);

    /* Phase 6: Demonstrate suspend/resume */
    printf("\n──── Phase 6: Suspend / Resume ────\n\n");
    if (id_med >= 0) {
        task_suspend(id_med);
        usleep(100000);
        task_resume(id_med);
    }

    /* Run remaining tasks */
    for (int i = 0; i < 3; i++) {
        scheduler_tick();
        usleep(150000);
    }

    /* Phase 7: Context inspection */
    printf("\n──── Phase 7: Context Inspection ────\n\n");
    for (int i = 0; i < 5; i++) {
        task_tcb_t *t = task_get_by_id(i);
        if (t) {
            printf("Task '%s' (ID:%d, Pri:%d, State:%s):\n",
                   t->name, t->id, t->priority, task_state_str(t->state));
            context_print(&t->context);
            printf("\n");
        }
    }

    /* Phase 8: Cleanup */
    printf("──── Phase 8: Cleanup ────\n\n");
    scheduler_stop();
    rtos_timer_stop_system();
    rtos_mutex_destroy(&shared_mutex);
    rtos_sem_destroy(&shared_sem);

    printf("\nFinal shared_resource value: %d\n", shared_resource);
    printf("\n══════════════════════════════════════════════════\n");
    printf("  RTOS Simulation Complete\n");
    printf("══════════════════════════════════════════════════\n");

    return 0;
}
