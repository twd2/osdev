#ifndef _WDOS_KERNEL_INTERRUPT_H_
#define _WDOS_KERNEL_INTERRUPT_H_

#include <runtime/types.h>

typedef struct interrupt_frame
{
    uint32_t isr_esp; // esp of interrupt_wrapper
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t _; // unused
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t errorcode;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
} __attribute__((packed)) interrupt_frame_t;

#define SECOND_REGISTER gs

#define INTERRUPT_VECTOR_IRQ0 (0x20)
#define INTERRUPT_VECTOR_IRQ1 (INTERRUPT_VECTOR_IRQ0 + 1)
#define INTERRUPT_VECTOR_IRQ2 (INTERRUPT_VECTOR_IRQ0 + 2)
#define INTERRUPT_VECTOR_IRQ3 (INTERRUPT_VECTOR_IRQ0 + 3)
#define INTERRUPT_VECTOR_IRQ4 (INTERRUPT_VECTOR_IRQ0 + 4)
#define INTERRUPT_VECTOR_IRQ5 (INTERRUPT_VECTOR_IRQ0 + 5)
#define INTERRUPT_VECTOR_IRQ6 (INTERRUPT_VECTOR_IRQ0 + 6)
#define INTERRUPT_VECTOR_IRQ7 (INTERRUPT_VECTOR_IRQ0 + 7)
#define INTERRUPT_VECTOR_IRQ8 (0x28)
#define INTERRUPT_VECTOR_IRQ9 (INTERRUPT_VECTOR_IRQ8 + 1)
#define INTERRUPT_VECTOR_IRQ10 (INTERRUPT_VECTOR_IRQ8 + 2)
#define INTERRUPT_VECTOR_IRQ11 (INTERRUPT_VECTOR_IRQ8 + 3)
#define INTERRUPT_VECTOR_IRQ12 (INTERRUPT_VECTOR_IRQ8 + 4)
#define INTERRUPT_VECTOR_IRQ13 (INTERRUPT_VECTOR_IRQ8 + 5)
#define INTERRUPT_VECTOR_IRQ14 (INTERRUPT_VECTOR_IRQ8 + 6)
#define INTERRUPT_VECTOR_IRQ15 (INTERRUPT_VECTOR_IRQ8 + 7)
#define INTERRUPT_VECTOR_IRQ_COUNT 16

#define INTERRUPT_EXCEPTION_COUNT 32
#define INTERRUPT_VECTOR_SYSCALL 0x80

#define IRQ_CLOCK 0
#define IRQ_KEYBOARD 1
#define IRQ_COM2 3
#define IRQ_COM1 4
#define IRQ_LPT1 5
#define IRQ_FDD 6
#define IRQ_LPT2 7
#define IRQ_CMOSALTER 8
#define IRQ_MOUSE 12
#define IRQ_FPU 13
#define IRQ_IDE0 14
#define IRQ_IDE1 15

void interrupt_wrapper_0();
void interrupt_wrapper_1();
void interrupt_wrapper_2();
void interrupt_wrapper_3();
void interrupt_wrapper_4();
void interrupt_wrapper_5();
void interrupt_wrapper_6();
void interrupt_wrapper_7();
void interrupt_wrapper_8();
void interrupt_wrapper_9();
void interrupt_wrapper_10();
void interrupt_wrapper_11();
void interrupt_wrapper_12();
void interrupt_wrapper_13();
void interrupt_wrapper_14();
void interrupt_wrapper_15();
void interrupt_wrapper_16();
void interrupt_wrapper_17();
void interrupt_wrapper_18();
void interrupt_wrapper_19();
void interrupt_wrapper_20();
void interrupt_wrapper_21();
void interrupt_wrapper_22();
void interrupt_wrapper_23();
void interrupt_wrapper_24();
void interrupt_wrapper_25();
void interrupt_wrapper_26();
void interrupt_wrapper_27();
void interrupt_wrapper_28();
void interrupt_wrapper_29();
void interrupt_wrapper_30();
void interrupt_wrapper_31();

void interrupt_wrapper_32();
void interrupt_wrapper_33();
void interrupt_wrapper_34();
void interrupt_wrapper_35();
void interrupt_wrapper_36();
void interrupt_wrapper_37();
void interrupt_wrapper_38();
void interrupt_wrapper_39();
void interrupt_wrapper_40();
void interrupt_wrapper_41();
void interrupt_wrapper_42();
void interrupt_wrapper_43();
void interrupt_wrapper_44();
void interrupt_wrapper_45();
void interrupt_wrapper_46();
void interrupt_wrapper_47();

void interrupt_wrapper_128();

extern const char *const cpu_exception_strings[32];

typedef void (*irq_handler_t)(uint8_t, interrupt_frame_t*);

void interrupt_handler(uint8_t vec, interrupt_frame_t frame);
void irq_dispatch(uint8_t irq, interrupt_frame_t *frame);
void register_irq_handler(uint8_t irq, irq_handler_t handler);
void enable_interrupt();
void disable_interrupt();

#endif // _WDOS_KERNEL_INTERRUPT_H_
