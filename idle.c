#include "idle.h"

#include <runtime/types.h>
#include <asm.h>
#include <process.h>

static uint8_t idle_stack[0x1000];

uint32_t idle_pid;

void init_idle()
{
    idle_pid = process_create_kernel_thread("IDLE", &idle_entry,
                                            &idle_stack[sizeof(idle_stack)]);
    kprint_ok_fail("[KDEBUG] create system idle process", idle_pid != (uint32_t)-1);
}

void idle_entry()
{
    kprint("[KDEBUG] I am system IDLE!\n");
    kprint("My PID=");
    kprint_int(get_pid());
    kprint("\n");
    while (true)
    {
        asm volatile ("hlt");
    }
}
