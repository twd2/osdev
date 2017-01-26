#ifndef _WDOS_KERNEL_MM_MM_H_
#define _WDOS_KERNEL_MM_MM_H_

#include <runtime/types.h>
#include <multiboot.h>
#include <boot.h>

#define PAGE_SIZE 0x1000
#define PAGE_SIZE_LOG2 12
#ifndef X86_64
#define BUCKET_COUNT 20 // 32 - 12
#else
#define BUCKET_COUNT 36 // 48 - 12
#endif

#define BIOS_MEM_MAP_MAX_COUNT 16

#define __VA(ptr) ((void *)((uintptr_t)(ptr) + (uintptr_t)(kernel_virtual_base)))
#define __PA(ptr) ((void *)((uintptr_t)(ptr) - (uintptr_t)(kernel_virtual_base)))

void *mm_align(void *ptr);
void *mm_palloc(size_t size);
void mm_init_mem(void *begin, void *end);
void init_mm(multiboot_info_t *mb_info);
void *mm_alloc_pages(size_t count);
void mm_free_pages(void *page);

extern void *const kernel_load_address;
extern void *const kernel_virtual_base;
extern void *palloc_mem_start;
extern void *free_mem_start;
extern void *free_mem_end;
extern uint32_t bios_mem_map_count;
extern memory_map_long_t bios_mem_map[BIOS_MEM_MAP_MAX_COUNT];

#endif // _WDOS_KERNEL_MM_MM_H_
