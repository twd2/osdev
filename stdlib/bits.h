#ifndef _WDOS_STDLIB_BITS_H_
#define _WDOS_STDLIB_BITS_H_

#include <runtime/types.h>

extern const uint8_t debruijn[32];

#define bits_log2(x) (debruijn[((uint32_t)(((x) & -(x)) * 0x077CB531U)) >> 27])

// for 64 bit
#define bits_llog2(x) (((x) >> 32) ? \
    (bits_log2((x) >> 32) + 32) : \
    (bits_log2((x) & 0xFFFFFFFFU)))

#define is_power_of_2(x) (!((x) & ((x) - 1)))

#define bits_mul(a, b) ((a) << bits_log2(b))
#define bits_div(a, b) ((a) >> bits_log2(b))
#define bits_mod(a, b) ((a) & ((b) - 1))

static inline uint32_t next_power_of_2(uint32_t x)
{
    if (is_power_of_2(x))
    {
        return x;
    }

    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    return x;
}

#endif // _WDOS_STDLIB_BITS_H_
