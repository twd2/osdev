#include <tss.h>

extern tss_entry_t tss_ptr;
extern descriptor_entry_t gdt32_tss;
extern idt_descriptor_entry_t idt_ptr[255];

void fill_descriptor(descriptor_entry_t *ptr, uint32_t base, uint32_t limit, uint32_t attr)
{
    ptr->limit1 = limit & 0xFFFF;
    ptr->base1 = base & 0xFFFF;
    ptr->base2 = (base >> 16) & 0xFF;
    ptr->attr1_limit2_attr2 = ((limit >> 8) & 0x0F00) | (attr & 0xF0FF);
    ptr->base3 = (base >> 24) & 0xFF;
}

void fill_idt_descriptor(idt_descriptor_entry_t *ptr, uint16_t selector, uint32_t offset)
{
    ptr->offset1 = offset & 0xFFFF;
    ptr->selector = selector;
    ptr->zero = 0;
    ptr->type_attr = DESCRIPTOR_ATTR_INTG;
    ptr->offset2 = (offset >> 16) & 0xFFFF;
}

void prepare_tss_gdt_entry()
{
    fill_descriptor(&gdt32_tss, (uint32_t)&tss_ptr, sizeof(tss_entry_t),
                    DESCRIPTOR_ATTR_TSS | DESCRIPTOR_ATTR_DPL0);
}

void prepare_idt()
{
    for (int i = 0; i < 255; ++i)
    {
        // TODO
        fill_idt_descriptor(idt_ptr + i, 0x08, 0x1001e0);
    }
}
