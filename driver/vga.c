#include "vga.h"
#include <io.h>

void init_vga()
{

}

void vga_set_start_address(uint16_t *gm)
{
    uint16_t start_address =
        (uint16_t)(((uintptr_t)gm - (uintptr_t)VGA_GRAPHICS_MEMORY_START_ADDRESS) >> 1);

    ureg_t flags;
    save_flags(flags);
    cli();

    outb(CRTC_ADDRESS, CRTC_START_ADDRESS_HIGH);
    io_delay();
    outb(CRTC_DATA, (start_address >> 8) & 0xFF);
    io_delay();
    outb(CRTC_ADDRESS, CRTC_START_ADDRESS_LOW);
    io_delay();
    outb(CRTC_DATA, start_address & 0xFF);
    io_delay();

    restore_flags(flags);
}

void vga_set_cursor_location(uint8_t x, uint8_t y)
{
    uint16_t location = y * VGA_WIDTH + x;

    ureg_t flags;
    save_flags(flags);
    cli();

    outb(CRTC_ADDRESS, CRTC_CURSOR_LOCATION_HIGH);
    io_delay();
    outb(CRTC_DATA, (location >> 8) & 0xFF);
    io_delay();
    outb(CRTC_ADDRESS, CRTC_CURSOR_LOCATION_LOW);
    io_delay();
    outb(CRTC_DATA, location & 0xFF);
    io_delay();

    // restore eflags
    asm volatile ("push %0\n"
                  "popf"
                  :
                  : "g"(eflags));
}
