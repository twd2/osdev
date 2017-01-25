#include "buddy.h"

#include <mm/page_list.h>
#include <stdlib/bits.h>

void buddy_init(buddy_t *b, bucket_t *buckets, size_t bucket_count,
                page_node_t *nodes, size_t node_count, size_t page_size)
{
    b->buckets = buckets;
    b->bucket_count = bucket_count;
    b->nodes = nodes;
    b->node_count = node_count;
    b->page_size = page_size;
    // assert is_power_of_2(page_size)
    b->page_size_log2 = bits_log2(page_size);

    // init buckets
    for (size_t i = 0; i < bucket_count; ++i)
    {
        page_list_init(&(buckets[i].list), NULL);
    }
}

inline page_node_t *buddy_page_node(buddy_t *b, void *page)
{
    return &b->nodes[((uintptr_t)page >> b->page_size_log2)];
}

inline page_node_t *buddy_pfn_node(buddy_t *b, uintptr_t pfn)
{
    return &b->nodes[pfn];
}

inline void *buddy_align(buddy_t *b, void *ptr)
{
    // t / b->page_size == ceiling((uintptr_t)page / b->page_size)
    uintptr_t t = (uintptr_t)ptr - 1 + b->page_size;
    // t - t % b->page_size == int(t / b->page_size) * b->page_size
    return (void *)(t & ~(b->page_size - 1));
    // ceiling((uintptr_t)page / b->page_size) * b->page_size
}

void buddy_init_pages(buddy_t *b, void *begin, size_t length)
{
    // assert (uintptr_t)begin % b->page_size == 0
    uint8_t *page = buddy_align(b, begin);
    length -= page - (uint8_t *)begin;
    // length % b->page_size bytes ignored
    size_t page_count = length >> b->page_size_log2;
    for (ptrdiff_t bucket = b->bucket_count - 1; bucket >= 0; --bucket)
    {
        const size_t pages_per_item = 1 << bucket;
        const size_t bucket_item_count = page_count >> bucket; // page_count / pages_per_item
        for (size_t item = 0; item < bucket_item_count; ++item)
        {
            page_node_t *node = buddy_page_node(b, page);
            node->page = page;
            node->used = false;
            node->count = pages_per_item;

            page_list_t *list = &(b->buckets[bucket].list);
            page_list_insert_as_prev(list, node, list->tail);

            page += pages_per_item << b->page_size_log2;
        }
        page_count -= bucket_item_count << bucket; // bucket_item_count * pages_per_item
    }
    // assert page_count == 0
}

void *buddy_alloc_pages(buddy_t *b, size_t count)
{
    if (count > (1 << (b->bucket_count - 1)))
    {
        return NULL;
    }

    count = next_power_of_2(count);
    size_t bucket = bits_log2(count);

    ptrdiff_t available_bucket = -1;
    for (size_t i = bucket; i < b->bucket_count; ++i)
    {
        page_list_t *list = &(b->buckets[i].list);
        if (list->head->next != list->tail && !list->head->next->used)
        {
            available_bucket = i;
            break;
        }
    }

    if (available_bucket == -1)
    {
        return NULL;
    }

    for (ptrdiff_t i = available_bucket; i > bucket; --i)
    {
        page_list_t *list = &(b->buckets[i].list);
        page_list_t *list_smaller = &(b->buckets[i - 1].list);

        page_node_t *node1 = list->head->next;
        // assert !node1->used
        // split
        // TODO: flags
        page_list_take_node(list, node1);
        node1->count >>= 1;

        void *page2 = (void *)((uintptr_t)node1->page + (node1->count << b->page_size_log2));
        page_node_t *node2 = buddy_page_node(b, page2);
        node2->page = page2;
        node2->used = false;
        node2->count = node1->count;

        page_list_insert_as_prev(list_smaller, node2, list_smaller->head->next);
        page_list_insert_as_prev(list_smaller, node1, node2);
    }

    page_list_t *list = &(b->buckets[bucket].list);
    page_node_t *node = list->head->next;
    node->used = true;
    // just move to last
    page_list_take_node(list, node);
    page_list_insert_as_prev(list, node, list->tail);
    return node->page;
}

void buddy_free_pages(buddy_t *b, void *page)
{
    void *page_aligned = buddy_align(b, page);
    if (page_aligned != page)
    {
        // assert false
    }

    page_node_t *node = buddy_page_node(b, page_aligned);
    // assert node->used

    ptrdiff_t bucket = bits_log2(node->count);

    // assert bucket != -1
    page_list_t *list = &(b->buckets[bucket].list);
    node->used = false;
    // just move to first
    page_list_take_node(list, node);
    page_list_insert_as_prev(list, node, list->head->next);

    // TODO: combine
}
