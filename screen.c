#include "screen.h"

static int x_pos = 0, y_pos = 0;
static unsigned char current_color = SCREEN_COLOR_DEFAULT;
static unsigned short *gm = (unsigned short *)0xb8000; // graphics memory, TODO: paging

void cls()
{
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i)
    {
        gm[i] = 0;
    }
}

void newline()
{
    x_pos = 0;
    ++y_pos;
    if (y_pos == SCREEN_HEIGHT)
    {
        scroll();
        --y_pos;
    }
}

void kset_color(unsigned char color)
{
    current_color = color;
}

void kput_char(char ch)
{
    char str[2] = { 0 };
    str[0] = ch;
    kprint(str);
}

int kprint(char *str)
{
    int i = 0;
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
            if (x_pos == SCREEN_WIDTH)
            {
                newline();
            }
        }
    }
    return i;
}

int kprint_hex(int x)
{
    kprint("0x");
    char buffer[9] = { 0 };
    for (int i = 7; i >= 0; --i)
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

int kprint_int(int x)
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
    int i = 0;
    while (x)
    {
        buffer[i] = x % 10 + '0';
        x /= 10;
        ++i;
    }

    // reverse
    for (int j = 0; j < (i >> 1); ++j)
    {
        char tmp = buffer[i - j - 1];
        buffer[i - j - 1] = buffer[j];
        buffer[j] = tmp;
    }
    return kprint(buffer);
}

void set_char(int x, int y, char ch, unsigned char color)
{
    gm[x + y * 80] = (color << 8) | ch;
}

void set_string(int x_offset, int y_offset, int width, char *str, unsigned char color)
{
    for (int i = 0; str[i] != '\0'; ++i)
    {
        set_char(x_offset + i % width, y_offset + i / width, str[i], color);
    }
}

void scroll()
{
    unsigned int *gm_int = (unsigned int *)gm;
    for (int i = 0; i < 80 * 24 / 2; ++i)
    {
        gm_int[i] = gm_int[i + 80 / 2];
    }
    for (int i = 80 * 24 / 2; i < 80 * 25 / 2; ++i)
    {
        gm_int[i] = 0;
    }
}