#include "tty.h"

#include <runtime/types.h>
#include <stdlib/string.h>
#include <thread.h>

static tty_t ttys[TTY_COUNT];
tty_t *const default_tty = &ttys[0];
static volatile tty_t *current_screen;
static tty_t *current_tty = NULL;

static size_t tty_width, tty_height;
static tty_driver_t tty_driver;

void init_tty(tty_driver_t *driver)
{
    memcpy(&tty_driver, driver, sizeof(tty_driver_t));
    tty_width = tty_driver.width;
    tty_height = tty_driver.height;

    for (int i = 0; i < TTY_COUNT; ++i)
    {
        tty_init(&ttys[i]);
        if (tty_driver.init)
        {
            tty_driver.init(&ttys[i], i);
        }
    }
    tty_switch(default_tty);
}

tty_t *tty_current_screen()
{
    return current_screen;
}

tty_t *tty_current_process()
{
    // TODO: process or thread?
    if (current_tty)
    {
        return current_tty;
    }
    thread_t *current_thread = thread_current();
    if (current_thread)
    {
        return current_thread->tty;
    }
    else
    {
        return default_tty;
    }
}

void tty_init(tty_t *tty)
{
    spinlock_init(&tty->lock);
    tty->x = tty->y = 0;
    tty->color = TTY_COLOR_DEFAULT;
    tty->mem = NULL;
}

void tty_enter(tty_t *tty)
{
    spinlock_wait_and_lock(&tty->lock);
}

void tty_leave(tty_t *tty)
{
    spinlock_release(&tty->lock);
}

tty_t *tty_select(size_t id)
{
    return &ttys[id];
}

void tty_switch(tty_t *tty)
{
    if (current_screen == tty)
    {
        return;
    }

    current_screen = tty;
    if (tty_driver.switch_handler)
    {
        tty_driver.switch_handler(tty);
    }
    tty_update_cursor_location();
}

void tty_set_current(tty_t *tty)
{
    current_tty = tty;
}

void tty_update_cursor_location()
{
    if (tty_driver.update_cursor_location)
    {
        tty_driver.update_cursor_location(current_screen);
    }
}

void tty_clear(tty_t *tty)
{
    tty_fill_color(tty, 0);
}

void tty_fill_color(tty_t *tty, tty_color_t color)
{
    uint32_t *gm_int = (uint32_t *)tty->mem;
    for (uint32_t i = 0; i < tty_width * tty_height / 2; ++i)
    {
        gm_int[i] = 0x00200020 | ((uint32_t)color << 24) | ((uint32_t)color << 8);
    }
    tty->x = tty->y = 0;
    if (tty == current_screen)
    {
        if (tty_driver.rerender)
        {
            tty_driver.rerender(tty);
        }
        tty_update_cursor_location();
    }
}

void tty_newline(tty_t *tty)
{
    tty->x = 0;
    ++tty->y;
    if (tty->y >= tty_height)
    {
        tty_scroll(tty);
        tty->y = tty_height - 1;
    }
}

void tty_scroll(tty_t *tty)
{
    uint32_t *gm_int = (uint32_t *)tty->mem;
    for (int i = 0; i < tty_width * (tty_height - 1) / 2; ++i)
    {
        gm_int[i] = gm_int[i + tty_width / 2];
    }
    for (int i = tty_width * (tty_height - 1) / 2; i < tty_width * tty_height / 2; ++i)
    {
        gm_int[i] = 0;
    }
    if (tty == current_screen)
    {
        if (tty_driver.rerender)
        {
            tty_driver.rerender(tty);
        }
        tty_update_cursor_location();
    }
}

inline void tty_set_char(tty_t *tty, size_t x, size_t y, char ch, tty_color_t color)
{
    tty->mem[x + y * tty_width] = (color << 8) | ch;
    if (tty == current_screen)
    {
        if (tty_driver.set_char)
        {
            tty_driver.set_char(tty, x, y, ch, color);
        }
    }
}

void tty_set_string(tty_t *tty, size_t x_offset, size_t y_offset,
                    size_t width, const char *str, tty_color_t color)
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
        if (str[i] == TTY_CMD_SET_COLOR)
        {
            ++i;
            if (str[i] == '\0')
            {
                break;
            }
            tty->color = str[i];
        }
        else if (str[i] == '\n')
        {
            // implies \r
            tty_newline(tty);
        }
        else if (str[i] == '\r')
        {
            tty->x = 0;
        }
        else if (str[i] == '\b')
        {
            if (tty->x)
            {
                --tty->x;
            }
        }
        else
        {
            tty_set_char(tty, tty->x, tty->y, str[i], tty->color);
            ++tty->x;
            if (tty->x >= tty_width)
            {
                tty_newline(tty);
            }
        }
    }
    if (tty == current_screen)
    {
        tty_update_cursor_location();
    }
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

void kfill_color(tty_color_t color)
{
    tty_fill_color(tty_current_process(), color);
}

uint8_t kget_color()
{
    return tty_current_process()->color;
}

void kset_color(tty_color_t color)
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
    char buffer[HEX_COUNT + 1] = { '\0' };
    utoh(x, buffer);
#undef HEX_COUNT
    return kprint(buffer) + 2;
}

uint8_t kprint_bin(uint32_t x)
{
#define BIT_COUNT (sizeof(x) * 8)
    kprint("0b");
    char buffer[BIT_COUNT + 1] = { '\0' };
    utob(x, buffer);
#undef BIT_COUNT
    return kprint(buffer) + 2;
}

uint8_t kprint_int(int32_t x)
{
    char buffer[12] = { '\0' }; // -2 147 483 647, 11 chars
    itos(x, buffer);
    return kprint(buffer);
}

uint8_t kprint_uint(uint32_t x)
{
    char buffer[11] = { '\0' }; // 4 294 967 295, 10 chars
    utos(x, buffer);
    return kprint(buffer);
}

uint8_t kprint_hex_long(uint64_t x)
{
#define HEX_COUNT (sizeof(x) * 2)
    kprint("0x");
    char buffer[HEX_COUNT + 1] = { '\0' };
    ultoh(x, buffer);
#undef HEX_COUNT
    return kprint(buffer) + 2;
}

uint8_t kprint_byte(uint8_t x)
{
#define HEX_COUNT (sizeof(x) * 2)
    kprint("0x");
    char buffer[HEX_COUNT + 1] = { '\0' };
    btoh(x, buffer);
#undef HEX_COUNT
    return kprint(buffer) + 2;
}

uint32_t kprint_ok_fail(const char *str, bool ok)
{
    uint32_t len = kprint(str);
    uint8_t ok_length = 4; // [OK]
    if (!ok)
    {
        ok_length = 6; // [FAIL]
    }
    for (uint8_t i = 0; i < tty_width - len - ok_length; ++i)
    {
        kprint(" ");
    }
    uint8_t old_color = kget_color();
    if (ok)
    {
        kprint("[" TTY_SET_OK_COLOR "OK" TTY_SET_DEFAULT_COLOR "]");
    }
    else
    {
        kprint("[" TTY_SET_FAIL_COLOR "FAIL" TTY_SET_DEFAULT_COLOR "]");
    }
    kset_color(old_color);
    return tty_width;
}
