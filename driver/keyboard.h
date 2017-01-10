#ifndef _WDOS_KERNEL_DRIVER_KEYBOARD_H_
#define _WDOS_KERNEL_DRIVER_KEYBOARD_H_

#include <runtime/types.h>
#include <interrupt.h>

#define KEYBOARD_BUFFER 0x60

void init_keyboard();
uint8_t keyboard_read();
void keyboard_irq_handler(uint8_t, interrupt_frame_t*);

#endif // _WDOS_KERNEL_DRIVER_KEYBOARD_H_
