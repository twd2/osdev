#include "bios.h"

#define FUNCTION_PRINT 1
#define FUNCTION_GETCHAR 2
#define FUNCTION_READ_SECTOR 3
#define FUNCTION_MEMORY_MAP 4
#define FUNCTION_DRIVE_PARAMS 5

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

int print_hex(uint32_t x)
{
#define HEX_COUNT (sizeof(x) * 2)
    static char buffer[HEX_COUNT + 3] = { '0', 'x', 0 };
    utoh(x, &buffer[2]);
#undef HEX_COUNT
    return print(buffer);
}

int print_bin(uint32_t x)
{
#define BIT_COUNT (sizeof(x) * 8)
    static char buffer[BIT_COUNT + 3] = { '0', 'b', 0 };
    utob(x, &buffer[2]);
#undef BIT_COUNT
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
#define HEX_COUNT (sizeof(x) * 2)
    static char buffer[HEX_COUNT + 1] = { 0 };
    btoh(x, buffer);
#undef HEX_COUNT
    return print(buffer);
}

int print_hex_long(uint64_t x)
{
#define HEX_COUNT (sizeof(x) * 2)
    static char buffer[HEX_COUNT + 3] = { '0', 'x', 0 };
    ultoh(x, &buffer[2]);
#undef HEX_COUNT
    return print(buffer);
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
    const uint16_t sector_size = get_boot_device_sector_size();
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
