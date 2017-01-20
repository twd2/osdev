#ifndef _WDOS_KERNEL_MM_PAGE_LIST_H_
#define _WDOS_KERNEL_MM_PAGE_LIST_H_

#include <runtime/types.h>

struct page_node;
typedef struct page_node page_node_t;
struct page_node
{
    page_node_t *prev, *next;
    void *page; // physical address
    bool lr : 1;
    uintptr_t flags : sizeof(uintptr_t) * 8 - 1; // rest unused
};

typedef struct page_list
{
    page_node_t *head, *tail; // guard
    size_t size;
    page_node_t *data; // fixed-size array
} page_list_t;

void page_list_init(page_list_t *list, page_node_t *data);
page_node_t *page_list_alloc_node(page_list_t *list);
void page_list_insert_as_prev(page_list_t *list, page_node_t *node, page_node_t *next);
void *page_list_remove_node(page_list_t *list, page_node_t *node); // returns page
page_node_t *page_list_last_node(page_list_t *list); // returns page
void page_list_swap_nodes(page_list_t *list, page_node_t *node1, page_node_t *node2);
void *page_list_remove_last_node(page_list_t *list, page_node_t *node); // returns page
page_node_t *page_list_find(page_list_t *list, void *page); // first greater than

#endif // _WDOS_KERNEL_MM_LIST_H_
