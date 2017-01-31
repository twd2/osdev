#include "mm.h"

#include <runtime/types.h>
#include <multiboot.h>
#include <boot.h>
#include <mm/paging.h>
#include <mm/buddy.h>
#include <tty.h>

static bool mm_inited = false;

// loader.asm
extern uint8_t _kernel_load_address;
extern uint8_t _kernel_virtual_base;
void *const kernel_load_address = &_kernel_load_address;
void *const kernel_virtual_base = &_kernel_virtual_base;

extern uint8_t _end_of_kernel;
void *palloc_mem_start = NULL;
void *free_mem_start = NULL;
void *free_mem_end = NULL; // [free_mem_start, free_mem_end)

uint32_t bios_mem_map_count;
memory_map_long_t bios_mem_map[BIOS_MEM_MAP_MAX_COUNT];

static buddy_t buddy;

inline void *mm_align(void *ptr)
{
    uintptr_t t = (uintptr_t)ptr - 1 + PAGE_SIZE;
    return (void *)(t & ~(PAGE_SIZE - 1));
}

inline void *mm_palloc(size_t size)
{
    if (mm_inited)
    {
        // assert false
        return NULL;
    }
    void *old_free_mem_start = free_mem_start;
    free_mem_start = (void *)((uint8_t *)free_mem_start + size);
    return __VA(old_free_mem_start);
}

static inline void copy_mem_map(multiboot_info_t *mb_info)
{
    memory_map_long_t *const mmap = bios_mem_map;
    multiboot_memory_map_t *mb_mmap =
        (multiboot_memory_map_t *)__VA((void *)mb_info->mmap_addr);
    multiboot_memory_map_t *mb_mmap_end =
        (multiboot_memory_map_t *)((uint8_t *)mb_mmap + mb_info->mmap_length);

    bios_mem_map_count = 0;
    while (mb_mmap != mb_mmap_end)
    {
        if (mb_mmap->type == MEMORY_TYPE_USABLE)
        {
            mmap[bios_mem_map_count].base = mb_mmap->addr;
            mmap[bios_mem_map_count].length = mb_mmap->len;
            ++bios_mem_map_count;
        }

        // next
        mb_mmap = (multiboot_memory_map_t *)((uint8_t *)mb_mmap + mb_mmap->size +
                                             sizeof(mb_mmap->size));
    }
}

static void print_bucket(bucket_t *b)
{
    page_list_t *list = &b->list;
    for (page_node_t *iter = list->head->next; iter != list->tail; iter = iter->next)
    {
        kprint(" [");
        kprint_hex(iter);
        kprint("]");
        kprint_hex(iter->page);
        kprint(",");
        kprint_byte(iter->used);
        kprint(" ");
    }
    kprint("\n");
}

static void print_buddy(buddy_t *b)
{
    kprint("======\n");
    for (size_t i = 0; i < b->bucket_count; ++i)
    {
        kprint("[BUCKET #");
        kprint_uint(i);
        kprint(" ");
        kprint_uint(1 << i);
        kprint(" pages] ");
        print_bucket(&b->buckets[i]);
    }
    kprint("======\n");
}

void init_mm(multiboot_info_t *mb_info)
{
    copy_mem_map(mb_info);
    palloc_mem_start = __PA(&_end_of_kernel);
    free_mem_start = __PA(&_end_of_kernel);
    free_mem_end = (void *)((mb_info->mem_upper << 10) + 0x100000);

    // paging: map [0, free_mem_end)
    init_mm_paging();

    bucket_t *buckets = (bucket_t *)mm_palloc(BUCKET_COUNT * sizeof(bucket_t));

    const size_t node_count = (uintptr_t)free_mem_end / PAGE_SIZE;
    page_node_t *nodes = (page_node_t *)mm_palloc(node_count * sizeof(page_node_t));

    mm_inited = true; // forbid mm_palloc
    free_mem_start = mm_align(free_mem_start);
    buddy_init(&buddy, buckets, BUCKET_COUNT, nodes, node_count, PAGE_SIZE);
    buddy_init_pages(&buddy, free_mem_start, free_mem_end - free_mem_start);
    print_buddy(&buddy);
    for (;;);
}

void *mm_alloc_pages(size_t count)
{
    return buddy_alloc_pages(&buddy, count);
}

void mm_free_pages(void *page)
{
    buddy_free_pages(&buddy, page);
}
