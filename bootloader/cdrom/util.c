#include "util.h"

#include "bios.h"

#ifndef NDEBUG
void debug_pause()
{
    print("Press any key to continue...\r\n");
    getchar();
}
#endif

void *palloc(uint32_t size)
{
    static uint8_t *buffer = (void *)0x30000;
    uint8_t *old_buffer = buffer;
    buffer += size;
    return (void *)old_buffer;
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
