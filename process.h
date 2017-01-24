#ifndef _WDOS_KERNEL_PROCESS_H_
#define _WDOS_KERNEL_PROCESS_H_

#include <runtime/types.h>
#include <tty.h>
#include <interrupt.h>

#define PROCESS_PRIORITY_MIN 1
#define PROCESS_PRIORITY_DEFAULT 7
#define PROCESS_PRIORITY_MAX 16

// control and status bits (eg. CF, ZF, ...), no system bits (eg. IF, ...)
#define EFLAGS_MASK 0b00000000000000000000110011010101

typedef struct process
{
    char name[16];
    tty_t *tty;
    interrupt_frame_t registers;
    ureg_t pdbr; // cr3
    ureg_t kernel_stack;
    uint8_t priority;
    uint8_t ticks;
} process_t;

typedef void (*entry_point_t)();

uint32_t get_pid();
uint32_t get_ttyid();
void init_process();
process_t *process_current();
interrupt_frame_t *process_registers(process_t *proc);
bool process_schedule();
void process_irq_handler(uint8_t irq, interrupt_frame_t *frame);
uint32_t process_create(const char *name, entry_point_t entry_point, void *stack,
                        void *kernel_stack);
uint32_t process_create_kernel(const char *name, entry_point_t entry_point, void *stack);
void process_set_priority(uint8_t priority);

#endif // _WDOS_KERNEL_PROCESS_H_
