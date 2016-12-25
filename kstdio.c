#include "kstdio.h"
#include <runtime/types.h>

static uint8_t x_pos = 0, y_pos = 0;
static uint8_t current_color = KSTDIO_COLOR_DEFAULT;
static uint16_t *gm = (uint16_t *)0xb8000; // graphics memory, TODO: paging
static bool screen_lock = false;

void enter_kstdio()
{
    while (screen_lock);
    // if process switch here?
    screen_lock = true;
}

void leave_kstdio()
{
    screen_lock = false;
}

void cls()
{
    cls_color(0);
}

void cls_color(uint8_t color)
{
    uint32_t *gm_int = (uint32_t *)gm;
    for (uint32_t i = 0; i < 80 * 25 / 2; ++i)
    {
        gm_int[i] = 0x00200020 | ((uint32_t)color << 24) | ((uint32_t)color << 8);
    }
    x_pos = 0;
    y_pos = 0;
}

void newline()
{
    x_pos = 0;
    ++y_pos;
    if (y_pos >= KSTDIO_HEIGHT)
    {
        scroll();
        y_pos = KSTDIO_HEIGHT - 1;
    }
}

void kset_color(uint8_t color)
{
    current_color = color;
}

void kput_char(char ch)
{
    char str[2] = { 0 };
    str[0] = ch;
    kprint(str);
}

uint32_t kprint(const char *str)
{
    uint32_t i = 0;
    for (; str[i] != '\0'; ++i)
    {
        if (str[i] == 0x01) // command: set color
        {
            ++i;
            current_color = str[i];
        }
        else if (str[i] == '\n')
        {
            newline();
        }
        else
        {
            set_char(x_pos, y_pos, str[i], current_color);
            ++x_pos;
            if (x_pos >= KSTDIO_WIDTH)
            {
                newline();
            }
        }
    }
    return i;
}

uint8_t kprint_hex(uint32_t x)
{
    kprint("0x");
    char buffer[9] = { 0 };
    for (int8_t i = 7; i >= 0; --i)
    {
        buffer[i] = (x & 0xF) + '0';
        if (buffer[i] > '9')
        {
            buffer[i] = buffer[i] - '9' + 'A' - 1;
        }
        x >>= 4;
    }
    return kprint(buffer) + 2;
}

uint8_t kprint_bin(uint32_t x)
{
    kprint("0b");
    char buffer[33] = { 0 };
    for (uint8_t i = 0; i < 32; ++i)
    {
        buffer[i] = (x & 0x80000000) ? '1' : '0';
        x <<= 1;
    }
    return kprint(buffer) + 2;
}

uint8_t kprint_int(int32_t x)
{
    if (!x)
    {
        return kprint("0");
    }
    if (x < 0)
    {
        kprint("-");
        x = -x;
    }
    char buffer[11] = { 0 };
    uint8_t i = 0;
    while (x)
    {
        buffer[i] = x % 10 + '0';
        x /= 10;
        ++i;
    }

    // reverse
    for (uint8_t j = 0; j < (i >> 1); ++j)
    {
        char tmp = buffer[i - j - 1];
        buffer[i - j - 1] = buffer[j];
        buffer[j] = tmp;
    }
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
    for (uint8_t i = 0; i < KSTDIO_WIDTH - len - ok_length; ++i)
    {
        kprint(" ");
    }
    uint8_t old_color = current_color;
    kprint("[");
    if (ok)
    {
        current_color = KSTDIO_MKCOLOR(KSTDIO_COLOR_GREEN, KSTDIO_COLOR_BLACK);
        kprint("OK");
    }
    else
    {
        current_color = KSTDIO_MKCOLOR(KSTDIO_COLOR_RED, KSTDIO_COLOR_BLACK);
        kprint("FAIL");
    }
    current_color = old_color;
    kprint("]");
}

void set_char(uint8_t x, uint8_t y, char ch, uint8_t color)
{
    gm[x + y * 80] = (color << 8) | ch;
}

void set_string(uint8_t x_offset, uint8_t y_offset, uint8_t width, const char *str, uint8_t color)
{
    for (int i = 0; str[i] != '\0'; ++i)
    {
        set_char(x_offset + i % width, y_offset + i / width, str[i], color);
    }
}

void scroll()
{
    uint32_t *gm_int = (uint32_t *)gm;
    for (int i = 0; i < 80 * 24 / 2; ++i)
    {
        gm_int[i] = gm_int[i + 80 / 2];
    }
    for (int i = 80 * 24 / 2; i < 80 * 25 / 2; ++i)
    {
        gm_int[i] = 0;
    }
}
