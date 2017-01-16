#ifndef _WDOS_BOOTLOADER_CDROM_STAGE2_BIOS_H_
#define _WDOS_BOOTLOADER_CDROM_STAGE2_BIOS_H_

#include <runtime/types.h>

#define BLOCK_COUNT 32 // read at most 32 sectors at once

#define OK 0
#define ERROR_UNKNOWN_FUNCTION 0xffff
// ERROR_BIOS_ERROR: OTHER

typedef struct dap
{
    uint8_t size;
    uint8_t reserved;
    uint16_t count;
    uint16_t dest_offset;
    uint16_t dest_seg;
    uint32_t lba_low;
    uint32_t lba_high;
} dap_t;

#define MEMORY_TYPE_USABLE 1
#define MEMORY_TYPE_RESERVED 2
// MEMORY_TYPE_UNUSABLE: OTHER

typedef struct bios_memory_map
{
    uint32_t base_low;
    uint32_t base_high;
    uint32_t length_low;
    uint32_t length_high;
    uint32_t type;
} bios_memory_map_t;

extern uint8_t boot_device;

void linear_to_logic(void *addr, uint16_t *out_seg, uint16_t *out_offset);
int print(const char *str); // str must be in [0x10000, 0x1ffff]
uint8_t getchar();

// buffer must be in [0x00000, 0xfffff]
int read_sector(uint8_t dev, uint32_t lba, void *buffer, uint16_t count);

int read_memory_map(bios_memory_map_t *buffer); // buffer must be in [0x10000, 0x1ffff]

#endif // _WDOS_BOOTLOADER_CDROM_STAGE2_BIOS_H_
