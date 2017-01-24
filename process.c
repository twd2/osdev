#include "process.h"

#include <asm.h>
#include <pm.h>
#include <interrupt.h>
#include <tty.h>
#include <stdlib/string.h>
#include <stdlib/memory.h>

static process_t processes[16];
static uint32_t process_count = 0;
static uint32_t current_process = -1;
static spinlock_t process_lock;
static volatile uint32_t ticks = 0;

uint32_t get_pid()
{
    return current_process;
}

uint32_t get_ttyid()
{
    return process_current()->tty - default_tty;
}

void init_process()
{
    register_irq_handler(IRQ_CLOCK, &process_irq_handler);
    spinlock_init(&process_lock);
}

process_t *process_current()
{
    if (current_process != -1)
    {
        return &processes[current_process];
    }
    else
    {
        return NULL;
    }
}

inline interrupt_frame_t *process_registers(process_t *proc)
{
    if ((proc->registers.cs & SELECTOR_RPL_MASK) != SELECTOR_RPL0)
    {
        return &(proc->registers);
    }
    else
    {
        return (interrupt_frame_t *)(proc->registers.isr_esp - sizeof(proc->registers.isr_esp));
    }
}

static inline void store_registers(process_t *proc, const interrupt_frame_t *src)
{
    interrupt_frame_t *frame = &proc->registers;
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

static inline void load_registers(interrupt_frame_t *dest, process_t *proc)
{
    interrupt_frame_t *frame = process_registers(proc);

    // preserve system flags
    frame->eflags = (dest->eflags & ~EFLAGS_MASK) | (frame->eflags & EFLAGS_MASK);

    // If this is not a kernel thread,
    // restoring its kernel stack is also needed.
    if ((frame->cs & SELECTOR_RPL_MASK) != SELECTOR_RPL0)
    {
        set_tss_stack0(proc->kernel_stack);
    }

    // Just let ESP point to the registers stored.
    dest->isr_esp = (uintptr_t)&frame->SECOND_REGISTER;

    // TODO: paging
}

bool process_schedule()
{
    process_t *proc = process_current();
    if (proc)
    {
        --proc->ticks;
        if (0 < proc->ticks && proc->ticks < proc->priority)
        {
            return false;
        }
        proc->ticks = proc->priority;
    }

    // TODO: schedule
    ++current_process;
    if (current_process >= process_count)
    {
        current_process = 0;
    }
    return true;
}

void process_irq_handler(uint8_t irq, interrupt_frame_t *frame)
{
    if (!process_count)
    {
        kprint_ok_fail("[KDEBUG] schedule failed: no process", false);
        return;
    }

    if (!spinlock_try_lock(&process_lock))
    {
        return;
    }

    process_t *proc_interrupted = process_current();
    if (process_schedule()) // if need to switch
    {
        // current_process changed

        // save current registers
        if (proc_interrupted)
        {
            store_registers(proc_interrupted, frame);
        }

        process_t *proc_new = process_current();

        // change registers to switch context
        load_registers(frame, proc_new);
        // TODO: paging
    }

out:
    spinlock_release(&process_lock);
}

uint32_t process_create(const char *name, entry_point_t entry_point, void *stack,
                        void *kernel_stack)
{
    if (process_count >= 16)
    {
        kprint_ok_fail("[KDEBUG] create process failed: process limit exceeded", false);
        return -1;
    }

    spinlock_wait_and_lock(&process_lock);

    process_t *proc = &processes[process_count];
    ++process_count;

    memset(proc, 0, sizeof(process_t));

    strcpy(proc->name, name);
    proc->tty = tty_current_screen();
    proc->ticks = proc->priority = PROCESS_PRIORITY_DEFAULT;

    proc->registers.cs = SELECTOR_USER_CODE;
    proc->registers.eip = (uintptr_t)entry_point;
    proc->registers.ss = SELECTOR_USER_DATA;
    proc->registers.esp = (uintptr_t)stack;
    proc->registers.ds = proc->registers.es = proc->registers.fs = proc->registers.gs =
        SELECTOR_USER_DATA;

    proc->kernel_stack = (uintptr_t)kernel_stack;

    spinlock_release(&process_lock);
    return process_count - 1;
}

uint32_t process_create_kernel(const char *name, entry_point_t entry_point, void *stack)
{
    if (process_count >= 16)
    {
        kprint_ok_fail("[KDEBUG] create process failed: process limit exceeded", false);
        return -1;
    }

    spinlock_wait_and_lock(&process_lock);

    process_t *proc = &processes[process_count];
    ++process_count;

    memset(proc, 0, sizeof(process_t));

    strcpy(proc->name, name);
    proc->tty = tty_current_screen();
    proc->ticks = proc->priority = PROCESS_PRIORITY_DEFAULT;

    proc->registers.cs = SELECTOR_KERNEL_CODE;
    proc->kernel_stack = NULL; // already kernel, so kernel_stack = NULL for TSS

    // "push" interrupt_frame
    interrupt_frame_t *frame = (interrupt_frame_t *)(stack - sizeof(interrupt_frame_t));
    frame->cs = SELECTOR_KERNEL_CODE;
    frame->eip = (uintptr_t)entry_point;
    frame->ds = frame->es = frame->fs = frame->gs = SELECTOR_KERNEL_DATA;
    proc->registers.isr_esp = (uintptr_t)&frame->SECOND_REGISTER;

    spinlock_release(&process_lock);
    return process_count - 1;
}

void process_set_priority(uint8_t priority)
{
    if (priority < PROCESS_PRIORITY_MIN || priority > PROCESS_PRIORITY_MAX)
    {
        return;
    }
    process_current()->priority = priority;
}
