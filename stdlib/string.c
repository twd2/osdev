#include "string.h"

uint8_t utoh(uint32_t x, char *buffer)
{
#define HEX_COUNT (sizeof(x) * 2)
    for (int8_t i = HEX_COUNT - 1; i >= 0; --i)
    {
        buffer[i] = (x & 0xF) + '0';
        if (buffer[i] > '9')
        {
            buffer[i] = buffer[i] - '9' + 'A' - 1;
        }
        x >>= 4;
    }
    buffer[HEX_COUNT] = 0;
    return HEX_COUNT;
#undef HEX_COUNT
}

uint8_t utob(uint32_t x, char *buffer)
{
#define BIT_COUNT (sizeof(x) * 8)
    for (uint8_t i = 0; i < BIT_COUNT; ++i)
    {
        buffer[i] = (x & 0x80000000) ? '1' : '0';
        x <<= 1;
    }
    buffer[BIT_COUNT] = 0;
    return BIT_COUNT;
#undef BIT_COUNT
}

uint8_t itos(int32_t x, char *buffer)
{
    if (!x)
    {
        buffer[0] = '0';
        buffer[1] = 0;
        return 1;
    }
    if (x == -2147483648)
    {
        buffer[0] = '-';
        buffer[1] = '2';
        buffer[2] = '1';
        buffer[3] = '4';
        buffer[4] = '7';
        buffer[5] = '4';
        buffer[6] = '8';
        buffer[7] = '3';
        buffer[8] = '6';
        buffer[9] = '4';
        buffer[10] = '8';
        buffer[11] = 0;
        return 11;
    }
    if (x < 0)
    {
        buffer[0] = '-';
        ++buffer;
        x = -x;
    }
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
    
    buffer[i] = 0;
    return i;
}
