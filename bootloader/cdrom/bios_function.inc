bios_function:
  push ebp
  mov ebp, esp
  push ebx
  push edi
  push esi

  mov ebx, [ebp + 8] ; first argument: function
  mov edi, [ebp + 12] ; second argument
  mov esi, [ebp + 16] ; third argument

  jmp selector_code16:_prepare_go_to_16 - $$

  [bits 16]
  _prepare_go_to_16:
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

  jmp 0x1000:_start16 - $$
  _start16:

  cli

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

  ; do function
  cmp bx, 1
  je function_print

  cmp bx, 2
  je function_getchar

  cmp bx, 3
  je function_read_sector

  cmp bx, 4
  je function_memory_map

  cmp bx, 5
  je function_drive_params

  function_default:
    xor bx, bx
    not bx ; error 0xffff: unknown function
  jmp function_end

  function_print:
    xor bx, bx ; bx = 0
    mov si, di ; high 16 bits of edi are ignored
    print_loop:
      cld
      lodsb
      test al, al
      jz print_return
      mov ah, 0x0e ; print
      int 0x10 ; bios print
      jc bios_error
    jmp print_loop
    print_return:
  jmp function_end

  function_getchar:
    xor bx, bx ; bx = 0
    xor ah, ah ; ah = 0
    int 0x16 ; bios get char
    jc bios_error
    mov [di], ax ; high 16 bits of edi are ignored
  jmp function_end

  function_read_sector:
    xor bx, bx ; bx = 0
    mov dx, di
    mov ah, 0x42 ; extended read sector
    ; dap is at ds:si
    int 0x13 ; bios function
    jc bios_error
  jmp function_end

  memory_map_count dd 0
  function_memory_map:
    xor ebx, ebx ; first record
    mov dword [memory_map_count - $$], ebx ; memory_map_count = 0
    memory_map_loop:
      mov eax, 0xe820 ; read memory info
      mov ecx, [si] ; sizeof(memory_map_t)
      mov edx, 0x534d4150 ; "SMAP"
      ; destination = es:di
      int 0x15 ; bios function
      jc bios_error
      cmp eax, 0x534d4150
      jne bios_error
      inc dword [memory_map_count - $$] ; ++memory_map_count
      test ebx, ebx ; is last record?
      jz memory_map_return
      add di, [si] ; next memory_map_t
    jmp memory_map_loop
    memory_map_return:
    mov eax, dword [memory_map_count - $$]
    mov [si], eax
  jmp function_end

  function_drive_params:
    xor bx, bx ; bx = 0
    mov dx, di
    mov ah, 0x48 ; extended get drive parameters
    ; data struct is at ds:si
    int 0x13 ; bios function
    jc bios_error
  jmp function_end

  bios_error:
  mov bx, ax ; bx = bios error = ax

  function_end:

  ; A20
  in al, 0x92
  or al, 0b00000010
  out 0x92, al

  ; enable protect mode
  mov eax, cr0
  or eax, 1
  mov cr0, eax

  ; return to protect mode
  xor eax, eax
  mov ax, cs
  shl eax, 4
  add eax, _return_to32 - $$
  mov [ds:ljmp_address2 - $$], eax ; logical address cs:_return_to32 to physical address

  ; ljmp dword selector_code:_return_to32
  db 0x66 ; operand-size override
  db 0xea ; ljmp
  ljmp_address2 dd 0xdeadbeef ; will be changed
  dw selector_code

  [bits 32]

  _return_to32:
  cli

  mov ax, selector_data
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  and ebx, 0x0000ffff
  mov eax, ebx ; return bx
  pop esi
  pop edi
  pop ebx
  leave
  ret
