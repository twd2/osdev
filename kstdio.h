#ifndef _WDOS_KERNEL_KSTDIO_H_
#define _WDOS_KERNEL_KSTDIO_H_

#include <runtime/types.h>

#define KSTDIO_COLOR_BLACK        0x0
#define KSTDIO_COLOR_BLUE         0x1
#define KSTDIO_COLOR_GREEN        0x2
#define KSTDIO_COLOR_CYAN         0x3
#define KSTDIO_COLOR_RED          0x4
#define KSTDIO_COLOR_MAGENTA      0x5
#define KSTDIO_COLOR_BROWN        0x6
#define KSTDIO_COLOR_LIGHTGREY    0x7
#define KSTDIO_COLOR_DARKGREY     0x8
#define KSTDIO_COLOR_LIGHTBLUE    0x9
#define KSTDIO_COLOR_LIGHTGREEN   0xA
#define KSTDIO_COLOR_LIGHTCYAN    0xB
#define KSTDIO_COLOR_LIGHTRED     0xC
#define KSTDIO_COLOR_LIGHTMAGENTA 0xD
#define KSTDIO_COLOR_LIGHTBROWN   0xE
#define KSTDIO_COLOR_WHITE        0xF

#define KSTDIO_MKCOLOR(f, b) (((b) << 4) | (f))
#define KSTDIO_COLOR_DEFAULT KSTDIO_MKCOLOR(KSTDIO_COLOR_LIGHTGREY, KSTDIO_COLOR_BLACK)

#define KSTDIO_WIDTH  80
#define KSTDIO_HEIGHT 25

void enter_kstdio();
void leave_kstdio();
void cls();
void cls_color(uint8_t color);
void kset_color(uint8_t color);
void kput_char(char ch);
uint32_t kprint(const char *str);
uint8_t kprint_int(int32_t x);
uint8_t kprint_hex(uint32_t x);
uint8_t kprint_bin(uint32_t x);
uint32_t kprint_ok_fail(const char *str, bool ok);
void set_char(uint8_t x, uint8_t y, char ch, uint8_t color);
void set_string(uint8_t x_offset, uint8_t y_offset, uint8_t width, const char *str, uint8_t color);
void scroll();

#endif // _WDOS_KERNEL_KSTDIO_H_
