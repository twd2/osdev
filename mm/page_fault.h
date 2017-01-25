#ifndef _WDOS_KERNEL_MM_PAGE_FAULT_H_
#define _WDOS_KERNEL_MM_PAGE_FAULT_H_

#include <runtime/types.h>

void *mm_page_fault_va();
size_t mm_page_fault_count();

#endif // _WDOS_KERNEL_MM_PAGE_FAULT_H_
