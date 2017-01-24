#ifndef _WDOS_KERNEL_DRIVER_PIT8253_H_
#define _WDOS_KERNEL_DRIVER_PIT8253_H_

#include <runtime/types.h>

#define CLOCK_FREQ 1193180
#define PIT8253_COUNTER0 0x40
#define PIT8253_COUNTER1 0x41
#define PIT8253_COUNTER2 0x42
#define PIT8253_MCR 0x43

#define PIT8253_MCR_BINARY 0
#define PIT8253_MCR_MODE2 (2 << 1)
#define PIT8253_MCR_COUNTER0 (0 << 6)
#define PIT8253_MCR_COUNTER1 (1 << 6)
#define PIT8253_MCR_COUNTER2 (2 << 6)
#define PIT8253_MCR_WRITE_LH (3 << 4)

#define IRQ_FREQ 500 // Hz

void init_pit8253();
void set_clock_freq(uint32_t interval);

#endif // _WDOS_KERNEL_DRIVER_PIT8253_H_
