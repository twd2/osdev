#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <mm/page_list.h>

void print_all_node(page_list_t *list, size_t n)
{
    printf("========\n");
    printf("size=%u\n", list->size);
    page_node_t *nodes = list->nodes;
    for (size_t i = 0; i < n; ++i)
    {
        printf("[%p] prev=%p, next=%p, page=%u\n", &nodes[i], nodes[i].prev, nodes[i].next,
               (uint32_t)nodes[i].page);
    }
    printf("========\n");
}

void print_each(page_list_t *list)
{
    printf(">");
    for (page_node_t *iter = list->head->next; iter != list->tail; iter = iter->next)
    {
        printf("%u ", (uint32_t)iter->page);
    }
    printf("\n<");
    for (page_node_t *iter = list->tail->prev; iter != list->head; iter = iter->prev)
    {
        printf("%u ", (uint32_t)iter->page);
    }
    printf("\n");
}

int main()
{
    page_node_t *data = (page_node_t *)malloc(32 * sizeof(page_node_t));
    page_list_t *list = (page_list_t *)malloc(sizeof(page_list_t));
    page_list_init(list, data);
    print_all_node(list, 32);

    for (int i = 1; i < 6; ++i)
    {
        page_node_t *node = page_list_alloc_node(list);
        node->page = (void *)i;
        page_list_insert_as_prev(list, node, list->tail);
    }

    page_node_t *node = page_list_alloc_node(list);
    node->page = (void *)6;
    page_list_insert_as_prev(list, node, list->head->next->next->next->next);

    print_all_node(list, 32);

    node = node->prev;
    page_list_take_node(list, node);
    page_list_insert_as_prev(list, node, list->head->next);
    print_all_node(list, 32);
    print_each(list);

    page_node_t *node1 = list->head->next->next, *node2 = node1->next;
    page_list_swap_nodes(list, node1, node2);
    print_all_node(list, 32);
    print_each(list);

    node1 = list->head->next->next;
    node2 = node1->prev;
    page_list_swap_nodes(list, node1, node2);
    print_all_node(list, 32);
    print_each(list);

    printf("removed: %u\n", (uint32_t)page_list_remove_last_phy_node(list));
    printf("removed: %u\n", (uint32_t)page_list_remove_last_phy_node(list));
    print_all_node(list, 32);
    print_each(list);

    printf("removed: %u\n",
           (uint32_t)page_list_remove_node(list, list->head->next->next->next));
    print_all_node(list, 32);
    print_each(list);
    printf("removed: %u\n",
           (uint32_t)page_list_remove_node(list, list->head->next));
    printf("removed: %u\n",
           (uint32_t)page_list_remove_node(list, list->head->next));
    printf("removed: %u\n",
           (uint32_t)page_list_remove_node(list, list->head->next));
    print_all_node(list, 32);
    print_each(list);

    for (int i = 1; i < 32; i += 3)
    {
        page_node_t *node = page_list_alloc_node(list);
        node->page = (void *)i;
        page_list_insert_as_prev(list, node, list->tail);
    }
    print_all_node(list, 32);
    print_each(list);

    page_node_t target;
    target.page = (void *)5;
    printf("found: %p\n", page_list_find_gt(list, &target));

    node = list->head->next->next->next->next;
    printf("updating: %p\n", node);
    page_list_update_position(list, node);
    print_all_node(list, 32);
    print_each(list);
    node->page = (void *)5;
    page_list_update_position(list, node);
    print_all_node(list, 32);
    print_each(list);
    node->page = (void *)777;
    page_list_update_position(list, node);
    print_all_node(list, 32);
    print_each(list);
    return 0;
}
