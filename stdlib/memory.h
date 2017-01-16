#ifndef _WDOS_STDLIB_MEMORY_H_
#define _WDOS_STDLIB_MEMORY_H_

#include <runtime/types.h>

uint32_t memset(void *dest, uint8_t value, uint32_t num);
uint32_t memcpy(void *dest, const void *src, uint32_t num);
int memcmp(const void *a, const void *b, uint32_t num);
uint32_t strlen(const char *src);
uint32_t strcpy(char *dest, const char *src);
char *strsplit(const char *str, char delim, char *out_buffer);

#endif // _WDOS_STDLIB_MEMORY_H_
