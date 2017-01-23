#ifndef _WDOS_KERNEL_TTY_H_
#define _WDOS_KERNEL_TTY_H_

#include <runtime/types.h>
#include <asm.h>

#define TTY_COLOR_BLACK        0x0
#define TTY_COLOR_BLUE         0x1
#define TTY_COLOR_GREEN        0x2
#define TTY_COLOR_CYAN         0x3
#define TTY_COLOR_RED          0x4
#define TTY_COLOR_MAGENTA      0x5
#define TTY_COLOR_BROWN        0x6
#define TTY_COLOR_LIGHTGREY    0x7
#define TTY_COLOR_DARKGREY     0x8
#define TTY_COLOR_LIGHTBLUE    0x9
#define TTY_COLOR_LIGHTGREEN   0xA
#define TTY_COLOR_LIGHTCYAN    0xB
#define TTY_COLOR_LIGHTRED     0xC
#define TTY_COLOR_LIGHTMAGENTA 0xD
#define TTY_COLOR_LIGHTBROWN   0xE
#define TTY_COLOR_WHITE        0xF

#define TTY_MKCOLOR(f, b) (((b) << 4) | (f))
#define TTY_COLOR_DEFAULT TTY_MKCOLOR(TTY_COLOR_LIGHTGREY, TTY_COLOR_BLACK)

#define TTY_COUNT 8

#define TTY_CMD_SET_COLOR 0x01
#define TTY_SET_COLOR "\001"
#define TTY_DEFAULT_COLOR "\007"
#define TTY_OK_COLOR "\002"
#define TTY_FAIL_COLOR "\004"

#define TTY_SET_DEFAULT_COLOR "\001\007"
#define TTY_SET_OK_COLOR "\001\002"
#define TTY_SET_FAIL_COLOR "\001\004"

typedef uint8_t tty_color_t;
typedef uint16_t tty_colored_char_t;

typedef struct tty
{
    spinlock_t lock;
    size_t x, y;
    tty_color_t color;
    tty_colored_char_t *mem;
} tty_t;

typedef struct tty_driver
{
    size_t width, height;
    void (*init)(tty_t *, size_t);
    void (*rerender)(tty_t *);
    void (*switch_handler)(tty_t *);
    void (*update_cursor_location)(tty_t *);
    void (*set_char)(tty_t *, size_t, size_t, char, tty_color_t);
} tty_driver_t;

extern tty_t *const default_tty;

void init_tty();
tty_t *tty_current_screen();
tty_t *tty_current_process();
void tty_init(tty_t *tty);
void tty_enter(tty_t *tty);
void tty_leave(tty_t *tty);
tty_t *tty_select(size_t id);
void tty_switch(tty_t *tty);
void tty_set_current(tty_t *tty);
void tty_update_cursor_location();
void tty_clear(tty_t *tty);
void tty_fill_color(tty_t *tty, tty_color_t color);
void tty_newline(tty_t *tty);
void tty_scroll(tty_t *tty);
void tty_set_char(tty_t *tty, size_t x, size_t y, char ch, tty_color_t color);
void tty_set_string(tty_t *tty, size_t x_offset, size_t y_offset,
                    size_t width, const char *str, tty_color_t color);
uint32_t tty_print(tty_t *tty, const char *str);

void ktty_enter();
void ktty_leave();
void kclear();
void kfill_color(tty_color_t color);
tty_color_t kget_color();
void kset_color(tty_color_t color);
void kput_char(char ch);
uint32_t kprint(const char *str);
uint8_t kprint_int(int32_t x);
uint8_t kprint_uint(uint32_t x);
uint8_t kprint_hex(uint32_t x);
uint8_t kprint_bin(uint32_t x);
uint8_t kprint_hex_long(uint64_t x);
uint8_t kprint_byte(uint8_t x);
uint32_t kprint_ok_fail(const char *str, bool ok);
void kset_char(size_t x, size_t y, char ch, tty_color_t color);
void kset_string(size_t x_offset, size_t y_offset, size_t width,
                 const char *str, tty_color_t color);

#endif // _WDOS_KERNEL_TTY_H_
