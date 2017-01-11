#ifndef _WDOS_SYSCALL_H_
#define _WDOS_SYSCALL_H_

#include <runtime/types.h>

#define SYSCALL_INTERRUPT "$0x80"

uint32_t syscall0(uint32_t);
uint32_t syscall1(uint32_t, uint32_t);
uint32_t syscall2(uint32_t, uint32_t, uint32_t);
uint32_t syscall3(uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t syscall4(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t syscall5(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t syscall6(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

#define SYS_exit 0
#define SYS_test 1
#define SYS_add 2
#define SYS_yield 3

void sys_exit(int);
int sys_test();
int sys_add(int, int);
void sys_yield();

#endif // _WDOS_SYSCALL_H_
