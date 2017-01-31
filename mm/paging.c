#include "paging.h"

#include <mm/mm.h>
#include <tty.h>

// loader.asm, for x86
extern pml2_t pd_ptr;
extern pml1_t pt0_ptr;
extern pml1_t pt1_ptr;
static pml1_t *pt_rest;

inline void mm_enable_page_global()
{
    asm volatile ("movl %%cr4, %%eax\n"
                  "or $0x80, %%eax\n"
                  "movl %%eax, %%cr4"
                  :
                  :
                  : "eax");
}

inline void mm_load_pml4(void *pml4)
{
    asm volatile ("movl %0, %%cr3"
                  :
                  : "r"(__PA(pml4))
                  :);
}

void init_mm_paging()
{
    // map [0, min(512MiB, free_mem_end))
    uintptr_t hi = (uintptr_t)free_mem_end;
    if (hi > MM_PAGING_DIRECT_MAP_MAX)
    {
        hi = MM_PAGING_DIRECT_MAP_MAX;
    }
    size_t pml1e_count = (hi + PAGE_SIZE - 1) / PAGE_SIZE;
    size_t pml1_count = (pml1e_count + MM_PML1_MASK + 1 - 1) / (MM_PML1_MASK + 1);
    if (pml1_count <= 2)
    {
        return; // already mapped, loader.asm
    }
    pt_rest = (pml1_t *)mm_palloc((pml1_count - 2) * PAGE_SIZE); // -2 : pt0, pt1
    if ((uintptr_t)pt_rest & 0xfff)
    {
        // not aligned!
        kprint("[KDEBUG] Assertion failed.\n");
        while (true);
    }

    // fill pml1
    pml1e_t pml1e = 2048 * 4096; // loader.asm
    for (size_t i = 2; i < pml1_count; ++i)
    {
        // for each pml1...
        for (size_t j = 0; j < MM_PML1_MASK + 1; ++j)
        {
            pt_rest[i - 2].entries[j] = pml1e | MM_PAGING_PXE_P | MM_PAGING_PXE_RW |
                                        MM_PAGING_PXE_G | MM_PAGING_PXE_US; // TODO
            pml1e += PAGE_SIZE;
        }
    }

    // fill pml2
    for (size_t i = 2; i < pml1_count; ++i)
    {
        pd_ptr.entries[768 + i] = (uintptr_t)__PA(&pt_rest[i - 2]) | MM_PAGING_PXE_P |
                                  MM_PAGING_PXE_RW | MM_PAGING_PXE_US; // TODO
    }

    kprint("[KDEBUG] mapped 0x00000000~");
    kprint_hex(hi);
    kprint("\n");

    mm_enable_page_global();
}

inline void mm_invalidate_tlb(void *va)
{
    asm volatile ("invlpg %0"
                  :
                  : "m"(*(uint8_t *)va)
                  : "memory");
}
