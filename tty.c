#include "tty.h"

#include <runtime/types.h>
#include <stdlib/string.h>
#include <driver/vga.h>
#include <process.h>

static tty_t ttys[TTY_COUNT];
tty_t *const default_tty = &ttys[0];
static tty_t *current_screen;
static tty_t *current_tty = NULL;

void init_tty()
{
    init_vga();
    for (int i = 0; i < TTY_COUNT; ++i)
    {
        tty_init(&ttys[i]);
        ttys[i].mem = (uint16_t *)(VGA_GRAPHICS_MEMORY_START_ADDRESS + (i << 12)); // 4K
    }
    tty_switch(default_tty);
}

tty_t *tty_current_screen()
{
    return current_screen;
}

tty_t *tty_current_process()
{
    if (current_tty)
    {
        return current_tty;
    }
    process_t *current_process = process_current();
    if (current_process)
    {
        return current_process->tty;
    }
    else
    {
        return default_tty;
    }
}

void tty_init(tty_t *tty)
{
    tty->lock = false;
    tty->x = tty->y = 0;
    tty->color = TTY_COLOR_DEFAULT;
    tty->mem = NULL;
}

void tty_enter(tty_t *tty)
{
    while (tty->lock);
    // if process switch here?
    tty->lock = true;
}

void tty_leave(tty_t *tty)
{
    tty->lock = false;
}

void tty_switch(tty_t *tty)
{
    current_screen = tty;
    vga_set_start_address(tty->mem);
    tty_update_cursor_location();
}

void tty_set_current(tty_t *tty)
{
    current_tty = tty;
}

void tty_update_cursor_location()
{
    vga_set_cursor_location(current_screen->x, current_screen->y);
}

void tty_clear(tty_t *tty)
{
    tty_fill_color(tty, 0);
}

void tty_fill_color(tty_t *tty, uint8_t color)
{
    uint32_t *gm_int = (uint32_t *)tty->mem;
    for (uint32_t i = 0; i < 80 * 25 / 2; ++i)
    {
        gm_int[i] = 0x00200020 | ((uint32_t)color << 24) | ((uint32_t)color << 8);
    }
    tty->x = tty->y = 0;
    /* TODO if (tty == current_screen)
    {
        tty_update_cursor_location();
    }*/
}

void tty_newline(tty_t *tty)
{
    tty->x = 0;
    ++tty->y;
    if (tty->y >= TTY_HEIGHT)
    {
        tty_scroll(tty);
        tty->y = TTY_HEIGHT - 1;
    }
}

void tty_scroll(tty_t *tty)
{
    uint32_t *gm_int = (uint32_t *)tty->mem;
    for (int i = 0; i < TTY_WIDTH * (TTY_HEIGHT - 1) / 2; ++i)
    {
        gm_int[i] = gm_int[i + TTY_WIDTH / 2];
    }
    for (int i = TTY_WIDTH * (TTY_HEIGHT - 1) / 2; i < TTY_WIDTH * TTY_HEIGHT / 2; ++i)
    {
        gm_int[i] = 0;
    }
}

inline void tty_set_char(tty_t *tty, uint8_t x, uint8_t y, char ch, uint8_t color)
{
    tty->mem[x + y * TTY_WIDTH] = (color << 8) | ch;
}

void tty_set_string(tty_t *tty, uint8_t x_offset, uint8_t y_offset,
                    uint8_t width, const char *str, uint8_t color)
{
    for (int i = 0; str[i] != '\0'; ++i)
    {
        tty_set_char(tty, x_offset + i % width, y_offset + i / width, str[i], color);
    }
}

uint32_t tty_print(tty_t *tty, const char *str)
{
    uint32_t i = 0;
    for (; str[i] != '\0'; ++i)
    {
        if (str[i] == 0x01) // command: set color
        {
            ++i;
            tty->color = str[i];
        }
        else if (str[i] == '\n')
        {
            tty_newline(tty);
        }
        else
        {
            tty_set_char(tty, tty->x, tty->y, str[i], tty->color);
            ++tty->x;
            if (tty->x >= TTY_WIDTH)
            {
                tty_newline(tty);
            }
        }
    }
    /* TODO if (tty == current_screen)
    {
        tty_update_cursor_location();
    }*/
    return i;
}

void ktty_enter()
{
    tty_enter(tty_current_process());
}

void ktty_leave()
{
    tty_leave(tty_current_process());
}

void kclear()
{
    tty_clear(tty_current_process());
}

void kfill_color(uint8_t color)
{
    tty_fill_color(tty_current_process(), color);
}

uint8_t kget_color()
{
    return tty_current_process()->color;
}

void kset_color(uint8_t color)
{
    tty_current_process()->color = color;
}

void kput_char(char ch)
{
    char str[2] = { 0 };
    str[0] = ch;
    kprint(str);
}

uint32_t kprint(const char *str)
{
    return tty_print(tty_current_process(), str);
}

uint8_t kprint_hex(uint32_t x)
{
#define HEX_COUNT (sizeof(x) * 2)
    kprint("0x");
    char buffer[HEX_COUNT + 1] = { 0 };
    utoh(x, buffer);
#undef HEX_COUNT
    return kprint(buffer) + 2;
}

uint8_t kprint_bin(uint32_t x)
{
#define BIT_COUNT (sizeof(x) * 8)
    kprint("0b");
    char buffer[BIT_COUNT + 1] = { 0 };
    utob(x, buffer);
#undef BIT_COUNT
    return kprint(buffer) + 2;
}

uint8_t kprint_int(int32_t x)
{
    char buffer[12] = { 0 }; // -2 147 483 647, 11 chars
    itos(x, buffer);
    return kprint(buffer);
}

uint32_t kprint_ok_fail(const char *str, bool ok)
{
    uint32_t len = kprint(str);
    uint8_t ok_length = 4; // [OK]
    if (!ok)
    {
        ok_length = 6; // [FAIL]
    }
    for (uint8_t i = 0; i < TTY_WIDTH - len - ok_length; ++i)
    {
        kprint(" ");
    }
    uint8_t old_color = kget_color();
    kprint("[");
    if (ok)
    {
        kset_color(TTY_MKCOLOR(TTY_COLOR_GREEN, TTY_COLOR_BLACK));
        kprint("OK");
    }
    else
    {
        kset_color(TTY_MKCOLOR(TTY_COLOR_RED, TTY_COLOR_BLACK));
        kprint("FAIL");
    }
    kset_color(old_color);
    kprint("]");
    return TTY_WIDTH;
}
