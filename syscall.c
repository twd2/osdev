#include <syscall.h>

uint32_t syscall0(uint32_t id)
{
    uint32_t ret;
    asm("int " SYSCALL_INTERRUPT
        : "=a"(ret)
        : "a"(id));
    return ret;
}

uint32_t syscall1(uint32_t id, uint32_t arg1)
{
    uint32_t ret;
    asm("int " SYSCALL_INTERRUPT
        : "=a"(ret)
        : "a"(id), "b"(arg1));
    return ret;
}

uint32_t syscall2(uint32_t id, uint32_t arg1, uint32_t arg2)
{
    uint32_t ret;
    asm("int " SYSCALL_INTERRUPT
        : "=a"(ret)
        : "a"(id), "b"(arg1), "c"(arg2));
    return ret;
}

uint32_t syscall3(uint32_t id, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    uint32_t ret;
    asm("int " SYSCALL_INTERRUPT
        : "=a"(ret)
        : "a"(id), "b"(arg1), "c"(arg2), "d"(arg3));
    return ret;
}

uint32_t syscall4(uint32_t id, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
{
    uint32_t ret;
    asm("int " SYSCALL_INTERRUPT
        : "=a"(ret)
        : "a"(id), "b"(arg1), "c"(arg2), "d"(arg3), "D"(arg4));
    return ret;
}

uint32_t syscall5(uint32_t id, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    uint32_t ret;
    asm("int " SYSCALL_INTERRUPT
        : "=a"(ret)
        : "a"(id), "b"(arg1), "c"(arg2), "d"(arg3), "D"(arg4), "S"(arg5));
    return ret;
}

void sys_exit(int exitcode)
{
    syscall1(SYS_exit, exitcode);
}

int sys_test()
{
    return (int)syscall0(SYS_test);
}

int sys_add(int a, int b)
{
    return (int)syscall2(SYS_add, (uint32_t)a, (uint32_t)b);
}

void sys_yield()
{
    syscall0(SYS_yield);
}
