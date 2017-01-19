#ifndef _WDOS_SYSCALL_H_
#define _WDOS_SYSCALL_H_

#include <runtime/types.h>

#define SYSCALL_INTERRUPT "$0x80"

#define SYS_exit 0
#define SYS_test 1
#define SYS_add 2
#define SYS_yield 3
#define SYS_delay 4
#define SYSCALL_COUNT 256

void sys_exit(int exitcode);
int sys_test();
int sys_add(int a, int b);
void sys_yield();
void sys_delay(int x);

#endif // _WDOS_SYSCALL_H_
