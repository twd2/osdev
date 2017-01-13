org 0x7c00
cmp dl, 0xe0 ; cdrom
jnz die

xor ax, ax
mov ss, ax
mov sp, 0x7c00

mov cx, 16
loop:
  mov word [dap_ptr + dap_sector_no_low], cx
  xor ax, ax
  mov ds, ax
  lea si, [dap_ptr] ; ds:si = dap_ptr
  mov ah, 0x42 ; extended read sector
  int 0x13 ; read sector
  add cx, 2
  cmp cx, 34
  je die
jmp loop

die:
cli
hlt
jmp die

dap_ptr:
db 16 ; dap size = 16
db 0 ; reversed
dw 2 ; count
dd 0xb8000000 ; dest 0xb800:0000
dd 0 ; sector no low
dd 0 ; sector no high

dap_count equ 2
dap_offset equ 4
dap_seg equ 6
dap_sector_no_low equ 8
dap_sector_no_high equ 12

times 510 - ($ - $$) db 0 
dw 0xaa55
