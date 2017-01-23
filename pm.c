#include <asm.h>
#include <pm.h>
#include <interrupt.h>

static tss_entry_t tss_entry;

inline void fill_descriptor(descriptor_entry_t *ptr, uint32_t base, uint32_t limit, uint32_t attr)
{
    ptr->limit1 = limit & 0xFFFF;
    ptr->base1 = base & 0xFFFF;
    ptr->base2 = (base >> 16) & 0xFF;
    ptr->attr1_limit2_attr2 = ((limit >> 8) & 0x0F00) | (attr & 0xF0FF);
    ptr->base3 = (base >> 24) & 0xFF;
}

inline void fill_gate(gate_entry_t *ptr, uint16_t selector, uint32_t offset, uint8_t attr)
{
    ptr->offset1 = offset & 0xFFFF;
    ptr->selector = selector;
    ptr->zero = 0;
    ptr->type_attr = attr;
    ptr->offset2 = (offset >> 16) & 0xFFFF;
}

inline void prepare_tss_gdt_entry()
{
    fill_descriptor(&gdt32_tss, (uintptr_t)&tss_entry, sizeof(tss_entry) - 1,
                    DESCRIPTOR_ATTR_TSS | DESCRIPTOR_ATTR_DPL3);
}

void init_pm()
{
    tss_entry.iomap_base = sizeof(tss_entry_t);
    prepare_tss_gdt_entry();
}

inline void reset_tss_busy(descriptor_entry_t *ptr)
{
    ptr->attr1_limit2_attr2 &= ~DESCRIPTOR_ATTR_BUSY;
}

inline void flush_tss()
{
    reset_tss_busy(&gdt32_tss);
    asm volatile ("ltr %0"
                  :
                  : "r"((uint16_t)SELECTOR_TSS));
}

void set_tss_stack0(ureg_t stack)
{
    tss_entry.ss0 = SELECTOR_KERNEL_DATA;
    tss_entry.esp0 = stack;
    flush_tss();
}

void prepare_idt()
{
    fill_gate(&idt_ptr[0], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_0,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[1], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_1,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[2], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_2,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[3], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_3,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[4], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_4,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[5], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_5,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[6], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_6,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[7], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_7,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[8], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_8,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[9], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_9,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[10], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_10,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[11], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_11,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[12], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_12,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[13], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_13,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[14], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_14,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[15], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_15,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[16], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_16,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[17], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_17,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[18], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_18,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[19], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_19,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[20], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_20,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[21], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_21,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[22], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_22,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[23], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_23,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[24], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_24,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[25], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_25,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[26], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_26,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[27], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_27,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[28], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_28,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[29], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_29,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[30], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_30,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[31], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_31,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[32], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_32,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[33], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_33,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[34], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_34,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[35], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_35,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[36], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_36,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[37], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_37,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[38], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_38,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[39], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_39,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[40], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_40,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[41], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_41,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[42], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_42,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[43], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_43,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[44], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_44,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[45], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_45,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[46], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_46,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[47], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_47,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL0);

    fill_gate(&idt_ptr[128], SELECTOR_KERNEL_CODE, (uintptr_t)&interrupt_wrapper_128,
              GATE_ATTR_INTERRUPT | DESCRIPTOR_ATTR_DPL3);
}
