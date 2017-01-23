#include "page_fault.h"

static size_t page_fault_count = 0;

inline void *mm_page_fault_va()
{
    void *ret;
    asm volatile ("mov %%cr2, %0"
                  : "=r"(ret)
                  :)
    return ret;
}

size_t mm_page_fault_count()
{
    return page_fault_count;
}
