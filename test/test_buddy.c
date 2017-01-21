#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mm/buddy.h>
#include <mm/page_list.h>

void mkmem(void *begin, void *end)
{
    void *ret = mmap(begin, end - begin, PROT_EXEC | PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (ret != begin)
    {
        perror("mmap");
        exit(1);
    }
    memset(begin, 0, end - begin);
}

void print_bucket(bucket_t *b)
{
    page_list_t *list = &b->list;
    for (page_node_t *iter = list->head->next; iter != list->tail; iter = iter->next)
    {
        printf("  [%p]page begin=%p, count=%u, used=%d\n", iter,
               iter->page, iter->count, iter->used);
    }
}

void print_buddy(buddy_t *b)
{
    printf("======\n");
    for (size_t i = 0; i < b->bucket_count; ++i)
    {
        printf("[BUCKET #%u %u pages]\n", i, 1 << i);
        print_bucket(&b->buckets[i]);
    }
    printf("======\n");
}

int main()
{
    // act as physical memory
    printf("mmapping...");
    fflush(stdout);
    void *free1_begin = (void *)0x100000;
    void *free1_end = (void *)0x8000000;
    mkmem(free1_begin, free1_end);

    void *free2_begin = (void *)0x9000000;
    void *free2_end = (void *)0xF0000000;
    mkmem(free2_begin, free2_end);
    printf("done\n");

    const size_t bucket_count = 20;
    const size_t page_size = 0x1000;
    const size_t node_count = (uintptr_t)free2_end / page_size;
    buddy_t buddy;
    bucket_t *buckets = (bucket_t *)free1_begin;
    free1_begin += bucket_count * sizeof(bucket_t);
    page_node_t *nodes = (page_node_t *)free1_begin;
    free1_begin += node_count * sizeof(page_node_t);
    
    buddy_init(&buddy, buckets, bucket_count, nodes, node_count, page_size);
    buddy_init_pages(&buddy, free1_begin, free1_end - free1_begin);
    print_buddy(&buddy);
    buddy_init_pages(&buddy, free2_begin, free2_end - free2_begin);
    print_buddy(&buddy);
    
    void *firstp = NULL;
    for (int i = 0; i < 100000000; ++i)
    {
        void *p = buddy_alloc_pages(&buddy, ((i & 0b1111111) << 5) + 1);
        /*if (i == 0)
        {
            firstp = p;
        }*/
        page_node_t *node = buddy_page_node(&buddy, p);
        //print_buddy(&buddy);
        if (p)
        {
            //printf("alloc: [%p]page begin=%p, count=%u, used=%d\n", node,
            //       node->page, node->count, node->used);
            buddy_free_pages(&buddy, p);
        }
        else
        {
            printf("alloc failed!\n");
            return 1;
        }
    }
    //print_buddy(&buddy);
    //buddy_free_pages(&buddy, firstp);
    print_buddy(&buddy);
    return 0;
}
