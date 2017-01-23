#include "vga.h"

#include <io.h>
#include <asm.h>
#include <tty.h>

static tty_driver_t vga_tty_driver =
{
    .width = VGA_WIDTH,
    .height = VGA_HEIGHT,
    .init = &vga_tty_init,
    .rerender = NULL,
    .switch_handler = &vga_tty_switch_handler,
    .update_cursor_location = &vga_tty_update_cursor_location,
    .set_char = NULL
};

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

    restore_flags(flags);
}

void vga_tty_init(tty_t *tty, size_t i)
{
    tty->mem = (tty_colored_char_t *)(VGA_GRAPHICS_MEMORY_START_ADDRESS + (i << 12)); // 4K
}

void vga_tty_switch_handler(tty_t *tty)
{
    vga_set_start_address(tty->mem);
}

void vga_tty_update_cursor_location(tty_t *tty)
{
    // TODO
    // vga_set_cursor_location(tty->x, tty->y);
}

const tty_driver_t *vga_get_tty_driver()
{
    return (const tty_driver_t *)&vga_tty_driver;
}
