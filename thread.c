#include "thread.h"

#include <asm.h>
#include <pm.h>
#include <interrupt.h>
#include <tty.h>
#include <stdlib/string.h>
#include <stdlib/memory.h>

static thread_t threads[16];
static uint32_t thread_count = 0;
static uint32_t current_thread = -1;
static spinlock_t thread_lock;
static volatile uint32_t ticks = 0;

uint32_t get_pid()
{
    return current_thread;
}

uint32_t get_ttyid()
{
    return thread_current()->tty - default_tty;
}

void init_thread()
{
    spinlock_init(&thread_lock);
    register_irq_handler(IRQ_CLOCK, &thread_irq_handler);
}

thread_t *thread_current()
{
    if (current_thread != -1)
    {
        return &threads[current_thread];
    }
    else
    {
        return NULL;
    }
}

inline interrupt_frame_t *thread_registers(thread_t *th)
{
    if ((th->registers.cs & SELECTOR_RPL_MASK) != SELECTOR_RPL0)
    {
        return &(th->registers);
    }
    else
    {
        return (interrupt_frame_t *)(th->registers.isr_esp - sizeof(th->registers.isr_esp));
    }
}

static inline void store_registers(thread_t *th, const interrupt_frame_t *src)
{
    interrupt_frame_t *frame = &th->registers;
    if ((src->cs & SELECTOR_RPL_MASK) != SELECTOR_RPL0)
    {
        frame->gs = src->gs;
        frame->fs = src->fs;
        frame->es = src->es;
        frame->ds = src->ds;
        frame->edi = src->edi;
        frame->esi = src->esi;
        frame->ebp = src->ebp;
        frame->ebx = src->ebx;
        frame->edx = src->edx;
        frame->ecx = src->ecx;
        frame->eax = src->eax;
        frame->eip = src->eip;
        frame->cs = src->cs;
        frame->esp = src->esp;
        frame->ss = src->ss;
        // preserve system flags
        frame->eflags = (frame->eflags & ~EFLAGS_MASK) | (src->eflags & EFLAGS_MASK);
    }
    else
    {
        // For kernel threads,
        // registers are stored (pushed) in thread's own (kernel) stack,
        // so just record the ESP and remember it's a kernel thread using CS here.
        frame->isr_esp = src->isr_esp;
        frame->cs = src->cs;
    }
}

static inline void load_registers(interrupt_frame_t *dest, thread_t *th)
{
    interrupt_frame_t *frame = thread_registers(th);

    // preserve system flags
    frame->eflags = (dest->eflags & ~EFLAGS_MASK) | (frame->eflags & EFLAGS_MASK);

    // If this is not a kernel thread,
    // restoring its kernel stack is also needed.
    if ((frame->cs & SELECTOR_RPL_MASK) != SELECTOR_RPL0)
    {
        set_tss_stack0(th->kernel_stack);
    }

    // Just let ESP point to the registers stored.
    dest->isr_esp = (uintptr_t)&frame->SECOND_REGISTER;

    // TODO: paging
}

bool thread_schedule()
{
    thread_t *th = thread_current();
    if (th)
    {
        --th->ticks;
        if (0 < th->ticks && th->ticks < th->priority)
        {
            return false;
        }
        th->ticks = th->priority;
    }

    // TODO: schedule
    ++current_thread;
    if (current_thread >= thread_count)
    {
        current_thread = 0;
    }
    return true;
}

void thread_irq_handler(uint8_t irq, interrupt_frame_t *frame)
{
    if (!thread_count)
    {
        kprint_ok_fail("[KDEBUG] schedule failed: no thread", false);
        return;
    }

    if (!spinlock_try_lock(&thread_lock))
    {
        return;
    }

    thread_t *th_interrupted = thread_current();
    if (thread_schedule()) // if need to switch
    {
        // current_thread changed

        // save current registers
        if (th_interrupted)
        {
            store_registers(th_interrupted, frame);
        }

        thread_t *th_new = thread_current();

        // change registers to switch context
        load_registers(frame, th_new);
        // TODO: paging
    }

out:
    spinlock_release(&thread_lock);
}

uint32_t thread_create(const char *name, entry_point_t entry_point, void *stack,
                        void *kernel_stack)
{
    if (thread_count >= 16)
    {
        kprint_ok_fail("[KDEBUG] create thread failed: thread limit exceeded", false);
        return -1;
    }

    spinlock_wait_and_lock(&thread_lock);

    thread_t *th = &threads[thread_count];
    ++thread_count;

    memset(th, 0, sizeof(thread_t));

    strcpy(th->name, name);
    th->tty = tty_current_screen();
    th->ticks = th->priority = THREAD_PRIORITY_DEFAULT;

    th->registers.cs = SELECTOR_USER_CODE;
    th->registers.eip = (uintptr_t)entry_point;
    th->registers.ss = SELECTOR_USER_DATA;
    th->registers.esp = (uintptr_t)stack;
    th->registers.ds = th->registers.es = th->registers.fs = th->registers.gs =
        SELECTOR_USER_DATA;

    th->kernel_stack = (uintptr_t)kernel_stack;

    spinlock_release(&thread_lock);
    return thread_count - 1;
}

uint32_t thread_create_kernel(const char *name, entry_point_t entry_point, void *stack)
{
    if (thread_count >= 16)
    {
        kprint_ok_fail("[KDEBUG] create thread failed: thread limit exceeded", false);
        return -1;
    }

    spinlock_wait_and_lock(&thread_lock);

    thread_t *th = &threads[thread_count];
    ++thread_count;

    memset(th, 0, sizeof(thread_t));

    strcpy(th->name, name);
    th->tty = tty_current_screen();
    th->ticks = th->priority = THREAD_PRIORITY_DEFAULT;

    th->registers.cs = SELECTOR_KERNEL_CODE;
    th->kernel_stack = NULL; // already kernel, so kernel_stack = NULL for TSS

    // "push" interrupt_frame
    interrupt_frame_t *frame = (interrupt_frame_t *)(stack - sizeof(interrupt_frame_t));
    frame->cs = SELECTOR_KERNEL_CODE;
    frame->eip = (uintptr_t)entry_point;
    frame->ds = frame->es = frame->fs = frame->gs = SELECTOR_KERNEL_DATA;
    th->registers.isr_esp = (uintptr_t)&frame->SECOND_REGISTER;

    spinlock_release(&thread_lock);
    return thread_count - 1;
}

void thread_set_priority(uint8_t priority)
{
    if (priority < THREAD_PRIORITY_MIN || priority > THREAD_PRIORITY_MAX)
    {
        return;
    }
    thread_current()->priority = priority;
}
