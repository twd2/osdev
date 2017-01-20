#ifndef _WDOS_STDLIB_STRING_H_
#define _WDOS_STDLIB_STRING_H_

#include <runtime/types.h>

uint8_t utoh(uint32_t x, char *buffer);
uint8_t ultoh(uint64_t x, char *buffer);
uint8_t utob(uint32_t x, char *buffer);
uint8_t ultob(uint64_t x, char *buffer);
uint8_t itos(int32_t x, char *buffer);
uint8_t utos(uint32_t x, char *buffer);
uint8_t btoh(uint8_t x, char *buffer);

uint32_t strlen(const char *src);
uint32_t strcpy(char *dest, const char *src);
const char *strsplit(const char *str, char delim, char *out_buffer);

#endif // _WDOS_STDLIB_STRING_H_
