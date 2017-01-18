#include "util.h"

#include "bios.h"

void *malloc(uint32_t size)
{
    static uint8_t *buffer = (void *)0x30000;
    uint8_t *old_buffer = buffer;
    buffer += size;
    return old_buffer;
}

void free(void *ptr)
{

}

void die(const char *str)
{
    print(str);
    while (true)
    {
        asm("cli\n"
            "hlt");
    }
}
