#ifndef _WDOS_BOOTLOADER_CDROM_STAGE2_UTIL_H_
#define _WDOS_BOOTLOADER_CDROM_STAGE2_UTIL_H_

#include <runtime/types.h>

#define assert(x) do { if (!(x)) die("\r\nAssertion failed.\r\n"); } while (0)

#ifndef NDEBUG
void debug_pause();
#else
#define debug_pause() do { } while (0)
#endif

void *palloc(uint32_t size); // p means physical, permanent
void die(const char *str);

#define is_power_of_2(x) (!((x) & ((x) - 1)))

#endif // _WDOS_BOOTLOADER_CDROM_STAGE2_UTIL_H_
