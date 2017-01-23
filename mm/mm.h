#ifndef _WDOS_KERNEL_MM_MM_H_
#define _WDOS_KERNEL_MM_MM_H_

#include <runtime/types.h>
#include <multiboot.h>
#include <boot.h>

#ifndef X86_64
#define MM_PML4_SHIFT 0
#define MM_PML4_MASK 0
#define MM_PML3_SHIFT 0
#define MM_PML3_MASK 0
#define MM_PML2_SHIFT 22
#define MM_PML2_MASK 0b1111111111
#define MM_PML1_SHIFT 12
#define MM_PML1_MASK 0b1111111111
#else
#define MM_PML4_SHIFT 39
#define MM_PML4_MASK 0b111111111
#define MM_PML3_SHIFT 30
#define MM_PML3_MASK 0b111111111
#define MM_PML2_SHIFT 21
#define MM_PML2_MASK 0b111111111
#define MM_PML1_SHIFT 12
#define MM_PML1_MASK 0b111111111
#endif

#define PAGE_SIZE 0x1000
#define PAGE_SIZE_LOG2 12
#define BUCKET_COUNT 20

#define BIOS_MEM_MAP_MAX_COUNT 16

// TODO: paging
#define __VA(ptr) ((void *)((uintptr_t)(ptr) + 0))
#define __PA(ptr) ((void *)((uintptr_t)(ptr) + 0))

void *mm_align(void *ptr);
void *mm_palloc(size_t size);
void mm_init_mem(void *begin, void *end);
void init_mm(multiboot_info_t *mb_info);
void *mm_alloc_pages(size_t count);
void mm_free_pages(void *page);

extern void *palloc_mem_start;
extern void *free_mem_start;
extern void *free_mem_end;
extern uint32_t bios_mem_map_count;
extern memory_map_long_t bios_mem_map[BIOS_MEM_MAP_MAX_COUNT];

#endif // _WDOS_KERNEL_MM_MM_H_
