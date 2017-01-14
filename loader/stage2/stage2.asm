org 0x0000
bits 16

nop
nop
nop
nop

mov ax, cs
mov ds, ax
mov es, ax
xor ax, ax
mov ss, ax
mov sp, 0x7c00

lea si, [hello_str]
call print

mov eax, 0xdeadbeef
_start:
times 0x7000 inc eax
jmp _start

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
