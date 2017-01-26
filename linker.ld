/* OUTPUT_FORMAT("elf64-x86-64")
 OUTPUT_ARCH(i386:x86-64) */
ENTRY(_start)

_kernel_load_address = 0x00100000;
_kernel_virtual_base = 0xC0000000;

SECTIONS
{
  . = _kernel_load_address;

  .init32.text ALIGN(4K):
  {
    *(.init32.text)
  }

  . += _kernel_virtual_base;

  .text ALIGN(4K) : AT(ADDR(.text) - _kernel_virtual_base)
  {
    *(.text)
  }

  .rodata ALIGN(4K): AT(ADDR(.rodata) - _kernel_virtual_base)
  {
    *(.rodata*)
  }

  .data ALIGN(4K): AT(ADDR(.data) - _kernel_virtual_base)
  {
    *(.data)
  }

  .bss ALIGN(4K): AT(ADDR(.bss) - _kernel_virtual_base)
  {
    *(COMMON)
    *(.bss)
  }

  .pm ALIGN(4K): AT(ADDR(.pm) - _kernel_virtual_base)
  {
    *(.pm)
  }

  .pd ALIGN(4K): AT(ADDR(.pd) - _kernel_virtual_base)
  {
    *(.pd)
    _end_of_kernel = (. + 4095) & (~ 4095); /* 4K align */
  }
}
