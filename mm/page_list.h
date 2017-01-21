#ifndef _WDOS_KERNEL_MM_PAGE_LIST_H_
#define _WDOS_KERNEL_MM_PAGE_LIST_H_

#include <runtime/types.h>

struct page_node;
typedef struct page_node page_node_t;
struct page_node
{
    page_node_t *prev, *next;
    void *page; // physical address
    size_t count; // how many pages does this node have?
    bool used : 1;
    uintptr_t flags : sizeof(uintptr_t) * 8 - 1; // rest unused bits
};

typedef struct page_list
{
    page_node_t *head, *tail; // guard
    size_t size;
    // size_t capacity;
    page_node_t *nodes; // fixed-size array
} page_list_t;

void page_list_init(page_list_t *list, page_node_t *nodes);
page_node_t *page_list_alloc_node(page_list_t *list);
void page_list_insert_as_prev(page_list_t *list, page_node_t *node, page_node_t *next);
void page_list_take_node(page_list_t *list, page_node_t *node); // breaks links
page_node_t *page_list_last_node(page_list_t *list); // logic
page_node_t *page_list_last_phy_node(page_list_t *list); // phy
void *page_list_remove_last_phy_node(page_list_t *list); // returns page
void page_list_swap_nodes(page_list_t *list, page_node_t *node1, page_node_t *node2);
void *page_list_remove_node(page_list_t *list, page_node_t *node); // returns page
page_node_t *page_list_find_gt(page_list_t *list, page_node_t *node); // first greater than
void page_list_update_position(page_list_t *list, page_node_t *node);

#endif // _WDOS_KERNEL_MM_PAGE_LIST_H_
