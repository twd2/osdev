#ifndef _WDOS_STDLIB_STRING_H_
#define _WDOS_STDLIB_STRING_H_

#include <runtime/types.h>

uint8_t utoh(uint32_t x, char *buffer);
uint8_t ultoh(uint64_t x, char *buffer);
uint8_t utob(uint32_t x, char *buffer);
uint8_t itos(int32_t x, char *buffer);
uint8_t btoh(uint8_t x, char *buffer);

#endif // _WDOS_STDLIB_STRING_H_
