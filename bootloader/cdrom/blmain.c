#include <runtime/types.h>
#include <multiboot.h>

#include <stdlib/memory.h>
#include <stdlib/string.h>

#include "bios.h"
#include "iso9660.h"
#include "elf.h"

#define assert(x) do { if (!(x)) die("\r\nAssertion failed.\r\n"); } while (0)

const char *const cmdline_file = "/boot/cmdline.txt";
const char *const kernel_file = "/boot/kernel.elf";

char kernel_cmdline[256];
void *const sector_buffer = (void *)0x20000;
multiboot_info_t mb_info;

void *malloc(uint32_t size)
{
    static uint8_t *buffer = (void *)0x30000;
    uint8_t *old_buffer = buffer;
    buffer += size;
    return old_buffer;
}

void free(void *ptr)
{

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

void load_memory_map()
{
    static bios_memory_map_t mem[12];
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
            if (limit_low < 0xa0000) // in 640 KiB
            {
                mb_info.mem_lower = (limit_low + 1) >> 10;
            }
            else if (!mb_info.mem_upper && mem[i].base_low >= 0x100000) // first upper memory hole
            {
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

void die(const char *str)
{
    print(str);
    while (true)
    {
        asm("cli\n"
            "hlt");
    }
}

void *read_extent(directory_record_t *file, void *buffer)
{
    uint32_t file_lba = file->extent_location, file_length = file->data_length;
    //                                         ceiling((file_length + 2047) / 2048)
    read_sector(boot_device, file_lba, buffer, ((file_length + 2047) >> 11));
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
        read_sector(boot_device, lba, sector_buffer, 1);
        print(".");
        // check CD001
        if (memcmp(pvd->std_ident, "CD001", 5))
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

    static char buffer[256];
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

    directory_record_t *end = read_extent(&pvd->root_directory, sector_buffer);
    directory_record_t *begin = sector_buffer;
    print(".");

    while (iter)
    {
        iter = strsplit(iter, '/', buffer);
        to_ident(buffer);

        directory_record_t *rec = find_record(buffer, begin, end);
        if (!rec)
        {
            return NULL;
        }
        
        if (!iter) // is last
        {
            return rec;
        }

        // TODO: check file flags
        end = read_extent(rec, sector_buffer);
        begin = sector_buffer;
        print(".");
    }

    return begin;
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

typedef void (*kernel_entry_t)();

kernel_entry_t load_kernel()
{
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
    read_sector(boot_device, elf_lba, sector_buffer, 1);
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
    }
    // TODO: load each segment
    return kernel_entry;
}

void jmp_kernel(kernel_entry_t kernel_entry, uint32_t mb_magic, multiboot_info_t *mb_info)
{
    asm("jmp *%%ecx"
        : 
        : "c"(kernel_entry), "a"(mb_magic), "b"(mb_info));
}

void debug_pause()
{
    print("Press any key to continue...\r\n");
    getchar();
}

void blmain()
{
    memset(bss_begin, 0, bss_end - bss_begin); // init .bss

    print("Stage 1 booted successfully.\r\n");

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
