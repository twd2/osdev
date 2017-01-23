#ifndef _WDOS_KERNEL_DRIVER_VGA_FONT_H_
#define _WDOS_KERNEL_DRIVER_VGA_FONT_H_

#include <runtime/types.h>

#define VGA_FONT_WIDTH 8
#define VGA_FONT_HEIGHT 16

extern uint8_t vga_font[256 * 16];

#define vga_font_get(ch) (&vga_font[(ch) * 16])

#endif // _WDOS_KERNEL_DRIVER_VGA_FONT_H_
