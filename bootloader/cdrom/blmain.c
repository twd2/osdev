#include <runtime/types.h>
#include <multiboot.h>

#include <stdlib/memory.h>
#include <stdlib/string.h>

#include "config.h"
#include "util.h"
#include "bios.h"
#include "iso9660.h"
#include "vesa.h"
#include "elf.h"

extern uint8_t _bss_begin, _bss_end;
void *bss_begin = &_bss_begin, *bss_end = &_bss_end;

const char *const cmdline_file = "/boot/cmdline.txt";
const char *const kernel_file = "/boot/kernel.elf";

void *const sector_buffer = (void *)0x20000; // length: 64 KiB

char kernel_cmdline[256];

uint32_t memory_map_count;
bios_memory_map_t memory_map[32];

multiboot_info_t mb_info;
vesa_control_info_t vesa_control_info;
vesa_mode_info_t vesa_mode_info;


void load_memory_map()
{
    print("Loading system memory map... ");
    memory_map_count = read_memory_map(memory_map);
    if (!memory_map_count)
    {
        die("\r\nLoad system memory map failed.\r\n");
    }
    print_int(memory_map_count);
    print(" entries.\r\n");
    print("+----------------------------------------------------+\r\n"
          "|                   SYSTEM MEMORY MAP                |\r\n"
          "+----------------------------------------------------+\r\n"
          "    BASE               LIMIT              TYPE USABLE?\r\n");
    mb_info.mem_lower = mb_info.mem_upper = 0;
#ifndef X86_64
    bool has_warning = false;
#endif
    for (uint32_t i = 0; i < memory_map_count; ++i)
    {
        uint64_t limit = memory_map[i].base - 1 + memory_map[i].length;
        uint32_t limit_low = memory_map[i].base_low - 1 + memory_map[i].length_low;
        print_byte((uint8_t)i);
        print(": ");
        print_hex_long(memory_map[i].base);
        print("~");
        print_hex_long(memory_map[i].base - 1 + memory_map[i].length);
        print("    ");
        print_int(memory_map[i].type);
        if (memory_map[i].type == MEMORY_TYPE_USABLE)
        {
#ifndef X86_64
            if (!memory_map[i].base_high && !(limit & 0xFFFFFFFF00000000ULL))
            {
#endif
            print("     YES");
            if (limit_low <= LOW_MEMORY_LIMIT) // in 640 KiB
            {
                mb_info.mem_lower = (limit_low + 1) >> 10;
            }
            else if (!mb_info.mem_upper && memory_map[i].base_low >= HIGH_MEMORY_BASE)
            {
                // first upper memory hole
                mb_info.mem_upper = memory_map[i].length_low >> 10;
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
        if ((i & 0xf) == 0xf)
        {
            debug_pause();
        }
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
    mb_info.flags = MULTIBOOT_INFO_FLAGS;
    // mem_lower and mem_upper are filled by load_memory_map()
    mb_info.boot_device = MKBOOT_DEVICE(boot_device, UNUSED_PARTITION,
                                        UNUSED_PARTITION, UNUSED_PARTITION);
    mb_info.cmdline = (uint32_t)kernel_cmdline;

    // copy memory_map
    assert(memory_map_count);
    mb_info.mmap_length = memory_map_count * sizeof(multiboot_memory_map_t);
    multiboot_memory_map_t *mb_mmap =
        (multiboot_memory_map_t *)palloc(mb_info.mmap_length);
    for (uint32_t i = 0; i < memory_map_count; ++i)
    {
        mb_mmap[i].size = sizeof(mb_mmap[i]) - sizeof(mb_mmap[i].size);
        mb_mmap[i].addr = memory_map[i].base;
        mb_mmap[i].len = memory_map[i].length;
        mb_mmap[i].type = memory_map[i].type;
    }
    mb_info.mmap_addr = (uint32_t)mb_mmap;

    // vbe_control_info, vbe_mode_info and vbe_mode will be filled by init_vesa()
}

void *read_all_extent(directory_record_t *file, void *buffer)
{
    const uint16_t sector_size = get_boot_device_sector_size();
    const uint32_t file_lba = file->extent_location, file_length = file->data_length;

    if (read_sector(boot_device, file_lba, buffer,
                    ((file_length - 1 + sector_size) / sector_size)) != OK)
    {
        die("\r\nRead error.\r\n");
    }
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
        // "/" is not a file.
        return NULL;
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
        if (!rec) // not found
        {
            return NULL;
        }

        if (!iter) // is last
        {
            if (rec->flags & ISO9660_FLAG_DIRECTORY)
            {
                // not a file
                return NULL;
            }
            return rec;
        }

        if (!(rec->flags & ISO9660_FLAG_DIRECTORY))
        {
            // not a directory
            return NULL;
        }
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
    print(".");

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
    print(".");
    lba += length / sector_size;
    dest = (void *)((uint8_t *)dest + (length - length % sector_size));
    length = length % sector_size;

    ret = read_sector(boot_device, lba, sector_buffer, 1); // last sector
    if (ret != OK)
    {
        return ret;
    }
    print(".");
    memcpy(dest, sector_buffer, length % sector_size);

    return OK;
}

kernel_entry_t load_kernel(multiboot_header_t *out_mb_header)
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

    kernel_entry_t kernel_entry = (kernel_entry_t)elf_header.e_entry;
    uint8_t code[] = { 0xeb, 0xfe /* jmp $ */, 0xcc /* int3 */ }; // default kernel code
    memcpy(kernel_entry, code, sizeof(code));

    print("kernel_entry=");
    print_hex(kernel_entry);
    print("\r\n");

    assert(elf_header.e_phnum);
    assert(elf_header.e_phentsize == sizeof(Elf32_Phdr));

    Elf32_Phdr *program_header =
        (Elf32_Phdr *)palloc(elf_header.e_phentsize * elf_header.e_phnum);
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

    memcpy(out_mb_header, mb_header, sizeof(multiboot_header_t));

    print("flags=");
    print_bin(mb_header->flags);
    print("\r\n");

    if (mb_header->flags & MULTIBOOT_UNSUPPORTED_FLAGS)
    {
        die("\r\nUnsupported flag(s) found.\r\n");
    }

    for (int i = 0; i < elf_header.e_phnum; ++i)
    {
        if (program_header[i].p_type == PT_LOAD)
        {
            print("Loading ");
            print_byte((uint8_t)i);
            print(".");
            int ret = load_segment(elf_lba + program_header[i].p_offset / sector_size,
                                   program_header[i].p_offset % sector_size,
                                   (void *)program_header[i].p_paddr,
                                   program_header[i].p_filesz);
            if (ret != OK)
            {
                die("\r\nRead error.\r\n");
            }
            if (program_header[i].p_filesz < program_header[i].p_memsz)
            {
                memset((void *)(program_header[i].p_paddr + program_header[i].p_filesz),
                       0, program_header[i].p_memsz - program_header[i].p_filesz);
                print("init .bss");
            }
            print("\r\n");
        }
    }

    return kernel_entry;
}

void print_vesa_mode_info(vesa_mode_info_t *info)
{
    print_int(info->x_res);
    print("x");
    print_int(info->y_res);
    print(", ");
    print_int(info->bpp);
    print("bpp, linear fb: ");
    print((info->attr & VESA_MODE_ATTR_LINEAR_FRAMEBUFFER) ? "YES" : "NO");
    print(", graphics: ");
    print((info->attr & VESA_MODE_ATTR_GRAPHICS) ? "YES" : "NO");
    print(", mem_model=");
    print_int(info->memory_model);
}

bool init_vesa(multiboot_header_t *mb_header)
{
    print("kernel requires video mode: ");
    print_int(mb_header->width);
    print("x");
    print_int(mb_header->height);
    print(", ");
    print_int(mb_header->depth);
    print("bpp\r\n");

    memset(&vesa_control_info, 0, sizeof(vesa_control_info));
#ifdef USE_VBE2
    memcpy(vesa_control_info.magic, "VBE2", 4);
#endif

    if (get_vesa_control_info(&vesa_control_info) != OK)
    {
        print("Warning: Get VESA control info failed.\r\n");
        return false;
    }

    static char buffer[256] = {0};

    memcpy(buffer, vesa_control_info.magic, 4);
    if (memcmp(buffer, VESA_MAGIC, VESA_MAGIC_LENGTH))
    {
        print("Warning: Get VESA control info failed.\r\n");
        return false;
    }

    print("+--------------+\r\n"
          "|  VESA  INFO  |\r\n"
          "+--------------+\r\n");
    print("magic=");
    print(buffer);
    print(", version=");
    print_int(vesa_control_info.version >> 8);
    print(".");
    print_int(vesa_control_info.version & 0xff);
    print("\r\n");
    strcpy(buffer, logic_to_linear(vesa_control_info.oem_string_seg,
                                   vesa_control_info.oem_string_offset));
    print("OEM string=\r\n  ");
    print(buffer);
    print("\r\ntotal_memory=");
    print_int((uint32_t)vesa_control_info.total_memory * 64);
    print(" KiB, supported modes:\r\n");

    debug_pause();

    uint16_t *modes =
        (uint16_t *)(logic_to_linear(vesa_control_info.video_mode_seg,
                                     vesa_control_info.video_mode_offset));

    uint16_t vesa_mode_selected = VESA_MODE_INVALID;
    uint32_t vesa_mode_diff = (uint32_t)-1;

    for (size_t i = 0; modes[i] != VESA_MODE_TERMINATOR; ++i)
    {
#ifndef NDEBUG
        print("  ");
        print_byte(modes[i] >> 8);
        print_byte(modes[i] & 0xff);
        print(": ");
#endif
        memset(&vesa_mode_info, 0, sizeof(vesa_mode_info));
        if (get_vesa_mode_info(modes[i], &vesa_mode_info) == OK)
        {
#ifndef NDEBUG
            print_vesa_mode_info(&vesa_mode_info);
#endif
            if ((vesa_mode_info.attr & VESA_MODE_ATTR_LINEAR_FRAMEBUFFER) &&
                (vesa_mode_info.attr & VESA_MODE_ATTR_GRAPHICS) &&
                vesa_mode_info.memory_model == VESA_MODE_MEMORY_MODEL_DIRECTCOLOR)
            {
                uint32_t diff =
                    mode_diff(mb_header->width, mb_header->height, mb_header->depth,
                              vesa_mode_info.x_res, vesa_mode_info.y_res, vesa_mode_info.bpp);
                if (!diff)
                {
                    vesa_mode_selected = modes[i];
                    vesa_mode_diff = diff;
                    // break;
                }
                else if (diff < vesa_mode_diff)
                {
                    vesa_mode_selected = modes[i];
                    vesa_mode_diff = diff;
                }
            }
        }
        else
        {
            print("Warning: Set VESA mode info failed.");
        }
        print("\r\n");
        if ((i & 0xf) == 0xf)
        {
            debug_pause();
        }
    }
    print("END\r\n");
    debug_pause();

    if (vesa_mode_selected == 0xffff || vesa_mode_diff == (uint32_t)-1)
    {
        print("Warning: No satisfied mode.\r\n");
        return false;
    }
    else
    {
        memset(&vesa_mode_info, 0, sizeof(vesa_mode_info));
        assert(get_vesa_mode_info(vesa_mode_selected, &vesa_mode_info) == OK);
        print("Selected mode:\r\n  ");
        print_byte(vesa_mode_selected >> 8);
        print_byte(vesa_mode_selected & 0xff);
        print(": ");
        print_vesa_mode_info(&vesa_mode_info);
        print("\r\n");
        print("base=");
        print_hex(vesa_mode_info.base);
        print("\r\n");
        print("Going to graphics mode... ");
        debug_pause();
    }

    if (set_vesa_mode(vesa_mode_selected | VESA_MODE_LINEAR_FRAMEBUFFER) != OK)
    {
        print("Warning: Set VESA mode failed.\r\n");
        return false;
    }

    mb_info.flags |= MULTIBOOT_INFO_VIDEO_INFO;
    mb_info.vbe_control_info = (uint32_t)&vesa_control_info;
    mb_info.vbe_mode_info = (uint32_t)&vesa_mode_info;
    mb_info.vbe_mode = vesa_mode_selected | VESA_MODE_LINEAR_FRAMEBUFFER;
    return true;
}

void jmp_kernel(kernel_entry_t kernel_entry, uint32_t mb_magic, multiboot_info_t *mb_info)
{
    asm volatile ("jmp *%%ecx"
                  :
                  : "c"(kernel_entry), "a"(mb_magic), "b"(mb_info));
}

void blmain()
{
    print("Stage 1 booted successfully.\r\n");

    if ((uintptr_t)bss_end - STAGE2_LOAD_ADDRESS > SEGMENT_LIMIT)
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

    multiboot_header_t mb_header;
    kernel_entry_t kernel_entry = load_kernel(&mb_header);
    assert(kernel_entry);

    debug_pause();

    if ((mb_header.flags & MULTIBOOT_VIDEO_MODE) &&
        mb_header.mode_type == MULTIBOOT_MODE_TYPE_GRAPHICS)
    {
        print("Calling kernel(at ");
        print_hex(kernel_entry);
        print(") with cmdline=\"");
        print(kernel_cmdline);
        print("\"...\r\n");
        if (!init_vesa(&mb_header))
        {
            debug_pause();
        }
    }
    else
    {
        print("Calling kernel(at ");
        print_hex(kernel_entry);
        print(") with cmdline=\"");
        print(kernel_cmdline);
        print("\"...\r\n");
        debug_pause();
    }

    jmp_kernel(kernel_entry, MULTIBOOT_BOOTLOADER_MAGIC, &mb_info);

    die("Unknown error.");
}
