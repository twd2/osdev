ENTRY(_start)

SECTIONS
{
  . = 0x00010000;

  .text ALIGN(4K):
  {
    *(.text)
    *(.rodata*)
    *(.data)
    *(COMMON)
    *(.bss)
  }
}
