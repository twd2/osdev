global _start
extern kmain

MULTIBOOT_HEADER_MAGIC equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS equ 0
MULTIBOOT_HEADER_CHECKSUM equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
KERNEL_STACK_SIZE equ 0x4000 ; 16KiB

section .bss
align 4
kernel_stack_base:
resb KERNEL_STACK_SIZE

section .text
align 4
bits 32

dd MULTIBOOT_HEADER_MAGIC
dd MULTIBOOT_HEADER_FLAGS
dd MULTIBOOT_HEADER_CHECKSUM

_start:
  nop
  nop
  nop
  nop
  mov esp, kernel_stack_base + KERNEL_STACK_SIZE
  push ebx
  push eax
  call kmain
  add esp, 8
  cli
  hlt

