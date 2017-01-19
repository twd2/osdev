#ifndef _WDOS_BOOTLOADER_CDROM_STAGE2_ISO9660_H_
#define _WDOS_BOOTLOADER_CDROM_STAGE2_ISO9660_H_

#include <runtime/types.h>

#define FIRST_LBA 16

#define ISO9660_MAGIC "CD001"
#define ISO9660_MAGIC_LENGTH 5

#define VD_TYPE_PRIMARY 0x01
#define VD_TYPE_TERMINATOR 0xff

#define ISO9660_FLAG_DIRECTORY 0b10

typedef struct directory_record
{
    uint8_t length;
    uint8_t ear_length;
    uint32_t extent_location;
    uint32_t extent_location_be; // big-endian, unused
    uint32_t data_length;
    uint32_t data_length_be; // big-endian, unused
    uint8_t datetime[7];
    uint8_t flags;
    uint8_t unit_size;
    uint8_t gap_size;
    uint16_t vol_seq_number;
    uint16_t vol_seq_number_be; // big-endian, unused
    uint8_t ident_length;
    char ident[0];
} __attribute__((packed)) directory_record_t;

typedef struct vd_primary
{
    uint8_t type;
    char std_ident[5];
    uint8_t version;
    uint8_t reserved1;
    char sys_ident[32];
    char vol_ident[32];
    uint8_t reserved2[8];
    uint8_t unused1[76];
    directory_record_t root_directory;
    uint8_t unused2[1858];    
} __attribute__((packed)) vd_primary_t;

directory_record_t *next_record(directory_record_t *ptr);
directory_record_t *find_record(const char *pattern, directory_record_t *begin, directory_record_t *end);
bool ident_cmp(const char *pattern, uint32_t ident_length, const char *ident);
void to_ident(char *pattern);

#endif // _WDOS_BOOTLOADER_CDROM_STAGE2_ISO9660_H_
