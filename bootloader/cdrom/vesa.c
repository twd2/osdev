#include "vesa.h"

static inline uint32_t abs(int x)
{
    return x > 0 ? x : -x;
}

uint32_t mode_diff(uint16_t target_width, uint16_t target_height, uint8_t target_depth,
                   uint16_t width, uint16_t height, uint8_t depth)
{
    uint32_t diff = 0;
    diff += abs(target_width - width) * 100000 / target_width;
    diff += abs(target_height - height) * 100000 / target_height;
    diff += abs(target_depth - depth) * 1000000 / target_depth;

    if (!(target_width == width && target_height == height))
    {
        diff += 0x10000000; // resolution first
    }
    return diff;
}
