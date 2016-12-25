#ifndef _WDOS_RUNTIME_TYPES_H_
#define _WDOS_RUNTIME_TYPES_H_

#define NULL 0

// LP64
// See more at: http://wiki.osdev.org/X86-64

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef unsigned long   u64;

typedef signed char     s8;
typedef signed short    s16;
typedef signed int      s32;
typedef signed long     s64;

typedef s8 int8_t;
typedef u8 uint8_t;

typedef s16 int16_t;
typedef u16 uint16_t;

typedef s32 int32_t;
typedef u32 uint32_t;

typedef s64 int64_t;
typedef u64 uint64_t;

typedef s64 intptr_t;
typedef u64 uintptr_t;

typedef s64 ptrdiff_t;

typedef u64 size_t;

typedef uint8_t bool;
#define true 1
#define false 0

#endif // _WDOS_RUNTIME_TYPES_H_
