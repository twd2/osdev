#ifndef _WDOS_KERNEL_IO_H_
#define _WDOS_KERNEL_IO_H_

#include <runtime/types.h>

void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
void outdw(uint16_t port, uint32_t value);

uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t indw(uint16_t port);

void io_delay();

#endif // _WDOS_KERNEL_IO_H_
