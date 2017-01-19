#ifndef _WDOS_BOOTLOADER_CDROM_STAGE2_CONFIG_H_
#define _WDOS_BOOTLOADER_CDROM_STAGE2_CONFIG_H_

#include <runtime/types.h>

#define NDEBUG 1
#define STAGE2_LOAD_ADDRESS 0x10000
#define SEGMENT_LIMIT 0xffff
#define LOW_MEMORY_BASE 0
#define LOW_MEMORY_LIMIT (0xa0000 - 1)
#define HIGH_MEMORY_BASE 0x100000
#define MULTIBOOT_UNSUPPORTED_FLAGS (~0b10) // only support mem_* and mmap_*
#define MULTIBOOT_FLAGS 0b1000111 // has mem_*, boot_device, cmdline and mmap_*

#define MKBOOT_DEVICE(dev, part1, part2, part3) \
    ((((uint32_t)(dev) & 0xff) << 24) | (((uint32_t)(part1) & 0xff) << 16) | \
     (((uint32_t)(part2) & 0xff) << 8) | ((uint32_t)(part2) & 0xff))
#define UNUSED_PARTITION 0xff

typedef void (*kernel_entry_t)();

#endif // _WDOS_BOOTLOADER_CDROM_STAGE2_CONFIG_H_
