#ifndef _WDOS_KERNEL_PROCESS_H_
#define _WDOS_KERNEL_PROCESS_H_

#include <runtime/types.h>
#include <tty.h>
#include <interrupt.h>

// control and status bits
#define EFLAGS_MASK 0b00000000000000000000110011010101

typedef struct process
{
    char name[16];
    tty_t *tty;
    interrupt_frame_t registers;
} process_t;

typedef void (*entry_point_t)();

uint32_t get_pid();
uint32_t get_ttyid();
void init_process();
process_t *process_current();
void process_schedule();
void process_irq_handler(uint8_t irq, interrupt_frame_t *frame);
uint32_t process_create(const char *name, uint16_t entry_point_seg, entry_point_t entry_point,
                        uint16_t stack_seg, void *stack);
uint32_t process_create_kernel_thread(const char *name, entry_point_t entry_point, void *stack);

#endif // _WDOS_KERNEL_PROCESS_H_
