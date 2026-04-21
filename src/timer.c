#include "rtos.h"

static volatile uint64_t system_ticks = 0;
static timer_t           system_timer;
static bool              system_timer_active = false;

static void system_tick_handler(int sig, siginfo_t *si, void *uc)
{
    (void)sig;
    (void)si;
    (void)uc;

    system_ticks++;

    task_tcb_t *current = scheduler_get_current_task();
    if (current && current->state == TASK_RUNNING) {
        current->total_ticks++;
        if (current->ticks_remaining > 0)
            current->ticks_remaining--;

        if (current->ticks_remaining == 0) {
            printf("[TIMER] Time slice expired for '%s' (tick: %lu)\n",
                   current->name, (unsigned long)system_ticks);
            scheduler_tick();
        }
    }
}

int rtos_timer_init(void)
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = system_tick_handler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("[TIMER] sigaction failed");
        return -1;
    }

    struct sigevent sev;
    memset(&sev, 0, sizeof(sev));
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;

    if (timer_create(CLOCK_REALTIME, &sev, &system_timer) == -1) {
        perror("[TIMER] timer_create failed");
        return -1;
    }

    struct itimerspec its;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = TIME_SLICE_MS * 1000000L;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = TIME_SLICE_MS * 1000000L;

    if (timer_settime(system_timer, 0, &its, NULL) == -1) {
        perror("[TIMER] timer_settime failed");
        return -1;
    }

    system_timer_active = true;
    printf("[TIMER] System timer initialized (tick: %dms)\n", TIME_SLICE_MS);
    return 0;
}

void rtos_timer_stop_system(void)
{
    if (system_timer_active) {
        struct itimerspec its;
        memset(&its, 0, sizeof(its));
        timer_settime(system_timer, 0, &its, NULL);
        timer_delete(system_timer);
        system_timer_active = false;
        printf("[TIMER] System timer stopped (total ticks: %lu)\n",
               (unsigned long)system_ticks);
    }
}

int rtos_timer_create(rtos_timer_t *timer, uint32_t period_ms,
                      bool periodic, timer_callback_t callback, void *arg)
{
    if (!timer) return -1;

    timer->active = false;
    timer->period_ms = period_ms;
    timer->periodic = periodic;
    timer->callback = callback;
    timer->callback_arg = arg;

    struct sigevent sev;
    memset(&sev, 0, sizeof(sev));
    sev.sigev_notify = SIGEV_NONE;

    if (timer_create(CLOCK_REALTIME, &sev, &timer->posix_timer) == -1) {
        perror("[TIMER] user timer_create failed");
        return -1;
    }

    printf("[TIMER] User timer created (period: %ums, periodic: %s)\n",
           period_ms, periodic ? "yes" : "no");
    return 0;
}

int rtos_timer_start(rtos_timer_t *timer)
{
    if (!timer) return -1;

    struct itimerspec its;
    its.it_value.tv_sec = timer->period_ms / 1000;
    its.it_value.tv_nsec = (timer->period_ms % 1000) * 1000000L;

    if (timer->periodic) {
        its.it_interval = its.it_value;
    } else {
        its.it_interval.tv_sec = 0;
        its.it_interval.tv_nsec = 0;
    }

    if (timer_settime(timer->posix_timer, 0, &its, NULL) == -1) {
        perror("[TIMER] timer_start failed");
        return -1;
    }

    timer->active = true;
    printf("[TIMER] User timer started\n");
    return 0;
}

int rtos_timer_stop(rtos_timer_t *timer)
{
    if (!timer) return -1;

    struct itimerspec its;
    memset(&its, 0, sizeof(its));
    timer_settime(timer->posix_timer, 0, &its, NULL);
    timer->active = false;

    printf("[TIMER] User timer stopped\n");
    return 0;
}

void rtos_timer_destroy(rtos_timer_t *timer)
{
    if (!timer) return;
    rtos_timer_stop(timer);
    timer_delete(timer->posix_timer);
    printf("[TIMER] User timer destroyed\n");
}

uint64_t rtos_timer_get_ticks(void)
{
    return system_ticks;
}
