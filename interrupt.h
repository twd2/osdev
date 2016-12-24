#ifndef _WDOS_INTERRUPT_H_
#define _WDOS_INTERRUPT_H_

#include <runtime/types.h>

typedef struct interrupt_frame
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_;
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
} interrupt_frame_t;

#define IRQ0 (0x20)
#define IRQ1 (IRQ0 + 1)
#define IRQ2 (IRQ0 + 2)
#define IRQ3 (IRQ0 + 3)
#define IRQ4 (IRQ0 + 4)
#define IRQ5 (IRQ0 + 5)
#define IRQ6 (IRQ0 + 6)
#define IRQ7 (IRQ0 + 7)
#define IRQ8 (0x70)
#define IRQ9 (IRQ8 + 1)
#define IRQ10 (IRQ8 + 2)
#define IRQ11 (IRQ8 + 3)
#define IRQ12 (IRQ8 + 4)
#define IRQ13 (IRQ8 + 5)
#define IRQ14 (IRQ8 + 6)
#define IRQ15 (IRQ8 + 7)

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
void interrupt_wrapper_112();
void interrupt_wrapper_113();
void interrupt_wrapper_114();
void interrupt_wrapper_115();
void interrupt_wrapper_116();
void interrupt_wrapper_117();
void interrupt_wrapper_118();
void interrupt_wrapper_119();
void interrupt_wrapper_128();
void interrupt_handler(uint32_t i, interrupt_frame_t frame);

#endif // _WDOS_INTERRUPT_H_
