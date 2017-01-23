#ifndef _WDOS_KERNEL_ASM_H_
#define _WDOS_KERNEL_ASM_H_

#include <runtime/types.h>
#include <pm.h>

extern descriptor_entry_t gdt32_tss;
extern gate_entry_t idt_ptr[256];
// unused: extern descriptor_entry_t gdt32_ptr[5];
extern const uint16_t SELECTOR_KERNEL_CODE;
extern const uint16_t SELECTOR_KERNEL_DATA;
extern const uint16_t SELECTOR_USER_CODE;
extern const uint16_t SELECTOR_USER_DATA;
extern const uint16_t SELECTOR_TSS;
extern const uint32_t TSS_LENGTH;

// loader.asm
void enter_ring3();
uint32_t get_eflags();

bool test_and_set(bool *x);
bool test_and_reset(bool *x);
ureg_t compare_and_swap(ureg_t *x, ureg_t compare_value, ureg_t swap_value);

typedef struct spinlock
{
    volatile bool lock;
} spinlock_t;

void spinlock_init(spinlock_t *l);
bool spinlock_try_lock(spinlock_t *l);
void spinlock_wait_and_lock(spinlock_t *l);
void spinlock_release(spinlock_t *l);

#define save_flags(flags) \
    do \
    { \
        asm volatile ("pushf\n" \
                      "pop %0" \
                      : "=g"(flags) \
                      : \
                      : "memory"); \
    } \
    while (0)

#define restore_flags(flags) \
    do \
    { \
        asm volatile ("push %0\n" \
                      "popf" \
                      : \
                      : "g"(flags) \
                      : "memory"); \
    } \
    while (0)

#define cli() asm volatile ("cli")
#define sti() asm volatile ("sti")

#endif // _WDOS_KERNEL_ASM_H_
