%include "descriptor.inc"

global _start
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

extern kmain
extern prepare_tss_gdt_entry
extern prepare_idt
extern interrupt_handler
extern _kernel_load_address
extern _kernel_virtual_base

MULTIBOOT_HEADER_MAGIC equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS equ 0b110 ; mem_*, mmap_*, vesa
MULTIBOOT_HEADER_CHECKSUM equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
MULTIBOOT_HEADER_MODE_TYPE equ 1 ; 0=graphics, 1=text
MULTIBOOT_HEADER_WIDTH equ 800
MULTIBOOT_HEADER_HEIGHT equ 600
MULTIBOOT_HEADER_DEPTH equ 32
KERNEL_STACK_SIZE equ 0x4000 ; 16KiB

section .bss
align 4
kernel_stack_base:
resb KERNEL_STACK_SIZE
isr_stack_base:
resb KERNEL_STACK_SIZE
user_stack_base:
resb KERNEL_STACK_SIZE

section .init32.text progbits alloc exec nowrite
align 4
bits 32

; multiboot headers
dd MULTIBOOT_HEADER_MAGIC
dd MULTIBOOT_HEADER_FLAGS
dd MULTIBOOT_HEADER_CHECKSUM
dd 0
dd 0
dd 0
dd 0
dd 0
dd MULTIBOOT_HEADER_MODE_TYPE
dd MULTIBOOT_HEADER_WIDTH
dd MULTIBOOT_HEADER_HEIGHT
dd MULTIBOOT_HEADER_DEPTH

nop
nop
nop
nop

; at 0x1xxxxx
_start:
  cli

  ; times 0x400000 nop ; makes kernel bigger

  ; save eax=MULTIBOOT_BOOTLOADER_MAGIC and ebx=multiboot_info_t*
  mov edi, eax
  mov esi, ebx

  ; init paging
  lea eax, [pd_ptr]
  lea ebx, [_kernel_virtual_base]
  sub eax, ebx
  lea ecx, [pt0_ptr]
  sub ecx, ebx
  or dword [eax], ecx
  or dword [eax + 768 * 4], ecx
  lea ecx, [pt1_ptr]
  sub ecx, ebx
  or dword [eax + 1 * 4], ecx
  or dword [eax + (768 + 1) * 4], ecx
  mov cr3, eax ; cr3 = pd_ptr (this is virtual address) - _kernel_virtual_base

  ; enable paging
  mov eax, cr0
  or eax, 0x80000000
  mov cr0, eax

  jmp _start_va

section .text
align 4
bits 32

; at 0xc01xxxxx
_start_va:
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
  ; prepare kernel stack
  mov ss, ax
  mov esp, kernel_stack_base + KERNEL_STACK_SIZE

  ; remove 0 ~ 8M-1 -> 0 ~ 8M-1
  lea eax, [pd_ptr]
  xor ebx, ebx
  mov [eax], ebx
  mov [eax + 1 * 4], ebx
  lea ebx, [_kernel_virtual_base]
  sub eax, ebx
  mov cr3, eax
  ; mov [0], eax test
  ; idt
  call prepare_idt
  lidt [idt_reg]

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
section .pm progbits alloc noexec write
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

idt_ptr:
%rep 256
  gate 0, 0, 0 ; will be filled later, see pm.c
%endrep

idt_length equ $ - idt_ptr
idt_reg:
  dw idt_length - 1
  dd idt_ptr

section .pd progbits alloc noexec write
align 4

%include "paging.inc"

pd_ptr: ; page directory
  ; 0 ~ 8M-1 -> 0 ~ 8M-1
  dd 0 | PAGING_PXE_P | PAGING_PXE_RW
  dd 0 | PAGING_PXE_P | PAGING_PXE_RW
  times 766 dd 0
  ; C0000000 ~ C0000000 + 8M-1 -> 0 ~ 8M-1
  dd 0 | PAGING_PXE_P | PAGING_PXE_RW | PAGING_PXE_US ; TODO
  dd 0 | PAGING_PXE_P | PAGING_PXE_RW | PAGING_PXE_US
  times 254 dd 0

%assign i 0
pt0_ptr: ; page table 0
%rep 1024
  dd (i << 12) | PAGING_PXE_P | PAGING_PXE_RW | PAGING_PXE_US ; TODO
  %assign i i + 1
%endrep
pt1_ptr: ; page table 1
%rep 1024
  dd (i << 12) | PAGING_PXE_P | PAGING_PXE_RW | PAGING_PXE_US ; TODO
  %assign i i + 1
%endrep

; global constants
section .rodata
align 4

SELECTOR_KERNEL_CODE dw selector_kernel_code
SELECTOR_KERNEL_DATA dw selector_kernel_data
SELECTOR_USER_CODE dw selector_user_code
SELECTOR_USER_DATA dw selector_user_data
SELECTOR_TSS dw selector_tss

