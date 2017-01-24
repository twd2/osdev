#include "syscall_impl.h"

#include <runtime/types.h>
#include <interrupt.h>
#include <process.h>

#define RETURN(x) do { frame->eax = (uint32_t)(x); return; } while (0)

typedef void (*syscall_handler_t)(interrupt_frame_t*, ...);

typedef struct syscall
{
    syscall_handler_t handler;
    uint8_t params_count;
} syscall_t;

static syscall_t syscall_handlers[SYSCALL_COUNT] = { NULL };

static void do_exit(interrupt_frame_t *frame, int exitcode)
{

}

static void do_test(interrupt_frame_t *frame)
{
    RETURN(0x900dbeef);
}

static void do_add(interrupt_frame_t *frame, int a, int b)
{
    RETURN(a + b);
}

static void do_yield(interrupt_frame_t *frame)
{
    process_current()->ticks = 1;
    process_irq_handler(IRQ_CLOCK, frame);
}

static void do_delay(interrupt_frame_t *frame, int x)
{
    enable_interrupt(); // allow context switch

    while (!x);

    int i = 10000;
    while (--i)
    {
        int j = 10 * x;
        while (--j);
    }
}

static void register_syscall_handler(uint32_t id, syscall_handler_t handler, uint8_t params_count)
{
    syscall_t *s = &syscall_handlers[id];
    s->handler = handler;
    s->params_count = params_count;
}

void init_syscall_impl()
{
#define REGISTER(func, params_count) \
    register_syscall_handler(SYS_##func, (syscall_handler_t)&do_##func, params_count)

    REGISTER(exit, 1);
    REGISTER(test, 0);
    REGISTER(add, 2);
    REGISTER(yield, 0);
    REGISTER(delay, 1);

#undef REGISTER
}

void syscall_dispatch(uint32_t id, interrupt_frame_t *frame)
{
    syscall_t *s = &syscall_handlers[id];
    if (s->params_count == 0)
    {
        s->handler(frame);
    }
    else if (s->params_count == 1)
    {
        s->handler(frame, frame->ebx);
    }
    else if (s->params_count == 2)
    {
        s->handler(frame, frame->ebx, frame->ecx);
    }
    else if (s->params_count == 3)
    {
        s->handler(frame, frame->ebx, frame->ecx, frame->edx);
    }
    else if (s->params_count == 4)
    {
        s->handler(frame, frame->ebx, frame->ecx, frame->edx, frame->edi);
    }
    else if (s->params_count == 5)
    {
        s->handler(frame, frame->ebx, frame->ecx, frame->edx, frame->edi, frame->esi);
    }
}
