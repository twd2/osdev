#include "bios.h"

#include <stdlib/memory.h>
#include <stdlib/string.h>

#include "util.h"

#define FUNCTION_PRINT 1
#define FUNCTION_GETCHAR 2
#define FUNCTION_READ_SECTOR 3
#define FUNCTION_MEMORY_MAP 4
#define FUNCTION_DRIVE_PARAMS 5

// bios_function.inc
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
    if (!(0x10000 <= (uint32_t)str && (uint32_t)str <= 0x1ffff))
    {
        die("Argument for str is out of range.\r\n");
    }
    return bios_function(FUNCTION_PRINT, (uint32_t)str, 0);
}

int print_hex(uint32_t x)
{
    static char buffer[sizeof(x) * 2 + 3] = { '0', 'x', 0 };
    utoh(x, &buffer[2]);
    return print(buffer);
}

int print_bin(uint32_t x)
{
    static char buffer[sizeof(x) * 8 + 3] = { '0', 'b', 0 };
    utob(x, &buffer[2]);
    return print(buffer);
}

int print_int(int32_t x)
{
    static char buffer[12] = { 0 }; // -2 147 483 647, 11 chars
    itos(x, buffer);
    return print(buffer);
}

int print_byte(uint8_t x)
{
    static char buffer[sizeof(x) * 2 + 1] = { 0 };
    btoh(x, buffer);
    return print(buffer);
}

int print_hex_long(uint64_t x)
{
    static char buffer[sizeof(x) * 2 + 3] = { '0', 'x', 0 };
    ultoh(x, &buffer[2]);
    return print(buffer);
}

uint8_t getchar()
{
    static uint8_t buffer[2];
    bios_function(FUNCTION_GETCHAR, (uint32_t)buffer, 0);
    return buffer[0];
}

// buffer must be in [0x00000, 0xfffff]
// count must be less than or equal to BIOS_DMA_MAX_LENGTH / sector_size.
inline static int _read_sector(uint8_t dev, uint32_t lba, void *buffer, uint16_t count)
{
    if (!count)
    {
        return OK;
    }
    static dap_t dap;
    dap.size = sizeof(dap);
    dap.reserved = 0;
    dap.count = count;
    dap.lba_low = lba;
    dap.lba_high = 0;
    linear_to_logic(buffer, &dap.dest_seg, &dap.dest_offset);
    return bios_function(FUNCTION_READ_SECTOR, dev, (uint32_t)&dap);
}

// buffer must be in [0x00000, 0xfffff]
// count must be less than or equal to BIOS_DMA_MAX_LENGTH / sector_size.
inline static int _read_sector_high_memory(uint8_t dev, uint32_t lba, void *dest, uint16_t count,
                                           void *buffer)
{
    const uint16_t sector_size = get_sector_size(dev);

    int ret = _read_sector(dev, lba, buffer, count);
    if (ret != OK)
    {
        return ret;
    }
    memcpy(dest, buffer, count * sector_size); // copy to high memory
    return OK;
}

int read_sector(uint8_t dev, uint32_t lba, void *buffer, uint16_t count)
{
    if ((uint32_t)buffer > 0xfffff)
    {
        die("Argument for buffer is out of range.\r\n");
    }

    const uint16_t sector_size = get_sector_size(dev);
    // how many sectors can we read at most at once?
    const uint16_t block_count = (uint16_t)(BIOS_DMA_MAX_LENGTH / (uint32_t)sector_size);
    assert(is_power_of_2(block_count));

    uint8_t *buffer8 = buffer;
    for (int i = 0; i < (count / block_count); ++i)
    {
        int ret = _read_sector(dev, lba, buffer8, block_count);
        if (ret != OK)
        {
            return ret;
        }
        lba += block_count;
        buffer8 += block_count * sector_size;
    }
    // the last several sectors
    return _read_sector(dev, lba, buffer8, count & (block_count - 1));
}

int read_sector_high_memory(uint8_t dev, uint32_t lba, void *dest, uint16_t count, void *buffer)
{
    if ((uint32_t)buffer > 0xfffff)
    {
        die("Argument for buffer is out of range.\r\n");
    }

    const uint16_t sector_size = get_sector_size(dev);
    // how many sectors can we read at most at once?
    const uint16_t block_count = (uint16_t)(BIOS_DMA_MAX_LENGTH / (uint32_t)sector_size);
    assert(is_power_of_2(block_count));

    uint8_t *dest8 = dest;
    for (int i = 0; i < (count / block_count); ++i)
    {
        int ret = _read_sector_high_memory(dev, lba, dest8, block_count, buffer);
        if (ret != OK)
        {
            return ret;
        }
        lba += block_count;
        dest8 += block_count * sector_size;
        print(".");
    }
    // the last several sectors
    return _read_sector_high_memory(dev, lba, dest8, count & (block_count - 1), buffer);
}

int read_memory_map(bios_memory_map_t *buffer)
{
    if (!(0x10000 <= (uint32_t)buffer && (uint32_t)buffer <= 0x1ffff))
    {
        die("Argument for buffer is out of range.\r\n");
    }

    static uint32_t size;
    size = sizeof(bios_memory_map_t); // in size, out count
    if (bios_function(FUNCTION_MEMORY_MAP, (uint32_t)buffer, (uint32_t)&size) != OK)
    {
        return 0;
    }
    return size;
}

int get_drive_params(uint8_t dev, drive_params_t *buffer)
{
    buffer->size = sizeof(drive_params_t);
    buffer->flags = 0;
    return bios_function(FUNCTION_DRIVE_PARAMS, dev, (uint32_t)buffer);
}

uint16_t get_boot_device_sector_size()
{
    static drive_params_t dp;
    if (dp.size == 0) // not initialized
    {
        if (get_drive_params(boot_device, &dp) != OK)
        {
            dp.size = 0;
            return 0;
        }
    }
    return dp.sector_size;
}

uint16_t get_sector_size(uint8_t dev)
{
    static drive_params_t dp;

    if (dev == boot_device)
    {
        return get_boot_device_sector_size();
    }

    if (get_drive_params(dev, &dp) != OK)
    {
        return 0;
    }
    return dp.sector_size;
}
