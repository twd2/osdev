#ifndef _WDOS_KERNEL_ASM_H_
#define _WDOS_KERNEL_ASM_H_

#include <runtime/types.h>
#include <pm.h>

extern tss_entry_t tss_ptr;
extern descriptor_entry_t gdt32_tss;
extern gate_entry_t idt_ptr[256];
// unused: extern descriptor_entry_t gdt32_ptr[5];
extern const uint16_t SELECTOR_KERNEL_CODE;
extern const uint16_t SELECTOR_KERNEL_DATA;
extern const uint16_t SELECTOR_USER_CODE;
extern const uint16_t SELECTOR_USER_DATA;
extern const uint16_t SELECTOR_TSS;
extern const uint32_t TSS_LENGTH;

void enter_ring3();
uint32_t get_eflags();

#endif // _WDOS_KERNEL_ASM_H_
