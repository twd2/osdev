#ifndef _WDOS_KERNEL_SYSCALL_IMPL_H_
#define _WDOS_KERNEL_SYSCALL_IMPL_H_

#include <runtime/types.h>
#include <interrupt.h>

#define SYS_exit 0
#define SYS_test 1
#define SYS_add 2
#define SYS_yield 3
#define SYS_delay 4
#define SYSCALL_COUNT 256

void init_syscall_impl();
void syscall_dispatch(uint32_t id, interrupt_frame_t *frame);

#endif // _WDOS_KERNEL_SYSCALL_IMPL_H_
