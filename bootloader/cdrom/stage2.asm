%include "../../descriptor.inc"

; Assume this code was loaded at 0x1000:0000 = 0x10000.
; (see stage1.asm, "jmp word 0x1000:0000")
; All 16-bit codes are linked manually (" - $$"s).

extern blmain

global _start
global boot_device
global bios_function

; everything's in .text.
[section .text]

[bits 16]
_start:
cli

mov ax, cs
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax

xor ax, ax
mov ss, ax
mov sp, 0xffff

mov [boot_device - $$], dl ; save boot device

; prepare GDTR
xor eax, eax
mov ax, cs
shl eax, 4
add eax, gdt32_ptr - $$
mov [ds:gdt32_reg - $$ + 2], eax ; logical address cs:gdt32_ptr to physical address
lgdt [ds:gdt32_reg - $$]

; A20
in al, 0x92
or al, 0b00000010
out 0x92, al

; enable protect mode
mov eax, cr0
or eax, 1
mov cr0, eax

; go to protect mode
xor eax, eax
mov ax, cs
shl eax, 4
add eax, _start32 - $$
mov [ds:ljmp_address - $$], eax ; logical address cs:_start32 to physical address

; ljmp dword selector_code:_start32
db 0x66 ; operand-size override
db 0xea ; ljmp
ljmp_address dd 0xdeadbeef ; will be changed
dw selector_code

; variables
boot_device db 0

gdt32_ptr: descriptor 0, 0, 0 ; none
gdt32_code: descriptor 0, 0xFFFFF, DESCRIPTOR_ATTR_CODE32 | DESCRIPTOR_ATTR_DPL0
gdt32_data: descriptor 0, 0xFFFFF, DESCRIPTOR_ATTR_DATA32 | DESCRIPTOR_ATTR_DPL0
gdt32_code16: descriptor 0x10000, 0x0FFFF, DESCRIPTOR_ATTR_CODE16 | DESCRIPTOR_ATTR_DPL0
gdt32_data16: descriptor 0, 0x0FFFF, DESCRIPTOR_ATTR_DATA16 | DESCRIPTOR_ATTR_DPL0

gdt32_length equ $ - gdt32_ptr
gdt32_reg:
  dw gdt32_length - 1 ; GDT limit
  dd 0 ; GDT base, will be filled later

selector_code equ ((gdt32_code - gdt32_ptr) | SELECTOR_GDT | SELECTOR_RPL0)
selector_data equ ((gdt32_data - gdt32_ptr) | SELECTOR_GDT | SELECTOR_RPL0)
selector_code16 equ ((gdt32_code16 - gdt32_ptr) | SELECTOR_GDT | SELECTOR_RPL0)
selector_data16 equ ((gdt32_data16 - gdt32_ptr) | SELECTOR_GDT | SELECTOR_RPL0)

[bits 32]
_start32:
mov ax, selector_data
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax

call blmain

die:
cli
hlt
jmp die

%include "bios_function.inc"
