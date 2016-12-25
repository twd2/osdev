#include <multiboot.h>
#include <kstdio.h>
#include <pm.h>
#include <asm.h>
#include <syscall.h>
#include <interrupt.h>
#include <driver/8259a.h>
#include <process.h>

static uint8_t process_stack[0x2000];

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
        enter_kstdio();
        kprint("A");
        kprint_int(++i);
        kprint(" ");
        leave_kstdio();
        delay(10);
    }
}

void process2()
{
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
        enter_kstdio();
        kprint("B");
        kprint_int(++i);
        kprint(" ");
        leave_kstdio();
        delay(20);
    }
}

int kmain(int mb_magic, multiboot_info_t *mb_info)
{
    if (mb_magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        // not multiboot
        cls();
        kprint("WDOS [version 0.0]\nMust boot from a multiboot bootloader.\n");
        asm("cli");
        for (;;)
        {
            asm("hlt");
        }
        return 0xdeadbeef;
    }

    cls();

    kprint_ok_fail("[KDEBUG] system booted successfully.", true);
    kprint("[KDEBUG] ====multiboot header====\n");
    kprint("-> magic=");
    kprint_hex(mb_magic);
    kprint(", flags=");
    kprint_bin(mb_info->flags);
    kprint(",\n-> mem_lower=");
    kprint_int(mb_info->mem_lower);
    kprint(" KiB, mem_upper=");
    kprint_int(mb_info->mem_upper);
    kprint(" KiB, boot_device=");
    kprint_hex(mb_info->boot_device);
    kprint(",\n-> cmdline=\"");
    kprint((char *)mb_info->cmdline);
    kprint("\"\n");

    kprint_ok_fail("[KDEBUG] init PIC 8259a", true);
    init_8259a();
    
    kprint_ok_fail("[KDEBUG] init PIT 8253", true);
    init_clock();

    kprint_ok_fail("[KDEBUG] init process scheduler", true);
    init_process();

    uint32_t kpid = process_create("kernel", SELECTOR_KERNEL_CODE, &process1,
                                   SELECTOR_KERNEL_DATA, &process_stack[0x1000]);
    kprint_ok_fail("[KDEBUG] create kernel process", kpid != (uint32_t)-1);
    uint32_t ipid = process_create("kernel", SELECTOR_USER_CODE, &process2,
                                   SELECTOR_USER_DATA, &process_stack[0x2000]);
    kprint_ok_fail("[KDEBUG] create init process", kpid != (uint32_t)-1);

    enable_interrupt();
    kprint_ok_fail("[KDEBUG] enable interrupt", true);

    // will not reach here
    for (;;)
    {
        asm("hlt");
    }
    return 0;
}
