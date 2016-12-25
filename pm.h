#ifndef _WDOS_PM_H_
#define _WDOS_PM_H_

#include <runtime/types.h>

#define DESCRIPTOR_ATTR_4K (1 << 15)
#define DESCRIPTOR_ATTR_DB (1 << 14)
#define DESCRIPTOR_ATTR_PRESENT (1 << 7)
#define DESCRIPTOR_ATTR_DPL0 (0 << 5)
#define DESCRIPTOR_ATTR_DPL3 (3 << 5)
#define DESCRIPTOR_ATTR_SEG (1 << 4)
#define DESCRIPTOR_ATTR_EX (1 << 3)
#define DESCRIPTOR_ATTR_DC (1 << 2)
#define DESCRIPTOR_ATTR_RW (1 << 1)
#define DESCRIPTOR_ATTR_AC (1 << 0)

#define SELECTOR_GDT (0 << 2)
#define SELECTOR_LDT (1 << 2)
#define SELECTOR_RPL0 (0 << 0)
#define SELECTOR_RPL3 (3 << 0)
#define SELECTOR_RPL_MASK (0b11)

#define DESCRIPTOR_ATTR_CODE32 (DESCRIPTOR_ATTR_PRESENT | DESCRIPTOR_ATTR_DB | \
                                DESCRIPTOR_ATTR_SEG | DESCRIPTOR_ATTR_EX | \
                                DESCRIPTOR_ATTR_RW | DESCRIPTOR_ATTR_4K)
#define DESCRIPTOR_ATTR_DATA32 (DESCRIPTOR_ATTR_PRESENT | DESCRIPTOR_ATTR_DB | \
                                DESCRIPTOR_ATTR_SEG | DESCRIPTOR_ATTR_RW | \
                                DESCRIPTOR_ATTR_4K)
#define DESCRIPTOR_ATTR_TSS (DESCRIPTOR_ATTR_PRESENT | 0x9)
#define DESCRIPTOR_ATTR_INTG (DESCRIPTOR_ATTR_PRESENT | 0xE)

typedef struct descriptor_entry
{
    uint16_t limit1;
    uint16_t base1;
    uint8_t base2;
    uint16_t attr1_limit2_attr2;
    uint8_t base3;
} __attribute__((packed)) descriptor_entry_t;

typedef struct gate_entry
{
    uint16_t offset1;  // offset bits 0..15
    uint16_t selector; // a code segment selector in GDT or LDT
    uint8_t zero;      // unused, set to 0
    uint8_t type_attr; // type and attributes, see below
    uint16_t offset2;  // offset bits 16..31
} __attribute__((packed)) gate_entry_t;

typedef struct tss_entry
{
    uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
    uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
    uint32_t ss0;        // The stack segment to load when we change to kernel mode.
    uint32_t esp1;       // everything below here is unusued now.. 
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;         
    uint32_t cs;        
    uint32_t ss;        
    uint32_t ds;        
    uint32_t fs;       
    uint32_t gs;         
    uint32_t ldt;      
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed)) tss_entry_t;

void fill_descriptor(descriptor_entry_t *ptr, uint32_t base, uint32_t limit, uint32_t attr);
void fill_gate(gate_entry_t *ptr, uint16_t selector, uint32_t offset, uint8_t attr);
void prepare_tss_gdt_entry();
void prepare_idt();

#endif // _WDOS_PM_H_
