#include "page_list.h"

void page_list_init(page_list_t *list, page_node_t *nodes)
{
    page_node_t *head, *tail;

    head = &list->_head;
    tail = &list->_tail;

    head->prev = NULL;
    head->next = tail;
    tail->prev = head;
    tail->next = NULL;

    list->size = 0;
    list->head = head;
    list->tail = tail;
    list->nodes = nodes;
}

page_node_t *page_list_alloc_node(page_list_t *list)
{
    page_node_t *n = &list->nodes[list->size];
    ++list->size;
    return n;
}

inline void page_list_insert_as_prev(page_list_t *list, page_node_t *node, page_node_t *next)
{
    node->next = next;
    node->prev = next->prev;
    next->prev->next = node;
    next->prev = node;
}

inline void page_list_take_node(page_list_t *list, page_node_t *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = node->next = NULL;
}

inline page_node_t *page_list_last_node(page_list_t *list)
{
    if (list->tail->prev == list->head)
    {
        return NULL;
    }
    return list->tail->prev;
}

inline page_node_t *page_list_last_phy_node(page_list_t *list)
{
    return &list->nodes[list->size - 1];
}

inline void page_list_swap_nodes(page_list_t *list,
                                 page_node_t *node1, page_node_t *node2)
{
    if (node1 == node2)
    {
        return;
    }

    // swap data
    void *t = node1->page;
    node1->page = node2->page;
    node2->page = t;

    // swap links
    if (node1->next == node2) // node1 is next to node2
    {
        node1->prev->next = node2;
        node2->prev = node1->prev;

        node2->next->prev = node1;
        node1->next = node2->next;

        node2->next = node1;
        node1->prev = node2;
    }
    else if (node2->next == node1)
    {
        node2->prev->next = node1;
        node1->prev = node2->prev;

        node1->next->prev = node2;
        node2->next = node1->next;

        node1->next = node2;
        node2->prev = node1;
    }
    else
    {
        node1->prev->next = node2;
        node1->next->prev = node2;

        node2->prev->next = node1;
        node2->next->prev = node1;

        page_node_t *tmp;

        tmp = node1->prev;
        node1->prev = node2->prev;
        node2->prev = tmp;
        tmp = node1->next;
        node1->next = node2->next;
        node2->next = tmp;
    }
}

inline void *page_list_remove_last_phy_node(page_list_t *list)
{
    page_node_t *node = page_list_last_phy_node(list);
    if (!node)
    {
        return NULL;
    }
    void *page = node->page;
    page_list_take_node(list, node);
    --list->size;
    return page;
}

void *page_list_remove_node(page_list_t *list, page_node_t *node)
{
    void *page = node->page;
    page_list_swap_nodes(list, node, page_list_last_phy_node(list)); // make node last phy.
    node = page_list_last_phy_node(list);
    page_list_remove_last_phy_node(list);
    return page;
}

static inline bool page_list_comparator_gt(page_list_t *list, page_node_t *a, page_node_t *b)
{
    // guard tail is greater than everything
    return (a == list->tail) || (a->page > b->page); // TODO: used > unused
}

inline page_node_t *page_list_find_gt(page_list_t *list, page_node_t *node)
{
    for (page_node_t *iter = list->head->next; iter != NULL; iter = iter->next)
    {
        if (page_list_comparator_gt(list, iter, node))
        {
            return iter;
        }
    }
    // should not be here
    return NULL;
}

// page changed or used/unused flag changed
void page_list_update_position(page_list_t *list, page_node_t *node)
{
    page_list_take_node(list, node);
    page_list_insert_as_prev(list, node, page_list_find_gt(list, node));
}
