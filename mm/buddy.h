#ifndef _WDOS_KERNEL_MM_BUDDY_H_
#define _WDOS_KERNEL_MM_BUDDY_H_

#include <runtime/types.h>
#include <mm/page_list.h>

typedef struct bucket
{
    page_list_t list;
} bucket_t;

typedef struct buddy
{
    bucket_t *buckets;
    size_t bucket_count;
    page_node_t *nodes; // first 2 * bucket_count nodes are for heads and tails of lists.
    size_t node_count;
    size_t page_size;
    uint8_t page_size_log2;
    uint8_t reserved[sizeof(ureg_t) - 1];
} buddy_t;

void buddy_init(buddy_t *b, bucket_t *buckets, size_t bucket_count,
                page_node_t *nodes, size_t node_count, size_t page_size);
page_node_t *buddy_page_node(buddy_t *b, void *page);
page_node_t *buddy_pfn_node(buddy_t *b, uintptr_t pfn);
void *buddy_align(buddy_t *b, void *ptr);
void buddy_init_pages(buddy_t *b, void *begin, size_t length);
void *buddy_alloc_pages(buddy_t *b, size_t count);
void buddy_free_pages(buddy_t *b, void *page);

#endif // _WDOS_KERNEL_MM_BUDDY_H_
