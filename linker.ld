/* OUTPUT_FORMAT("elf64-x86-64")
 OUTPUT_ARCH(i386:x86-64) */
ENTRY(_start)

kernel_load_address = 0x00100000;
kernel_virtual_address = 0xC0000000;

SECTIONS
{
  . = 0x00100000;

  .text ALIGN(4K):
  {
    *(.text)
  }

  .rodata ALIGN(4K):
  {
    *(.rodata*)
  }

  .data ALIGN(4K):
  {
    *(.data)
  }

  .bss ALIGN(4K):
  {
    *(COMMON)
    *(.bss)
  }

  .pm ALIGN(4K):
  {
    *(.pm)
    end_of_kernel = (. + 4095) & (~ 4095); /* 4K align */
  }
}
