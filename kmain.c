#include <boot.h>
#include <multiboot.h>
#include <tty.h>
#include <pm.h>
#include <asm.h>
#include <mm/mm.h>
#include <syscall.h>
#include <interrupt.h>
#include <driver/pic8259a.h>
#include <driver/pit8253.h>
#include <driver/keyboard.h>
#include <process.h>
#include <syscall_impl.h>
#include <idle.h>
#include <driver/vesa.h>

static uint8_t process_stack[0x8000];

void delay(int x)
{
    int i = 10000;
    while (--i)
    {
        int j = 10 * x;
        while (--j);
    }
}

void process1()
{
    kset_color(TTY_MKCOLOR(TTY_COLOR_LIGHTCYAN, TTY_COLOR_BLACK));
    kprint("My PID=");
    kprint_int(get_pid());
    kprint("\n");
    kprint("calling sys_test()...");
    kprint_hex(sys_test());
    kprint("\n");
    kprint("calling sys_add(123, 654)...");
    kprint_int(sys_add(123, 654));
    kprint("\n");
    uint32_t i = 0;
    while (true)
    {
        ktty_enter();
        kprint("My PID=");
        kprint_int(get_pid());
        kprint(" A");
        kprint_int(++i);
        kprint("\n");
        ktty_leave();
        //kprint("calling sys_delay(1)...");
        //sys_delay(1);
        //delay(1);
    }
    // TODO: sys_exit
}

void process2()
{
    kset_color(TTY_MKCOLOR(TTY_COLOR_WHITE, TTY_COLOR_BLACK));
    kprint("My PID=");
    kprint_int(get_pid());
    kprint("\n");
    kprint("calling sys_test()...");
    kprint_hex(sys_test());
    kprint("\n");
    kprint("calling sys_add(135, 531)...");
    kprint_int(sys_add(135, 531));
    kprint("\n");
    uint32_t i = 0;
    while (true)
    {
        ktty_enter();
        kprint("My PID=");
        kprint_int(get_pid());
        kprint(" B");
        kprint_int(++i);
        kprint("\n");
        ktty_leave();
        //kprint("calling sys_delay(2)...");
        //sys_delay(2);
        //delay(2);
    }
    // TODO: sys_exit
}

void process3()
{
    kprint("My PID=");
    kprint_int(get_pid());
    kprint("\nThis is tty ");
    kprint_int(get_ttyid() + 1);
    kprint(".\n");
    while (true)
    {
        sys_yield();
    }
}

void print_multiboot_info(int mb_magic, multiboot_info_t *mb_info)
{
    kprint("[KDEBUG] ====multiboot header====\n");
    kprint("-> magic=");
    kprint_hex(mb_magic);
    kprint(", mb_info at ");
    kprint_hex(mb_info);
    kprint(",\n-> flags=");
    kprint_bin(mb_info->flags);
    kprint(",\n-> mem_lower=");
    kprint_int(mb_info->mem_lower);
    kprint(" KiB, mem_upper=");
    kprint_int(mb_info->mem_upper);
    kprint(" KiB, boot_device=");
    kprint_hex(mb_info->boot_device);
    kprint(", \n-> mmap_addr=");
    kprint_hex(mb_info->mmap_addr);
    kprint(", mmap_length=");
    kprint_int(mb_info->mmap_length);
    kprint(" bytes,\n-> cmdline at ");
    kprint_hex(mb_info->cmdline);
    kprint(", cmdline=\"");
    kprint((char *)__VA((void *)mb_info->cmdline));
    kprint("\"\n");
}

void print_mem_info()
{
    kprint("[KDEBUG] ====system memory info====\n");
    kprint("-> free_mem_start=");
    kprint_hex(free_mem_start);
    kprint(", free_mem_end=");
    kprint_hex(free_mem_end);
    kprint(", bios_mem_map_count=");
    kprint_int(bios_mem_map_count);
    kprint("\n");
    memory_map_long_t *mmap = bios_mem_map;
    for (uint32_t i = 0; i < bios_mem_map_count; ++i)
    {
        kprint("[KDEBUG] ");
        kprint_int(i);
        kprint(": base=");
        kprint_hex_long(mmap[i].base);
        kprint(", limit=");
        kprint_hex_long(mmap[i].base - 1 + mmap[i].length);
        kprint("\n");
    }
}

int kmain(int mb_magic, multiboot_info_t *mb_info)
{
    mb_info = __VA(mb_info);

    if (mb_magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        // not multiboot
        /*init_tty();
        kclear();
        kprint("WDOS [version 0.0]\nMust boot from a multiboot bootloader.\n");*/
        return 0xdeadbeef;
    }

    init_pm();

    init_vesa(mb_info);
    if (vesa_available)
    {
        init_tty(vesa_get_tty_driver());
    }
    else
    {
        init_vga();
        init_tty(vga_get_tty_driver());
    }

    kclear();
    kprint_ok_fail("[KDEBUG] check VESA availability", vesa_available);

    kprint_ok_fail("[KDEBUG] system booted successfully.", true);
    print_multiboot_info(mb_magic, mb_info);

    bool flags_satisified = (mb_info->flags & MULTIBOOT_NEEDED_FLAGS) == MULTIBOOT_NEEDED_FLAGS;
    kprint_ok_fail("[KDEBUG] check multiboot header flags", flags_satisified);
    if (!flags_satisified)
    {
        return 0xdeadbeef;
    }

    init_vesa(mb_info);
    kprint("abcdefghijklmnopqrstuvwxyz\n");
    kprint("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n");

    //for (;;) asm volatile ("cli\nhlt");
    init_mm(mb_info);
    print_mem_info();

    kprint_ok_fail("[KDEBUG] init PIC 8259a", true);
    init_pic8259a();
    
    kprint_ok_fail("[KDEBUG] init PIT 8253", true);
    init_pit8253();

    kprint_ok_fail("[KDEBUG] init keyboard", true);
    init_keyboard();

    init_syscall_impl();

    kprint_ok_fail("[KDEBUG] init process scheduler", true);
    init_process();

    init_idle();
    init_dwm();

    tty_switch(default_tty + 1);
    uint32_t kpid = process_create("kernel", SELECTOR_KERNEL_CODE, &process1,
                                   SELECTOR_KERNEL_DATA, &process_stack[0x1000],
                                   &process_stack[0x2000]);
    kprint_ok_fail("[KDEBUG] create kernel process", kpid != (uint32_t)-1);
    tty_switch(default_tty + 2);
    uint32_t ipid = process_create("init", SELECTOR_USER_CODE, &process2,
                                   SELECTOR_USER_DATA, &process_stack[0x3000],
                                   &process_stack[0x4000]);
    kprint_ok_fail("[KDEBUG] create init process", ipid != (uint32_t)-1);
    tty_switch(default_tty + 3);
    uint32_t spid = process_create("shell", SELECTOR_USER_CODE, &process3,
                                   SELECTOR_USER_DATA, &process_stack[0x5000],
                                   &process_stack[0x6000]);
    tty_switch(default_tty + 4);
    uint32_t s2pid = process_create("shell", SELECTOR_USER_CODE, &process3,
                                    SELECTOR_USER_DATA, &process_stack[0x7000],
                                    &process_stack[0x8000]);
    tty_switch(default_tty);

    kprint(TTY_SET_COLOR "\013hello, world\b\b\b\b\btwd2.\n" TTY_SET_COLOR TTY_DEFAULT_COLOR);

    kprint(        " \001\0011\001\0022\001\0033\001\0044\001\0055\001\0066\001\0077"
           "\001\0108\001\0119\001\012A\001\013B\001\014C\001\015D\001\016E\001\017F\r\n"
           TTY_SET_COLOR TTY_DEFAULT_COLOR);

    enable_interrupt();
    kprint_ok_fail("[KDEBUG] enable interrupt", true);

    // will not reach here
    for (;;)
    {
        asm("hlt");
    }
    return 0;
}
