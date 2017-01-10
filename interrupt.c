#include <interrupt.h>
#include <kstdio.h>
#include <driver/8259a.h>
#include <process.h>

const char *cpu_exception_strings[32] = 
{
    "#DE Divide-by-zero Error",
    "#DB Debug",
    "--- Non-maskable Interrupt",
    "#BP Breakpoint",
    "#OF Overflow",
    "#BR Bound Range Exceeded",
    "#UD Invalid Opcode",
    "#NM Device Not Available",
    "#DF Double Fault",
    "--- Coprocessor Segment Overrun",
    "#TS Invalid TSS",
    "#NP Segment Not Present",
    "#SS Stack-Segment Fault",
    "#GP General Protection Fault",
    "#PF Page Fault",
    "--- [Reserved]",
    "#MF x87 Floating-Point Exception",
    "#AC Alignment Check",
    "#MC Machine Check",
    "#XM/#XF SIMD Floating-Point Exception",
    "#VE Virtualization Exception",
    "--- [Reserved]",
    "--- [Reserved]",
    "--- [Reserved]",
    "--- [Reserved]",
    "--- [Reserved]",
    "--- [Reserved]",
    "--- [Reserved]",
    "--- [Reserved]",
    "--- [Reserved]",
    "#SX Security Exception",
    "--- [Reserved]",
};

static irq_handler_t irq_handlers[16] = { NULL };

void interrupt_handler(uint8_t vec, interrupt_frame_t frame)
{
    if (vec == 0x80)
    {
        kprint("[KDEBUG] Interrupt: system call #");
        kprint_int(frame.eax);
        kprint("\n");
        // TODO
        if (frame.eax == 1) // test
        {
            frame.eax = 0x900dbeef;
        }
        else if (frame.eax == 2)
        {
            frame.eax = frame.ebx + frame.ecx;
        }
        else
        {
            frame.eax = 0xdeadbeef;
        }
    }
    else if (vec < 32) // cpu exception
    {
        cls_color(KSTDIO_MKCOLOR(KSTDIO_COLOR_LIGHTGREY, KSTDIO_COLOR_RED));
        kset_color(KSTDIO_MKCOLOR(KSTDIO_COLOR_LIGHTGREY, KSTDIO_COLOR_RED));
        kprint("[pid=");
        kprint_int(get_pid());
        kprint("] Exception #");
        kprint_int(vec);
        kprint(": ");
        kprint(cpu_exception_strings[vec]);
        kprint("\n");
        kprint("-> errorcode=");
        kprint_hex(frame.errorcode);
        kprint(", frame=");
        kprint_hex(&frame);
        kprint("\n-> cs:eip=");
        kprint_hex(frame.cs);
        kprint(":");
        kprint_hex(frame.eip);
        kprint(", eflags=");
        kprint_hex(frame.eflags);
        if ((frame.cs & 0b11) != 0)
        {
            kprint("\n-> ss:esp=");
            kprint_hex(frame.ss);
            kprint(":");
            kprint_hex(frame.esp);
        }
        kprint("\n-> eax=");
        kprint_hex(frame.eax);
        kprint(", ebx=");
        kprint_hex(frame.ebx);
        kprint(", ecx=");
        kprint_hex(frame.ecx);
        kprint(", edx=");
        kprint_hex(frame.edx);
        kprint("\n-> ebp=");
        kprint_hex(frame.ebp);
        kprint(", esi=");
        kprint_hex(frame.esi);
        kprint(", edi=");
        kprint_hex(frame.edi);
        kprint(", isr_esp=");
        kprint_hex(frame.isr_esp);
        kprint("\n");
        for (;;)
        {
            asm("cli");
            asm("hlt");
        }
    }
    else if (vec >= INTERRUPT_VECTOR_IRQ0 && vec <= INTERRUPT_VECTOR_IRQ7)
    {
        irq_dispatch(vec - INTERRUPT_VECTOR_IRQ0, &frame);
    }
    else if (vec >= INTERRUPT_VECTOR_IRQ8 && vec <= INTERRUPT_VECTOR_IRQ15)
    {
        irq_dispatch(vec - INTERRUPT_VECTOR_IRQ8, &frame);
    }
}

inline void irq_dispatch(uint8_t irq, interrupt_frame_t *frame)
{
    if (irq_handlers[irq])
    {
        irq_handlers[irq](irq, frame);
    }
    else
    {
        if (irq >= 1)
        {
            kprint("[KDEBUG] No handler for IRQ #");
            kprint_int(irq);
            kprint("\n");
        }
    }
    send_eoi(irq);
}

void register_irq_handler(uint8_t irq, irq_handler_t handler)
{
    irq_handlers[irq] = handler;
}

inline void enable_interrupt()
{
    asm("sti");
}

inline void disable_interrupt()
{
    asm("cli");
}