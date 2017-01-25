#ifndef _WDOS_KERNEL_MM_PAGING_H_
#define _WDOS_KERNEL_MM_PAGING_H_

#include <runtime/types.h>

#ifdef X86_64
#error Not Implemented.
#endif

#ifndef X86_64
#define MM_PML4_SHIFT 0
#define MM_PML4_MASK 0
#define MM_PML3_SHIFT 0
#define MM_PML3_MASK 0
#define MM_PML2_SHIFT 22
#define MM_PML2_MASK 0b1111111111
#define MM_PML1_SHIFT 12
#define MM_PML1_MASK 0b1111111111
#else
#define MM_PML4_SHIFT 39
#define MM_PML4_MASK 0b111111111
#define MM_PML3_SHIFT 30
#define MM_PML3_MASK 0b111111111
#define MM_PML2_SHIFT 21
#define MM_PML2_MASK 0b111111111
#define MM_PML1_SHIFT 12
#define MM_PML1_MASK 0b111111111
#endif

// X = ML4, ML3, ML2, ML1 (ML4, DPT, D, T for x86, x86-64)
#define MM_PAGING_PXE_P 0x1
#define MM_PAGING_PXE_RW 0x2
#define MM_PAGING_PXE_US 0x4
#define MM_PAGING_PXE_PWT 0x8
#define MM_PAGING_PXE_PCD 0x10
#define MM_PAGING_PXE_A 0x20
#define MM_PAGING_PXE_D 0x40
#define MM_PAGING_PXE_PS 0x80
#define MM_PAGING_PXE_PAT 0x80
#define MM_PAGING_PXE_G 0x100

#ifdef X86_64
#define MM_PAGING_PXE_NX (1UL << 63)
#endif

typedef uintptr_t pml4e_t;
typedef uintptr_t pml3e_t;
typedef uintptr_t pml2e_t;
typedef uintptr_t pml1e_t;

typedef struct pml4
{
    pml4e_t entries[MM_PML4_MASK + 1];
} pml4_t;

typedef struct pml3
{
    pml3e_t entries[MM_PML3_MASK + 1];
} pml3_t;

typedef struct pml2
{
    pml2e_t entries[MM_PML2_MASK + 1];
} pml2_t;

typedef struct pml1
{
    pml1e_t entries[MM_PML1_MASK + 1];
} pml1_t;

void mm_invalidate_tlb(void *va);

#endif // _WDOS_KERNEL_MM_PAGING_H_
