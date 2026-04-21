#ifndef CONTEXT_H
#define CONTEXT_H

#include "rtos.h"

void    context_init(cpu_context_t *ctx);
void    context_save(cpu_context_t *ctx);
void    context_restore(cpu_context_t *ctx);
void    context_switch(cpu_context_t *from, cpu_context_t *to);
void    context_print(const cpu_context_t *ctx);

#endif
