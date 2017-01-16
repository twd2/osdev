#include "memory.h"

uint32_t memset(void *dest, uint8_t value, uint32_t num)
{
    uint32_t value32 = ((uint32_t)value << 24) | ((uint32_t)value << 16) |
                       ((uint32_t)value << 8) | ((uint32_t)value << 0);
    uint32_t *dest32 = dest;
    uint8_t *dest8 = dest + (num & ~3);
    for (uint32_t i = 0; i < (num >> 2); ++i)
    {
        dest32[i] = value32;
    }
    for (uint32_t i = 0; i < (num & 3); ++i)
    {
        dest8[i] = value;
    }
    return num;
}

uint32_t memcpy(void *dest, const void *src, uint32_t num)
{
    uint32_t *dest32 = dest;
    const uint32_t *src32 = src;
    uint8_t *dest8 = dest + (num & ~3);
    const uint8_t *src8 = src + (num & ~3);
    for (uint32_t i = 0; i < (num >> 2); ++i)
    {
        dest32[i] = src32[i];
    }
    for (uint32_t i = 0; i < (num & 3); ++i)
    {
        dest8[i] = src8[i];
    }
    return num;
}

int memcmp(const void *a, const void *b, uint32_t num)
{
    const uint32_t *a32 = a;
    const uint32_t *b32 = b;
    const uint8_t *a8 = a + (num & ~3);
    const uint8_t *b8 = b + (num & ~3);
    for (uint32_t i = 0; i < (num >> 2); ++i)
    {
        if (a32[i] != b32[i])
        {
            return 1;
        }
    }
    for (uint32_t i = 0; i < (num & 3); ++i)
    {
        if (a8[i] != b8[i])
        {
            return 1;
        }
    }
    return 0;
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
    return memcpy(dest, src, strlen(src) + 1) - 1;
}

char *strsplit(const char *str, char delim, char *out_buffer)
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
}
