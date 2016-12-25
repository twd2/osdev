#ifndef _WDOS_STDLIB_MEMORY_H_
#define _WDOS_STDLIB_MEMORY_H_

#include <runtime/types.h>

uint32_t memcpy(void *dest, const void *src, uint32_t num);
uint32_t strlen(const char *src);
uint32_t strcpy(char *dest, const char *src);

#endif // _WDOS_STDLIB_MEMORY_H_
