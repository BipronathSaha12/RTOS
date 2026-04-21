#include "rtos.h"

static pthread_mutex_t context_lock = PTHREAD_MUTEX_INITIALIZER;
static volatile uint64_t switch_count = 0;

void context_init(cpu_context_t *ctx)
{
    memset(ctx, 0, sizeof(cpu_context_t));
    ctx->flags = 0x0202;
}

void context_save(cpu_context_t *ctx)
{
    pthread_mutex_lock(&context_lock);

    for (int i = 0; i < 16; i++)
        ctx->registers[i] = (uint64_t)rand();

    ctx->stack_pointer = (uint64_t)(uintptr_t)&ctx;
    ctx->program_counter = (uint64_t)(uintptr_t)__builtin_return_address(0);
    ctx->flags |= 0x01;

    pthread_mutex_unlock(&context_lock);
}

void context_restore(cpu_context_t *ctx)
{
    pthread_mutex_lock(&context_lock);

    ctx->flags &= ~0x01;
    ctx->flags |= 0x02;

    pthread_mutex_unlock(&context_lock);
}

void context_switch(cpu_context_t *from, cpu_context_t *to)
{
    pthread_mutex_lock(&context_lock);
    switch_count++;

    printf("[CONTEXT] Switch #%lu: saving context (PC=0x%lx, SP=0x%lx)\n",
           (unsigned long)switch_count,
           (unsigned long)from->program_counter,
           (unsigned long)from->stack_pointer);

    from->flags |= 0x01;

    to->flags &= ~0x01;
    to->flags |= 0x02;

    printf("[CONTEXT] Switch #%lu: restoring context (PC=0x%lx, SP=0x%lx)\n",
           (unsigned long)switch_count,
           (unsigned long)to->program_counter,
           (unsigned long)to->stack_pointer);

    pthread_mutex_unlock(&context_lock);
}

void context_print(const cpu_context_t *ctx)
{
    printf("  CPU Context:\n");
    printf("    PC  = 0x%016lx\n", (unsigned long)ctx->program_counter);
    printf("    SP  = 0x%016lx\n", (unsigned long)ctx->stack_pointer);
    printf("    FLG = 0x%016lx\n", (unsigned long)ctx->flags);
    printf("    Registers:\n");
    for (int i = 0; i < 16; i += 4) {
        printf("      R%-2d=0x%08lx  R%-2d=0x%08lx  R%-2d=0x%08lx  R%-2d=0x%08lx\n",
               i,   (unsigned long)ctx->registers[i],
               i+1, (unsigned long)ctx->registers[i+1],
               i+2, (unsigned long)ctx->registers[i+2],
               i+3, (unsigned long)ctx->registers[i+3]);
    }
}
