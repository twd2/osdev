#include <asm.h>
#include <pm.h>
#include <interrupt.h>

void fill_descriptor(descriptor_entry_t *ptr, uint32_t base, uint32_t limit, uint32_t attr)
{
    ptr->limit1 = limit & 0xFFFF;
    ptr->base1 = base & 0xFFFF;
    ptr->base2 = (base >> 16) & 0xFF;
    ptr->attr1_limit2_attr2 = ((limit >> 8) & 0x0F00) | (attr & 0xF0FF);
    ptr->base3 = (base >> 24) & 0xFF;
}

void fill_gate(gate_entry_t *ptr, uint16_t selector, uint32_t offset, uint8_t attr)
{
    ptr->offset1 = offset & 0xFFFF;
    ptr->selector = selector;
    ptr->zero = 0;
    ptr->type_attr = attr;
    ptr->offset2 = (offset >> 16) & 0xFFFF;
}

void prepare_tss_gdt_entry()
{
    fill_descriptor(&gdt32_tss, (uint32_t)&tss_ptr, sizeof(tss_entry_t),
                    DESCRIPTOR_ATTR_TSS | DESCRIPTOR_ATTR_DPL3);
}

void prepare_idt()
{
    fill_gate(&idt_ptr[0], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_0,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[1], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_1,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[2], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_2,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[3], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_3,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[4], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_4,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[5], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_5,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[6], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_6,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[7], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_7,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[8], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_8,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[9], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_9,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[10], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_10,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[11], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_11,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[12], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_12,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[13], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_13,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[14], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_14,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[15], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_15,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[16], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_16,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[17], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_17,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[18], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_18,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[19], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_19,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[20], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_20,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[21], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_21,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[22], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_22,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[23], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_23,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[24], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_24,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[25], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_25,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[26], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_26,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[27], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_27,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[28], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_28,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[29], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_29,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[30], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_30,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[31], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_31,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[32], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_32,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[33], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_33,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[34], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_34,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[35], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_35,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[36], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_36,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[37], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_37,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[38], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_38,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[39], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_39,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[112], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_112,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[113], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_113,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[114], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_114,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[115], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_115,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[116], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_116,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[117], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_117,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[118], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_118,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[119], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_119,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL0);
    fill_gate(&idt_ptr[128], SELECTOR_KERNEL_CODE, (uint32_t)&interrupt_wrapper_128,
              DESCRIPTOR_ATTR_INTG | DESCRIPTOR_ATTR_DPL3);
}
