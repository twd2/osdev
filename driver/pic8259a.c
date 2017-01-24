#include "pic8259a.h"
#include <io.h>
#include <interrupt.h>

void init_pic8259a()
{
    outb(PIC8259A_MASTER_COMMAND, 0b00010001); // ICW1: PC, is ICW1, edge, 8B vec, cascade, has ICW4
    io_delay();
    outb(PIC8259A_SLAVE_COMMAND, 0b00010001);
    io_delay();

    outb(PIC8259A_MASTER_DATA, INTERRUPT_VECTOR_IRQ0); // ICW2: IRQ0, 80x86
    io_delay();
    outb(PIC8259A_SLAVE_DATA, INTERRUPT_VECTOR_IRQ8); // ICW2: IRQ8, 80x86
    io_delay();

    outb(PIC8259A_MASTER_DATA, 0b00000100); // ICW3: IR2 cascade
    io_delay();
    outb(PIC8259A_SLAVE_DATA, 2); // ICW3: IR2 cascade
    io_delay();

    outb(PIC8259A_MASTER_DATA, 1); // ICW4: 80x86
    io_delay();
    outb(PIC8259A_SLAVE_DATA, 1);
    io_delay();

    outb(PIC8259A_MASTER_DATA, 0); // OCW1: enable all interrupts(no mask)
    io_delay();
    outb(PIC8259A_SLAVE_DATA, 0);
    io_delay();
}

void send_eoi(uint8_t irq)
{
    uint16_t port = PIC8259A_MASTER_COMMAND;
    if (irq >= 8)
    {
        port = PIC8259A_SLAVE_COMMAND;
    }
    outb(port, PIC8259A_EOI);
    io_delay();
}
