#include <boot.h>
#include <multiboot.h>
#include <tty.h>
#include <pm.h>
#include <asm.h>
#include <syscall.h>
#include <interrupt.h>
#include <driver/8259a.h>
#include <driver/clock.h>
#include <driver/keyboard.h>
#include <process.h>
#include <syscall_impl.h>

extern uint8_t end_of_kernel;
void *free_mem_start = &end_of_kernel;
void *free_mem_end = NULL; // [free_mem_start, free_mem_end)
uint32_t memory_map_count;
memory_map_t memory_map[16];
static uint8_t process_stack[0x4000];

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
    uint8_t i = 0;
    while (true)
    {
        ktty_enter();
        kprint("My PID=");
        kprint_int(get_pid());
        kprint(" A");
        kprint_int(++i);
        kprint(" ");
        ktty_leave();
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
    uint8_t i = 0;
    while (true)
    {
        ktty_enter();
        kprint("My PID=");
        kprint_int(get_pid());
        kprint(" B");
        kprint_int(++i);
        kprint(" ");
        ktty_leave();
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
    kprint((char *)mb_info->cmdline);
    kprint("\"\n");
}

void copy_mem_map(multiboot_info_t *mb_info)
{
    memory_map_long_t *mmap = (memory_map_long_t *)memory_map;
    memory_map_t *mb_mmap = (memory_map_t *)mb_info->mmap_addr;
    memory_map_t *mb_mmap_end = (memory_map_t *)(mb_info->mmap_addr + mb_info->mmap_length);

    memory_map_count = 0;
    while (mb_mmap != mb_mmap_end)
    {
        if (mb_mmap->type == MEMORY_TYPE_USABLE)
        {
            mmap[memory_map_count].base_low = mb_mmap->base_addr_low;
            mmap[memory_map_count].base_high = mb_mmap->base_addr_high;
            mmap[memory_map_count].length_low = mb_mmap->length_low;
            mmap[memory_map_count].length_high = mb_mmap->length_high;
            ++memory_map_count;
        }

        //next
        mb_mmap = (memory_map_t *)((uint8_t *)mb_mmap + mb_mmap->size + sizeof(mb_mmap->size));
    }
}

void print_mem_info()
{
    kprint("[KDEBUG] ====system memory info====\n");
    kprint("-> free_mem_start=");
    kprint_hex(free_mem_start);
    kprint(", free_mem_end=");
    kprint_hex(free_mem_end);
    kprint(", memory_map_count=");
    kprint_int(memory_map_count);
    kprint("\n");
    memory_map_long_t *mmapl = (memory_map_long_t *)memory_map;
    for (uint32_t i = 0; i < memory_map_count; ++i)
    {
        kprint("[KDEBUG] ");
        kprint_int(i);
        kprint(": base=");
        kprint_hex_long(mmapl[i].base);
        kprint(", limit=");
        kprint_hex_long(mmapl[i].base - 1 + mmapl[i].length);
        kprint("\n");
    }
}

int kmain(int mb_magic, multiboot_info_t *mb_info)
{
    if (mb_magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        // not multiboot
        init_tty();
        kclear();
        kprint("WDOS [version 0.0]\nMust boot from a multiboot bootloader.\n");
        return 0xdeadbeef;
    }

    free_mem_end = (void *)((mb_info->mem_upper << 10) + 0x100000);

    init_tty();
    kclear();

    kprint_ok_fail("[KDEBUG] system booted successfully.", true);
    print_multiboot_info(mb_magic, mb_info);

    bool flags_satisified = (mb_info->flags & MULTIBOOT_NEEDED_FLAGS) == MULTIBOOT_NEEDED_FLAGS;
    kprint_ok_fail("[KDEBUG] check multiboot header flags", flags_satisified);
    if (!flags_satisified)
    {
        return 0xdeadbeef;
    }

    copy_mem_map(mb_info);
    print_mem_info();

    kprint_ok_fail("[KDEBUG] init PIC 8259a", true);
    init_8259a();
    
    kprint_ok_fail("[KDEBUG] init PIT 8253", true);
    init_clock();

    kprint_ok_fail("[KDEBUG] init keyboard", true);
    init_keyboard();

    init_syscall_impl();

    kprint_ok_fail("[KDEBUG] init process scheduler", true);
    init_process();

    init_idle();

    tty_switch(default_tty + 1);
    uint32_t kpid = process_create("kernel", SELECTOR_KERNEL_CODE, &process1,
                                   SELECTOR_KERNEL_DATA, &process_stack[0x1000]);
    kprint_ok_fail("[KDEBUG] create kernel process", kpid != (uint32_t)-1);
    tty_switch(default_tty + 2);
    uint32_t ipid = process_create("init", SELECTOR_USER_CODE, &process2,
                                   SELECTOR_USER_DATA, &process_stack[0x2000]);
    kprint_ok_fail("[KDEBUG] create init process", ipid != (uint32_t)-1);
    tty_switch(default_tty + 3);
    uint32_t spid = process_create("shell", SELECTOR_USER_CODE, &process3,
                                   SELECTOR_USER_DATA, &process_stack[0x3000]);
    tty_switch(default_tty + 4);
    uint32_t s2pid = process_create("shell", SELECTOR_USER_CODE, &process3,
                                    SELECTOR_USER_DATA, &process_stack[0x4000]);
    tty_switch(default_tty);

    kprint(TTY_SET_COLOR "\013hello, world\n" TTY_SET_COLOR TTY_DEFAULT_COLOR);

    enable_interrupt();
    kprint_ok_fail("[KDEBUG] enable interrupt", true);

    // will not reach here
    for (;;)
    {
        asm("hlt");
    }
    return 0;
}
