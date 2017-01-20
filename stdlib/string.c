#include "string.h"
#include "memory.h"

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

uint8_t ultoh(uint64_t x, char *buffer)
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

uint8_t ultob(uint64_t x, char *buffer)
{
#define BIT_COUNT (sizeof(x) * 8)
    for (uint8_t i = 0; i < BIT_COUNT; ++i)
    {
        buffer[i] = (x & 0x8000000000000000ULL) ? '1' : '0';
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
        buffer[1] = '\0';
        return 1;
    }

    if (x == -2147483648)
    {
        return strcpy(buffer, "-2147483648");
    }

    if (x > 0)
    {
        return utos((uint32_t)x, buffer);
    }
    else // x < 0
    {
        buffer[0] = '-';
        return utos((uint32_t)(-x), &buffer[1]) + 1;
    }
}

uint8_t utos(uint32_t x, char *buffer)
{
    if (!x)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return 1;
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
    
    buffer[i] = '\0';
    return i;
}

uint8_t btoh(uint8_t x, char *buffer)
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

uint32_t strlen(const char *src)
{
    uint32_t length = 0;
    while (*src)
    {
        ++length;
        ++src;
    }
    return length;
}

uint32_t strcpy(char *dest, const char *src)
{
    uint32_t length = 0;
    while (*src)
    {
        *dest = *src;
        ++length;
        ++src;
        ++dest;
    }
    *dest = '\0';
    return length;
}

const char *strsplit(const char *str, char delim, char *out_buffer)
{
    while (*str && *str != delim)
    {
        *out_buffer = *str;
        ++str;
        ++out_buffer;
    }
    *out_buffer = 0;
    if (!*str)
    {
        return NULL; // no more
    }
    else if (*str == delim)
    {
        return str + 1;
    }
    return NULL;
}
