#include "idle.h"

#include <runtime/types.h>
#include <asm.h>
#include <thread.h>

static uint8_t idle_stack[0x1000];

uint32_t idle_pid;

void init_idle()
{
    idle_pid = thread_create_kernel("IDLE", &idle_entry, &idle_stack[sizeof(idle_stack)]);
    kprint_ok_fail("[KDEBUG] create system idle process", idle_pid != (uint32_t)-1);
}

void idle_entry()
{
    thread_set_priority(THREAD_PRIORITY_MIN);
    kprint("[IDLE] I am system IDLE!\n");
    kprint("[IDLE] My PID=");
    kprint_int(get_pid());
    kprint("\n");
    while (true)
    {
        sys_yield();
        // asm volatile ("hlt");
    }
}
