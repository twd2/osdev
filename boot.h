#ifndef _WDOS_KERNEL_BOOT_H_
#define _WDOS_KERNEL_BOOT_H_

#include <runtime/types.h>

#define MULTIBOOT_NEEDED_FLAGS 0b1000111
#define MEMORY_TYPE_USABLE 1
#define MEMORY_TYPE_RESERVED 2

typedef struct memory_map_long
{
    union
    {
        struct
        {
            uint32_t base_low;
            uint32_t base_high;
        };
        uint64_t base;
    };
    union
    {
        struct
        {
            uint32_t length_low;
            uint32_t length_high;
        };
        uint64_t length;
    };
} memory_map_long_t;

#endif // _WDOS_KERNEL_BOOT_H_
