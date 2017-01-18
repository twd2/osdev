#include <runtime/types.h>
#include <multiboot.h>

#include <stdlib/memory.h>
#include <stdlib/string.h>

#include "util.h"
#include "bios.h"
#include "iso9660.h"
#include "elf.h"

#define LOW_MEMORY_BASE 0
#define LOW_MEMORY_LIMIT (0xa0000 - 1)
#define HIGH_MEMORY_BASE 0x100000

extern uint8_t _bss_begin, _bss_end;
void *bss_begin = &_bss_begin, *bss_end = &_bss_end;

const char *const cmdline_file = "/boot/cmdline.txt";
const char *const kernel_file = "/boot/kernel.elf";

char kernel_cmdline[256];
void *const sector_buffer = (void *)0x20000; // length: 64 KiB
multiboot_info_t mb_info;

#define NDEBUG
#ifndef NDEBUG
void debug_pause()
{
    print("Press any key to continue...\r\n");
    getchar();
}
#else
#define debug_pause() do { } while (0)
#endif

void load_memory_map()
{
    static bios_memory_map_t mem[16];
    print("Loading system memory map... ");
    int count = read_memory_map(mem);
    if (!count)
    {
        die("\r\nLoad system memory map failed.\r\n");
    }
    print_int(count);
    print(" entries.\r\n");
    print("+----------------------------------------------------+\r\n"
          "|                   SYSTEM MEMORY MAP                |\r\n"
          "+----------------------------------------------------+\r\n"
          "    BASE               LIMIT              TYPE USABLE?\r\n");
    mb_info.mem_lower = mb_info.mem_upper = 0;
#ifndef X86_64
    bool has_warning = false;
#endif
    for (int i = 0; i < count; ++i)
    {
        uint64_t limit = mem[i].base - 1 + mem[i].length;
        uint32_t limit_low = mem[i].base_low - 1 + mem[i].length_low;
        print_byte((uint8_t)i);
        print(": ");
        print_hex_long(mem[i].base);
        print("~");
        print_hex_long(mem[i].base - 1 + mem[i].length);
        print("    ");
        print_int(mem[i].type);
        if (mem[i].type == MEMORY_TYPE_USABLE)
        {
#ifndef X86_64
            if (!mem[i].base_high && !(limit & 0xFFFFFFFF00000000ULL))
            {
#endif
            print("     YES");
            if (limit_low <= LOW_MEMORY_LIMIT) // in 640 KiB
            {
                mb_info.mem_lower = (limit_low + 1) >> 10;
            }
            else if (!mb_info.mem_upper && mem[i].base_low >= HIGH_MEMORY_BASE)
            {
                // first upper memory hole
                mb_info.mem_upper = mem[i].length_low >> 10;
            }
#ifndef X86_64
            }
            else
            {
                has_warning = true;
                print("     NO*");
            }
#endif
        }
        else
        {
            print("      NO");
        }
        print("\r\n");
    }
#ifndef X86_64
    if (has_warning)
    {
        print("* Warning: High memory is not usable for 32-bit system.\r\n");
    }
#endif
    print("mem_lower=");
    print_int(mb_info.mem_lower);
    print(" KiB, mem_upper=");
    print_int(mb_info.mem_upper);
    print(" KiB\r\n");
}

void fill_multiboot_info()
{
    mb_info.flags = 0b111; // has mem_lower, mem_upper, boot_device and cmdline
    // mem_lower and mem_upper are filled by load_memory_map()
    mb_info.boot_device = ((uint32_t)boot_device << 24) | 0xffffff; // ???
    mb_info.cmdline = (uint32_t)kernel_cmdline;
}

void *read_all_extent(directory_record_t *file, void *buffer)
{
    const uint16_t sector_size = get_boot_device_sector_size();
    const uint32_t file_lba = file->extent_location, file_length = file->data_length;
    //                                         ceiling(file_length / 2048)
    assert(read_sector(boot_device, file_lba, buffer,
                       ((file_length - 1 + sector_size) / sector_size)) == OK);
    return (void *)((uint8_t *)buffer + file_length);
}

directory_record_t *find_file(const char *path)
{
    if (strlen(path) > 255)
    {
        die("\r\nPath is too long.\r\n");
    }

    // load root directory
    vd_primary_t *pvd = (vd_primary_t *)sector_buffer;
    for (uint32_t lba = FIRST_LBA; ; ++lba)
    {
        read_sector(boot_device, lba, sector_buffer, 1); // might fail
        print(".");
        // check magic "CD001"
        if (memcmp(pvd->std_ident, ISO9660_MAGIC, ISO9660_MAGIC_LENGTH))
        {
            die("\r\nInvalid CD-ROM.\r\n");
        }
        if (pvd->type == VD_TYPE_PRIMARY)
        {
            // found
            break;
        }
        else if (pvd->type == VD_TYPE_TERMINATOR)
        {
            die("\r\nNo primary volume descriptor.\r\n");
        }
    }

    const char *iter = path;
    if (*iter == '/')
    {
        ++iter;
    }
    else
    {
        die("\r\nRelative path is not supported.\r\n");
    }

    if (!*iter) // path == "/"
    {
        return &pvd->root_directory;
    }

    directory_record_t *end = read_all_extent(&pvd->root_directory, sector_buffer);
    directory_record_t *begin = sector_buffer;
    print(".");

    static char ident[256];
    while (iter)
    {
        iter = strsplit(iter, '/', ident);
        to_ident(ident);

        directory_record_t *rec = find_record(ident, begin, end);
        if (!rec)
        {
            return NULL;
        }
        
        if (!iter) // is last
        {
            return rec;
        }

        // TODO: check file/directory flags
        end = read_all_extent(rec, sector_buffer); // next level
        begin = sector_buffer;
        print(".");
    }

    return NULL;
}

void load_cmdline()
{
    print("Loading cmdline file ");
    print(cmdline_file);
    print(".");
    directory_record_t *rec = find_file(cmdline_file);
    if (!rec)
    {
        die("\r\nFile not found.\r\n");
    }
    if (rec->data_length >= sizeof(kernel_cmdline))
    {
        die("\r\nCmdline is too long.\r\n");
    }
    uint32_t length = rec->data_length;
    read_all_extent(rec, sector_buffer);
    memcpy(kernel_cmdline, sector_buffer, length);
    for (char *p = kernel_cmdline; *p; ++p)
    {
        if (*p == '\r' || *p == '\n')
        {
            *p = 0;
            break;
        }
    }
    print(" ");
    print_int(strlen(kernel_cmdline));
    print(" bytes read.\r\n");
}

int load_segment(uint32_t lba, uint32_t offset, void *dest, uint32_t length)
{
    const uint16_t sector_size = get_boot_device_sector_size();
    assert(offset < sector_size);
    int ret;

// |......***.|..........|..........|..........|
// |......****|***.......|..........|..........|
///   first       mid        last
// |......****|**********|***.......|..........|

    ret = read_sector(boot_device, lba, sector_buffer, 1); // first sector
    if (ret != OK)
    {
        return ret;
    }

    if (length <= sector_size - offset)
    {
        memcpy(dest, sector_buffer, length);
        return OK;
        // done
    }
    else
    {
        memcpy(dest, sector_buffer, sector_size - offset);
        ++lba;
        dest = (void *)((uint8_t *)dest + sector_size - offset);
        length -= sector_size - offset;
    }

    // mid
    ret = read_sector_high_memory(boot_device, lba, dest, length / sector_size, sector_buffer);
    if (ret != OK)
    {
        return ret;
    }
    lba += length / sector_size;
    dest = (void *)((uint8_t *)dest + (length - length % sector_size));
    length = length % sector_size;

    ret = read_sector(boot_device, lba, sector_buffer, 1); // last sector
    memcpy(dest, sector_buffer, length % sector_size);

    return OK;
}

typedef void (*kernel_entry_t)();

kernel_entry_t load_kernel()
{
    const uint16_t sector_size = get_boot_device_sector_size();
    print("Loading kernel ");
    print(kernel_file);
    print(".");
    directory_record_t *rec = find_file(kernel_file);
    if (!rec)
    {
        die("\r\nFile not found.\r\n");
    }

    uint32_t elf_lba = rec->extent_location, elf_length = rec->data_length;

    print(" ");
    print_int(elf_length);
    print(" bytes.\r\n");

    Elf32_Ehdr elf_header;
    assert(read_sector(boot_device, elf_lba, sector_buffer, 1) == OK);
    print(".");
    memcpy(&elf_header, sector_buffer, sizeof(Elf32_Ehdr));

    if (memcmp(elf_header.e_ident, ELFMAG, SELFMAG))
    {
        die("\r\nNot an ELF file.\r\n");
    }

    if (elf_header.e_type != ET_EXEC)
    {
        die("\r\nNot an executable ELF file.\r\n");
    }

    kernel_entry_t kernel_entry = elf_header.e_entry;
    uint8_t code[] = { 0xeb, 0xfe /* jmp $ */, 0xcc /* int3 */ }; // default kernel code
    memcpy(kernel_entry, code, sizeof(code));

    print("kernel_entry=");
    print_hex(kernel_entry);
    print("\r\n");

    assert(elf_header.e_phnum);
    assert(elf_header.e_phentsize == sizeof(Elf32_Phdr));

    Elf32_Phdr *program_header =
        (Elf32_Phdr *)malloc(elf_header.e_phentsize * elf_header.e_phnum);
    memcpy(program_header, (uint8_t *)sector_buffer + elf_header.e_phoff,
           elf_header.e_phentsize * elf_header.e_phnum);

    print("+-------------------------------------------------------------+\r\n"
          "|                        PROGRAM HEADERS                      |\r\n"
          "+-------------------------------------------------------------+\r\n"
          "    TYPE OFFSET     VIRT ADDR  PHY ADDR   FILE SIZE  MEM SIZE  \r\n");
    for (int i = 0; i < elf_header.e_phnum; ++i)
    {
        print_byte((uint8_t)i);
        print(": ");
        if (program_header[i].p_type == PT_LOAD)
        {
            print("LOAD");
        }
        else
        {
            print("????");
        }
        print(" ");
        print_hex(program_header[i].p_offset);
        print(" ");
        print_hex(program_header[i].p_vaddr);
        print(" ");
        print_hex(program_header[i].p_paddr);
        print(" ");
        print_hex(program_header[i].p_filesz);
        print(" ");
        print_hex(program_header[i].p_memsz);
        print("\r\n");
        
        if (program_header[i].p_type == PT_LOAD &&
            program_header[i].p_paddr - 1 + program_header[i].p_memsz >=
            (HIGH_MEMORY_BASE + (mb_info.mem_upper << 10)))
        {
            die("\r\nNo memory.\r\n");
        }
    }

    // compatible with multiboot
    // check multiboot header
    uint32_t ph0_offset = program_header[0].p_offset;
    assert(read_sector(boot_device, elf_lba + ph0_offset / sector_size, sector_buffer, 1) == OK);

    multiboot_header_t *mb_header =
        (multiboot_header_t *)((uint8_t *)sector_buffer + ph0_offset % sector_size);

    if (mb_header->magic != MULTIBOOT_HEADER_MAGIC)
    {
        die("\r\nELF file is not multiboot compatible.\r\n");
    }

    if (-(mb_header->magic + mb_header->flags) != mb_header->checksum)
    {
        die("\r\nChecksum is incorrect.\r\n");
    }

    debug_pause();

    for (int i = 0; i < elf_header.e_phnum; ++i)
    {
        if (program_header[i].p_type == PT_LOAD)
        {
            print("Loading ");
            print_byte((uint8_t)i);
            print(".");
            load_segment(elf_lba + program_header[i].p_offset / sector_size,
                         program_header[i].p_offset % sector_size,
                         (void *)program_header[i].p_paddr,
                         program_header[i].p_filesz);
            if (program_header[i].p_filesz < program_header[i].p_memsz)
            {
                memset(program_header[i].p_paddr + program_header[i].p_filesz,
                       0, program_header[i].p_memsz - program_header[i].p_filesz);
                print("init .bss");
            }
            print("\r\n");
        }
    }

    return kernel_entry;
}

void jmp_kernel(kernel_entry_t kernel_entry, uint32_t mb_magic, multiboot_info_t *mb_info)
{
    asm("jmp *%%ecx"
        : 
        : "c"(kernel_entry), "a"(mb_magic), "b"(mb_info));
}

void blmain()
{
    print("Stage 1 booted successfully.\r\n");

    if ((uint32_t)bss_end > 0x1ffff)
    {
        die("Section .bss is too long.\r\n");
    }
    memset(bss_begin, 0, bss_end - bss_begin); // init .bss

    print("boot_device=");
    print_byte(boot_device);
    print(", sector size is ");
    print_int(get_boot_device_sector_size());
    print(" bytes.\r\n");

    load_memory_map();

    debug_pause();

    load_cmdline();
    fill_multiboot_info();

    debug_pause();

    kernel_entry_t kernel_entry = load_kernel();

    debug_pause();

    print("Calling kernel(at ");
    print_hex(kernel_entry);
    print(") with cmdline=\"");
    print(kernel_cmdline);
    print("\"...\r\n");
    assert(kernel_entry);
    jmp_kernel(kernel_entry, MULTIBOOT_BOOTLOADER_MAGIC, &mb_info);

    die("Unknown error.");
}
