#include "paging.h"

void mm_invalidate_tlb(void *va)
{
    asm volatile ("invlpg %0"
                  :
                  : "m"(*(uint8_t *)va));
}
