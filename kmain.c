#include <multiboot.h>
#include <screen.h>
#include <rdtsc.h>

void delay(int x)
{
    int i = 10000;
    while (--i)
    {
        int j = 100 * x;
        while (--j);
    }
}

int kmain(int mb_magic, multiboot_info_t *mb_info)
{
    if (mb_magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        // not multiboot
        cls();
        kprint("WDOS [version 0.0]\nMust boot from a multiboot bootloader.\n");
        for (;;);
        return 0xdeadbeef;
    }

    cls();
    kprint("magic=");
    kprint_hex(mb_magic);
    kprint(", flags=");
    kprint_hex(mb_info->flags);
    kprint(",\nmem_lower=");
    kprint_int(mb_info->mem_lower);
    kprint(" KiB, mem_upper=");
    kprint_int(mb_info->mem_upper);
    kprint(" KiB, boot_device=");
    kprint_hex(mb_info->boot_device);
    kprint(",\ncmdline=\"");
    kprint((char *)mb_info->cmdline);
    kprint("\"\n");
    
    set_string(10, 5, 7, "hello, world!", SCREEN_COLOR_DEFAULT);
    delay(10);
    char c = 0x07;
    return 0;
    while (1)
    {
        delay(1);
        c += 0x11;
        kset_color(c);
        kprint("hello, world");
    }
    return 0;
}
