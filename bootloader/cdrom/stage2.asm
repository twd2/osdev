%include "descriptor.inc"

org 0x0000
bits 16

cli

mov ax, cs
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
xor ax, ax
mov ss, ax
mov sp, 0xffff

lea si, [hello_str]
call print

; prepare GDTR
xor eax, eax
mov ax, cs
shl eax, 4
add eax, gdt32_ptr
mov [gdt32_reg + 2], eax ; logical address cs:gdt32_ptr to physical address
lgdt [ds:gdt32_reg]

; A20
in al, 0x92
or al, 0b00000010
out 0x92, al

; enable protect mode
mov eax, cr0
or eax, 1
mov cr0, eax

; goto protect mode
xor eax, eax
mov ax, cs
shl eax, 4
add eax, _start32
mov [ds:ljmp_address], eax ; logical address cs:_start32 to physical address

; ljmp selector_code:_start32
db 0x66 ; operand-size override
db 0xea ; ljmp
ljmp_address dd 0xdeadbeef ; will be changed
dw selector_code

print: ; ds:si
lodsb
or al, al
jz print_return
mov ah, 0x0e ; print
mov bh, 0
mov bl, 0
int 0x10 ; bios print
jmp print
print_return:
ret

hello_str db "Stage 1 booted successfully.", 13, 10, 0
return_str db "Returned to 16.", 13, 10, 0

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

bits 32

_start32:
mov ax, selector_data
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax
mov dword [0xb8000], 'P E '

jmp selector_code16:_prepare_return_to16
_prepare_return_to16:

mov ax, selector_data16
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax

; disable protect mode
mov eax, cr0
and al, 0b11111110
mov cr0, eax

bits 16
jmp 0x1000:_start16

bits 16
_start16:

mov ax, cs
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
xor ax, ax
mov ss, ax

; A20
in al, 0x92
and al, 0b11111101
out 0x92, al

mov ax, 0xb800
mov ds, ax
mov dword [ds:160], 'R@E@'

mov ax, cs
mov ds, ax
lea si, [return_str]
call print

die:
cli
hlt
jmp die

