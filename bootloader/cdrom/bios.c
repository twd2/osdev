#include "bios.h"

#define FUNCTION_PRINT 1
#define FUNCTION_GETCHAR 2
#define FUNCTION_READ_SECTOR 3
#define FUNCTION_MEMORY_MAP 4

int bios_function(uint32_t arg1, uint32_t arg2, uint32_t arg3);

inline void linear_to_logic(void *addr, uint16_t *out_seg, uint16_t *out_offset)
{
    // linear to seg:offset
    // 0x8abcd -> 0x8abc:000d
    *out_seg = ((uint32_t)addr & 0xFFFFFFF0) >> 4;
    *out_offset = (uint32_t)addr & 0x000F;
}

int print(const char *str)
{
    return bios_function(FUNCTION_PRINT, (uint32_t)str, 0);
}

uint8_t getchar()
{
    static uint8_t buffer[2];
    bios_function(FUNCTION_GETCHAR, (uint32_t)buffer, 0);
    return buffer[0];
}

// count <= 64
static int _read_sector(uint8_t dev, uint32_t lba, void *buffer, uint16_t count)
{
    static dap_t dap;
    dap.size = sizeof(dap);
    dap.reserved = 0;
    dap.count = count;
    dap.lba_low = lba;
    dap.lba_high = 0;
    linear_to_logic(buffer, &dap.dest_seg, &dap.dest_offset);
    return bios_function(FUNCTION_READ_SECTOR, dev, (uint32_t)&dap);
}

int read_sector(uint8_t dev, uint32_t lba, void *buffer, uint16_t count)
{
    uint16_t sector_size = 512;
    if (dev >= 0xe0) // cdrom
    {
        sector_size = 2048;
    }
    uint8_t *buffer8 = buffer;
    uint32_t lba_rest = lba + (count & ~31);
    uint8_t *buffer_rest = buffer8 + (count & ~31) * sector_size;
    for (int i = 0; i < (count >> 5); ++i)
    {
        int ret = _read_sector(dev, lba + (i << 5), buffer8 + (i << 5) * sector_size, 32);
        if (ret != OK)
        {
            return ret;
        }
    }
    return _read_sector(dev, lba_rest, buffer_rest, count & 31); // the last several sectors
}

int read_memory_map(bios_memory_map_t *buffer)
{
    static uint32_t size;
    size = sizeof(bios_memory_map_t); // in size, out count
    if (bios_function(FUNCTION_MEMORY_MAP, (uint32_t)buffer, (uint32_t)&size) != OK)
    {
        return 0;
    }
    return size;
}
