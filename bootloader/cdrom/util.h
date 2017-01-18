#ifndef _WDOS_BOOTLOADER_CDROM_STAGE2_UTIL_H_
#define _WDOS_BOOTLOADER_CDROM_STAGE2_UTIL_H_

#include <runtime/types.h>

#define assert(x) do { if (!(x)) die("\r\nAssertion failed.\r\n"); } while (0)

void *malloc(uint32_t size); // a shabby malloc :)
void free(void *ptr);
void die(const char *str);

#define is_power_of_2(x) (!((x) & ((x) - 1)))

#endif // _WDOS_BOOTLOADER_CDROM_STAGE2_UTIL_H_
