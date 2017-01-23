#ifndef _WDOS_KERNEL_BOOT_H_
#define _WDOS_KERNEL_BOOT_H_

#include <runtime/types.h>
#include <multiboot.h>

#define MULTIBOOT_NEEDED_FLAGS (MULTIBOOT_INFO_MEMORY | MULTIBOOT_INFO_BOOTDEV | \
                                MULTIBOOT_INFO_CMDLINE | MULTIBOOT_INFO_MEM_MAP)
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
