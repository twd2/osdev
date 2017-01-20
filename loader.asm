%include "descriptor.inc"

global _start
global tss_ptr
global gdt32_tss
global idt_ptr
global enter_ring3
global get_eflags

; global constants
global SELECTOR_KERNEL_CODE
global SELECTOR_KERNEL_DATA
global SELECTOR_USER_CODE
global SELECTOR_USER_DATA
global SELECTOR_TSS
global TSS_LENGTH

extern kmain
extern prepare_tss_gdt_entry
extern prepare_idt
extern interrupt_handler

MULTIBOOT_HEADER_MAGIC equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS equ 0b10 ; mem_*, mmap_*
MULTIBOOT_HEADER_CHECKSUM equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
KERNEL_STACK_SIZE equ 0x4000 ; 16KiB

section .bss
align 4
kernel_stack_base:
resb KERNEL_STACK_SIZE
isr_stack_base:
resb KERNEL_STACK_SIZE
user_stack_base:
resb KERNEL_STACK_SIZE

section .text
align 4
bits 32

; multiboot headers
dd MULTIBOOT_HEADER_MAGIC
dd MULTIBOOT_HEADER_FLAGS
dd MULTIBOOT_HEADER_CHECKSUM

nop
nop
nop
nop

_start:
  cli

  ; times 0x400000 nop ; makes kernel bigger

  ; save eax=MULTIBOOT_BOOTLOADER_MAGIC and ebx=multiboot_info_t*
  mov edi, eax
  mov esi, ebx

  ; gdt
  lgdt [gdt32_reg]
  ; use new gdt
  jmp selector_kernel_code:_start_kernel

_start_kernel:
  mov ax, selector_kernel_data
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  mov esp, kernel_stack_base + KERNEL_STACK_SIZE ; prepare kernel stack

  ; idt
  call prepare_idt
  lidt [idt_reg]

  ; load tss
  mov [tss_ss0], ss
  mov [tss_esp0], dword isr_stack_base + KERNEL_STACK_SIZE
  call prepare_tss_gdt_entry
  mov ax, selector_tss
  ltr ax

  push esi
  push edi
  call kmain
  add esp, 8

  cli
  hlt

enter_ring3:
  ; avoid reenter
  mov ax, ss
  and ax, SELECTOR_RPL_MASK
  test ax, ax
  jnz _start_user

  mov eax, esp
  push selector_user_data ; ss
  push eax                ; esp
  push selector_user_code ; cs
  push _start_user        ; eip
  retf ; enter ring3
_start_user:
  ret

get_eflags:
  pushf
  pop eax
  ret

; interrupt_wrapper_macro i
%macro interrupt_wrapper_macro 1
  global interrupt_wrapper_%1
  interrupt_wrapper_%1:
  %if !(%1 == 8 || (%1 >= 10 && %1 <= 14) || %1 == 17 || %1 == 30)
    push 0xFFFFFFFF ; dummy errorcode
  %endif
  pushad
  push ds
  push es
  push fs
  push gs
  push esp
  cld
  push %1
  jmp interrupt_wrapper_common
%endmacro

interrupt_wrapper_common:
  call interrupt_handler
  add esp, 4 ; pop %1
  pop esp
  pop gs
  pop fs
  pop es
  pop ds
  popad
  add esp, 4 ; pop errorcode
  iret

; interrupt wrappers
%assign i 0
%rep 256
  interrupt_wrapper_macro i
  %assign i i + 1
%endrep

; protected mode related
section .pm
align 4

gdt32_ptr: descriptor 0, 0, 0 ; none
gdt32_kernel_code: descriptor 0, 0xFFFFF, DESCRIPTOR_ATTR_CODE32 | DESCRIPTOR_ATTR_DPL0
gdt32_kernel_data: descriptor 0, 0xFFFFF, DESCRIPTOR_ATTR_DATA32 | DESCRIPTOR_ATTR_DPL0
gdt32_user_code: descriptor 0, 0xFFFFF, DESCRIPTOR_ATTR_CODE32 | DESCRIPTOR_ATTR_DPL3
gdt32_user_data: descriptor 0, 0xFFFFF, DESCRIPTOR_ATTR_DATA32 | DESCRIPTOR_ATTR_DPL3
gdt32_tss: descriptor 0, 0, 0 ; will be filled later, see pm.c

gdt32_length equ $ - gdt32_ptr
gdt32_reg:
  dw gdt32_length - 1 ; GDT limit
  dd gdt32_ptr ; GDT base

selector_kernel_code equ ((gdt32_kernel_code - gdt32_ptr) | SELECTOR_GDT | SELECTOR_RPL0)
selector_kernel_data equ ((gdt32_kernel_data - gdt32_ptr) | SELECTOR_GDT | SELECTOR_RPL0)
selector_user_code equ ((gdt32_user_code - gdt32_ptr) | SELECTOR_GDT | SELECTOR_RPL3)
selector_user_data equ ((gdt32_user_data - gdt32_ptr) | SELECTOR_GDT | SELECTOR_RPL3)
selector_tss equ ((gdt32_tss - gdt32_ptr) | SELECTOR_GDT | SELECTOR_RPL3)

tss_ptr:
  dd 0
tss_esp0:
  dd 0
tss_ss0:
  dd 0
tss_esp1:
  times 22 dd 0 ; esp1 ~ ldtr
  dw 0
  dw $ - tss_ptr + 2 ; iomap_base
tss_length equ $ - tss_ptr

idt_ptr:
%rep 256
  gate 0, 0, 0 ; will be filled later, see pm.c
%endrep

idt_length equ $ - idt_ptr
idt_reg:
  dw idt_length - 1
  dd idt_ptr

; global constants
section .rodata
align 4

SELECTOR_KERNEL_CODE dw selector_kernel_code
SELECTOR_KERNEL_DATA dw selector_kernel_data
SELECTOR_USER_CODE dw selector_user_code
SELECTOR_USER_DATA dw selector_user_data
SELECTOR_TSS dw selector_tss
TSS_LENGTH dd tss_length

