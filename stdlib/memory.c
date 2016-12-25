#include "memory.h"

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
