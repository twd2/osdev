#ifndef _WDOS_KERNEL_MM_MM_H_
#define _WDOS_KERNEL_MM_MM_H_

void init_mm();

void mm_init_mem(void *begin, void *end);

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

#endif // _WDOS_KERNEL_MM_MM_H_
