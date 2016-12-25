#include <io.h>

void outb(uint16_t port, uint8_t value)
{
    asm("out %%al, %%dx"
        :
        : "d"(port), "a"(value)
       );
}

void outw(uint16_t port, uint16_t value)
{
    asm("out %%ax, %%dx"
        :
        : "d"(port), "a"(value)
       );
}

void outdw(uint16_t port, uint32_t value)
{
    asm("out %%eax, %%dx"
        :
        : "d"(port), "a"(value)
       );
}

uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm("xor %%eax, %%eax\n"
        "in %%dx, %%al"
        : "=a"(ret)
        : "d"(port)
       );
   return ret;
}

uint16_t inw(uint16_t port)
{
    uint16_t ret;
    asm("xor %%eax, %%eax\n"
        "in %%dx, %%ax"
        : "=a"(ret)
        : "d"(port)
       );
   return ret;
}

uint32_t indw(uint16_t port)
{
    uint32_t ret;
    asm("in %%dx, %%eax"
        : "=a"(ret)
        : "d"(port)
       );
   return ret;
}

void io_delay()
{
    asm("nop\nnop\nnop\nnop");
}
