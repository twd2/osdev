#ifndef _WDOS_RUNTIME_TYPES_H_
#define _WDOS_RUNTIME_TYPES_H_

#define NULL ((void *)0)

#ifdef X86_64

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

// register type
typedef u64 ureg_t;
typedef s64 reg_t;

#else

// 32-bit system

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

typedef signed char       s8;
typedef signed short      s16;
typedef signed int        s32;
typedef signed long long  s64;

typedef s8 int8_t;
typedef u8 uint8_t;

typedef s16 int16_t;
typedef u16 uint16_t;

typedef s32 int32_t;
typedef u32 uint32_t;

typedef s64 int64_t;
typedef u64 uint64_t;

typedef s32 intptr_t;
typedef u32 uintptr_t;

typedef s32 ptrdiff_t;

typedef u32 size_t;

// register type
typedef u32 ureg_t;
typedef s32 reg_t;

#endif

typedef uint8_t bool;
#define true 1
#define false 0

#endif // _WDOS_RUNTIME_TYPES_H_
