#include "process.h"
#include <asm.h>
#include <pm.h>
#include <interrupt.h>
#include <tty.h>
#include <stdlib/memory.h>

static process_t processes[16];
static uint32_t process_count = 0;
static uint32_t current_process = -1;
static volatile bool process_lock = false;

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

#define MOVE_REGISTERS(dest, src) \
    dest->gs = src->gs; \
    dest->fs = src->fs; \
    dest->es = src->es; \
    dest->ds = src->ds; \
    dest->edi = src->edi; \
    dest->esi = src->esi; \
    dest->ebp = src->ebp; \
    dest->ebx = src->ebx; \
    dest->edx = src->edx; \
    dest->ecx = src->ecx; \
    dest->eax = src->eax; \
    dest->eip = src->eip; \
    dest->cs = src->cs; \
    dest->eflags = (dest->eflags & ~EFLAGS_MASK) | (src->eflags & EFLAGS_MASK);

// only appear when cs.RPL != 0
#define MOVE_OPTIONAL_REGISTERS(dest, src) \
    dest->esp = src->esp; \
    dest->ss = src->ss;

static inline void store_registers(interrupt_frame_t *dest, const interrupt_frame_t *src)
{
    if ((src->cs & SELECTOR_RPL_MASK) != SELECTOR_RPL0)
    {
        MOVE_REGISTERS(dest, src);
        MOVE_OPTIONAL_REGISTERS(dest, src);
    }
    else
    {
        dest->isr_esp = src->isr_esp;
        dest->cs = src->cs;
    }
}

static inline void load_registers(interrupt_frame_t *dest, interrupt_frame_t *src)
{
    // src CPL=3
    if ((src->cs & SELECTOR_RPL_MASK) != SELECTOR_RPL0)
    {
        // save system flags
        src->eflags = (dest->eflags & ~EFLAGS_MASK) | (src->eflags & EFLAGS_MASK);
        dest->isr_esp = (uint32_t)&src->SECOND_REGISTER;
    }
    else
    {
        interrupt_frame_t *frame = (interrupt_frame_t *)(src->isr_esp - sizeof(uint32_t));
        // save system flags
        frame->eflags = (dest->eflags & ~EFLAGS_MASK) | (frame->eflags & EFLAGS_MASK);
        dest->isr_esp = src->isr_esp;
    }
    // TODO: paging
}

#undef MOVE_REGISTERS
#undef MOVE_OPTIONAL_REGISTERS

void process_schedule()
{
    // TODO: schedule
    ++current_process;
    if (current_process >= process_count)
    {
        current_process = 0;
    }
}

void process_irq_handler(uint8_t irq, interrupt_frame_t *frame)
{
    if (process_lock)
    {
        return;
    }
    process_lock = true;
    // save current registers
    if (current_process != (uint32_t)(-1))
    {
        store_registers(&processes[current_process].registers, frame);
    }
    if (!process_count)
    {
        kprint_ok_fail("[KDEBUG] schedule failed: no process", false);
        process_lock = false;
        return;
    }
    // current_process changed
    process_schedule();
    // change registers to switch process
    load_registers(frame, &processes[current_process].registers);
    process_lock = false;
}

uint32_t process_create(const char *name, uint16_t entry_point_seg, entry_point_t entry_point,
                        uint16_t stack_seg, void *stack)
{
    if (process_count >= 16)
    {
        kprint_ok_fail("[KDEBUG] create process failed: process limit exceeded", false);
        return -1;
    }
    process_lock = true;
    process_t *proc = &processes[process_count];
    ++process_count;
    strcpy(proc->name, name);
    proc->tty = tty_current_screen();
    proc->registers.cs = entry_point_seg;
    proc->registers.eip = (uint32_t)entry_point;
    proc->registers.ss = stack_seg;
    proc->registers.esp = (uint32_t)stack;
    proc->registers.ds = proc->registers.es = proc->registers.fs = proc->registers.gs = stack_seg;
    // process.cpl == 0
    if ((entry_point_seg & SELECTOR_RPL_MASK) == SELECTOR_RPL0)
    {
        // allocate interrupt_frame 
        interrupt_frame_t *frame = (interrupt_frame_t *)(stack - sizeof(interrupt_frame_t));
        frame->isr_esp = proc->registers.isr_esp;
        frame->cs = proc->registers.cs;
        frame->eip = proc->registers.eip;
        frame->ds = frame->es = frame->fs = frame->gs = stack_seg;
        proc->registers.isr_esp = (uint32_t)&frame->SECOND_REGISTER;
    }
    process_lock = false;
    return process_count - 1;
}

uint32_t process_create_kernel_thread(const char *name, entry_point_t entry_point, void *stack)
{
    if (process_count >= 16)
    {
        kprint_ok_fail("[KDEBUG] create process failed: process limit exceeded", false);
        return -1;
    }
    process_lock = true;
    process_t *proc = &processes[process_count];
    ++process_count;
    strcpy(proc->name, name);
    proc->tty = tty_current_screen();
    proc->registers.cs = SELECTOR_KERNEL_CODE;
    proc->registers.eip = (uint32_t)entry_point;
    proc->registers.ss = SELECTOR_KERNEL_DATA;
    proc->registers.esp = (uint32_t)stack;
    proc->registers.ds = proc->registers.es = proc->registers.fs = proc->registers.gs =
        SELECTOR_KERNEL_DATA;
    // allocate interrupt_frame
    interrupt_frame_t *frame = (interrupt_frame_t *)(stack - sizeof(interrupt_frame_t));
    frame->isr_esp = proc->registers.isr_esp;
    frame->cs = SELECTOR_KERNEL_CODE;
    frame->eip = proc->registers.eip;
    frame->ds = frame->es = frame->fs = frame->gs = SELECTOR_KERNEL_DATA;
    proc->registers.isr_esp = (uint32_t)&frame->SECOND_REGISTER;
    process_lock = false;
    return process_count - 1;
}
