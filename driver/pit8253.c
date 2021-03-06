#include "pit8253.h"
#include <io.h>

void init_pit8253()
{
    set_clock_freq(IRQ_FREQ);
}

inline void set_clock_freq(uint32_t freq)
{
    uint16_t counter = (uint16_t)(CLOCK_FREQ / freq);
    outb(PIT8253_MCR,
         PIT8253_MCR_BINARY | PIT8253_MCR_MODE2 | PIT8253_MCR_COUNTER0 | PIT8253_MCR_WRITE_LH);
    io_delay();
    outb(PIT8253_COUNTER0, counter & 0xFF);
    io_delay();
    outb(PIT8253_COUNTER0, (counter >> 8) & 0xFF);
    io_delay();
}
