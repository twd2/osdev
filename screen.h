#ifndef _WDOS_KERNEL_SCREEN_H_
#define _WDOS_KERNEL_SCREEN_H_

#define SCREEN_COLOR_BLACK        0x0
#define SCREEN_COLOR_BLUE         0x1
#define SCREEN_COLOR_GREEN        0x2
#define SCREEN_COLOR_CYAN         0x3
#define SCREEN_COLOR_RED          0x4
#define SCREEN_COLOR_MAGENTA      0x5
#define SCREEN_COLOR_BROWN        0x6
#define SCREEN_COLOR_LIGHTGREY    0x7
#define SCREEN_COLOR_DARKGREY     0x8
#define SCREEN_COLOR_LIGHTBLUE    0x9
#define SCREEN_COLOR_LIGHTGREEN   0xA
#define SCREEN_COLOR_LIGHTCYAN    0xB
#define SCREEN_COLOR_LIGHTRED     0xC
#define SCREEN_COLOR_LIGHTMAGENTA 0xD
#define SCREEN_COLOR_LIGHTBROWN   0xE
#define SCREEN_COLOR_WHITE        0xF

#define SCREEN_COLOR_DEFAULT ((SCREEN_COLOR_BLACK << 4) | SCREEN_COLOR_LIGHTGREY)

#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25

void cls();
void kset_color(unsigned char color);
void kput_char(char ch);
int kprint(char *str);
int kprint_int(int x);
int kprint_hex(int x);
void set_char(int x, int y, char ch, unsigned char color);
void set_string(int x_offset, int y_offset, int width, char *str, unsigned char color);
void scroll();

#endif // _WDOS_KERNEL_SCREEN_H_
