#include <interrupt.h>

void interrupt_handler(uint32_t i, interrupt_frame_t frame)
{
    //cls();
    kprint("on interrupt i=");
    kprint_hex(i);
    kprint(", errorcode=");
    kprint_hex(frame.errorcode);
    kprint(", eip=");
    kprint_hex(frame.eip);
    kprint(", cs=");
    kprint_hex(frame.cs);
    kprint(", eax=");
    kprint_hex(frame.eax);
    kprint("\n");
    if (i != 0x80)
        for(;;);
}
