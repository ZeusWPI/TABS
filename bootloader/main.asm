; DISK_ID EQU 0x80

KERNEL_START EQU 0x100000
ELF_START EQU 0x8000

NUM_SECTORS EQU 65

org 0x7C00
bits 16

jmp .start

[bits 16]
 
; Function: check_a20
;
; Returns: 0 in ax if the a20 line is disabled (memory wraps around)
;          1 in ax if the a20 line is enabled (memory does not wrap around)
 
check_a20:
    cli
 
    xor ax, ax ; ax = 0
    mov es, ax
 
    not ax ; ax = 0xFFFF
    mov ds, ax
 
    mov di, 0x0500
    mov si, 0x0510
 
    mov al, byte [es:di]
    mov bx, ax
 
    mov al, byte [ds:si]
 
    mov byte [es:di], 0x00
    mov byte [ds:si], 0xFF
 
    cmp byte [es:di], 0xFF
 
    mov byte [ds:si], al
 
    mov ax, bx
    mov byte [es:di], al
 
    mov ax, 0
    je .check_a20__exit
 
    mov ax, 1
 
.check_a20__exit:
    ret

;;; Print string at bx
puts:
.puts_loop:

mov al, [bx]        ; load the character
cmp al, 0           ; check for null byte
je .end_puts_loop

mov ah, 0x0e
int 0x10            ; print charachter
inc bx
jmp .puts_loop
.end_puts_loop:
ret



;;; Print bl as hex
puthex:

; Print high bits
mov cl, bl
shr cl, 4

cmp cl, 9
jg .puthex_high_alpha

; It's < 10, print as digit
mov al, cl
add al, 0x30
mov ah, 0x0e
int 0x10
jmp .puthex_low

.puthex_high_alpha:
; It's >= 10, print as letter
mov al, cl
add al, 0x57 ; 0x57 = (0x61 - 10)
mov ah, 0x0e
int 0x10

.puthex_low:
; Print low bits
mov cl, bl
and cl, 0b1111

cmp cl, 9
jg .puthex_low_alpha

; It's < 10, print as digit
mov al, cl
add al, 0x30
mov ah, 0x0e
int 0x10
jmp .puthex_end

.puthex_low_alpha:
; It's >= 10, print as letter
mov al, cl
add al, 0x57 ; 0x57 = (0x61 - 10)
mov ah, 0x0e
int 0x10

.puthex_end:
ret



.start:

; set up stack
mov ax, 0
mov ss, ax
mov sp, 0x7C00
mov ds, ax
mov es, ax


; disable vga cursor
mov ah, 0x01
mov ch, 0b00100000
mov cl, 0b00000000
int 0x10

; reset disk system
mov ah, 0x00
; mov dl, DISK_ID
int 0x13

; print result on error
jnc .reset_disk_no_error
mov bl, ah
call puthex
.reset_disk_no_error:

mov cx, 2       ; start at sector 2 (skip bootloader)
mov bx, ELF_START  ; write to ELF_START
.read_disk_loop:

; Read sector dl into memory
mov ah, 0x02
mov al, 0x01    ; number of sectors
mov ch, 0x00    ; cylinder number, low 8 bits
; mov cl, 0x01    ; cylinder number, high 2 bits + sector number (6 bits)
mov dh, 0x00    ; head number
; mov dl, DISK_ID    ; drive number
int 0x13

; exit on error
jc .read_disk_error

cmp cx, NUM_SECTORS
jge .read_disk_end ; we have reached the sector limit, time to boot

inc cl      ; next sector
add bx, 512 ; add 512 to data start so we don't overwrite sectors

jmp .read_disk_loop

.read_disk_error:

mov bl, ah
call puthex

.read_disk_end:

call check_a20
cmp ax, 1
je .a20_enabled

mov ah, 0x24     ; set a20 gate
mov al, 0x01     ; turn it on
int 0x15

jnc .a20_enabled

mov bl, ah
call puthex
jmp .end

.a20_enabled:

; disable interrupts
cli

; reset ds
xor ax, ax
mov ds, ax

; load the GDT
lgdt [.gdt_desc]

; switch to protected mode
mov eax, cr0
or eax, 1
mov cr0, eax

; jump to 32 bit code
jmp 0x08:.clear_pipe

[BITS 32]
.clear_pipe:

; set Data Segment and Stack segment
mov ax, 10h
mov ds, ax
mov ss, ax

; set up stack
mov esp, 0x090000

; parse elf file at ELF_START to KERNEL_START

; verify ELF header
mov esi, ELF_START
cmp dword [esi], 464C457Fh ; ELF magic
jne .invalid_elf
cmp word [esi+4], 0101h    ; lsb 32 bit, little endian
jne .invalid_elf
cmp word [esi+18], 03      ; x86 architecture
jne .invalid_elf

; read the entrypoint and store it
mov eax, dword [esi+0x18] ; program entry position
mov dword [.entrypoint], eax

mov cx, word [esi+0x2C]     ; read phnum (number of program headers)
mov eax, dword [esi+0x1C]   ; read phoff (offset of program header)

; ebx is now ELF_START, esi jumps to the start of the program header
mov ebx, esi
add esi, eax

; set up for loop
sub esi, 0x20
inc cx

.elf_ph_loop:
add esi, 0x20
dec cx
jz .invalid_elf             ; there is no valid code block
cmp word [esi], 1           ; check if p_type is loadable
jne .elf_ph_loop

; add offset to ebx     (ebx = pointer to code)
add ebx, dword [esi+0x04]

; store p_filesz        (ecx = size of the segment)
mov ecx, dword [esi+0x10]

; calculate size of bss (edx = size of bss)
mov edx, dword [esi+0x14]
sub edx, ecx

; check if ecx is zero
or ecx, ecx
jz .invalid_elf

; set source
mov esi, ebx

; set destination
mov edi, KERNEL_START

; repeat ecx/4 times (because it moves 4 bytes at a time)
shr ecx, 2

; copy
repnz movsd

; jump to start of kernel
jmp [.entrypoint]

.end:
jmp .end

.invalid_elf:
mov byte [ds:0x0B8000], 'E'
mov byte [ds:0x0B8002], 'L'
mov byte [ds:0x0B8004], 'F'
jmp .end

; padding for bootloader - SFS superblock
times 404 - ($ - $$) db 0

; room for SFS superblock
times 42 db 0

; padding for SFS superblock - data
times 66 - (.file_end - .data) db 0

.data:

.gdt:
dd 0
dd 0

dw 0FFFFh
dw 0
db 0
db 10011010b
db 11001111b
db 0

dw 0FFFFh
dw 0
db 0
db 10010010b
db 11001111b
db 0
.gdt_end:

.gdt_desc:
dw .gdt_end - .gdt - 1
dd .gdt

.entrypoint: dd 0

; magic string
dw 0xAA55

.file_end: