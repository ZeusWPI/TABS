DISK_ID EQU 0x81

org 0x7C00
bits 16

jmp .start
 
[bits 16]
 
; Function: check_a20
;
; Purpose: to check the status of the a20 line in a completely self-contained state-preserving way.
;          The function can be modified as necessary by removing push's at the beginning and their
;          respective pop's at the end if complete self-containment is not required.
;
; Returns: 0 in ax if the a20 line is disabled (memory wraps around)
;          1 in ax if the a20 line is enabled (memory does not wrap around)
 
check_a20:
    pushf
    push ds
    push es
    push di
    push si
 
    cli
 
    xor ax, ax ; ax = 0
    mov es, ax
 
    not ax ; ax = 0xFFFF
    mov ds, ax
 
    mov di, 0x0500
    mov si, 0x0510
 
    mov al, byte [es:di]
    push ax
 
    mov al, byte [ds:si]
    push ax
 
    mov byte [es:di], 0x00
    mov byte [ds:si], 0xFF
 
    cmp byte [es:di], 0xFF
 
    pop ax
    mov byte [ds:si], al
 
    pop ax
    mov byte [es:di], al
 
    mov ax, 0
    je check_a20__exit
 
    mov ax, 1
 
check_a20__exit:
    pop si
    pop di
    pop es
    pop ds
    popf
 
    ret

;;; Print string at bx
puts:
mov al, '!'
mov ah, 0x0e
int 0x10
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
mov dl, bl
shr dl, 4

cmp dl, 9
jg .puthex_high_alpha

; It's < 10, print as digit
mov al, dl
add al, 0x30
mov ah, 0x0e
int 0x10
jmp .puthex_low

.puthex_high_alpha:
; It's >= 10, print as letter
mov al, dl
add al, 0x57 ; 0x57 = (0x61 - 10)
mov ah, 0x0e
int 0x10

.puthex_low:
; Print low bits
mov dl, bl
and dl, 0b1111

cmp dl, 9
jg .puthex_low_alpha

; It's < 10, print as digit
mov al, dl
add al, 0x30
mov ah, 0x0e
int 0x10
jmp .puthex_end

.puthex_low_alpha:
; It's >= 10, print as letter
mov al, dl
add al, 0x57 ; 0x57 = (0x61 - 10)
mov ah, 0x0e
int 0x10

.puthex_end:
ret



.start:

; reset disk system
mov ah, 0x00
mov dl, DISK_ID
int 0x13

; print result on error
jnc .reset_disk_no_error
mov bl, ah
call puthex
.reset_disk_no_error:

mov cx, 1       ; start at sector 1
mov bx, 0x8000  ; write to 0x8000
push bx
push cx
.read_disk_loop:

; Read sector dl into memory
mov ah, 0x02
mov al, 0x01    ; number of sectors
mov ch, 0x00    ; cylinder number, low 8 bits
; mov cl, 0x01    ; cylinder number, high 2 bits + sector number (6 bits)
mov dh, 0x00    ; head number
mov dl, DISK_ID    ; drive number
int 0x13

jc .read_disk_error
; print result on error
jnc .read_disk_no_error
mov bl, ah
call puthex
.read_disk_no_error:

pop cx
pop bx
inc cl      ; next sector
add bx, 512 ; add 512 to data start so we don't overwrite sectors
push bx
push cx

jmp .read_disk_loop
.read_disk_error:

call check_a20
cmp ax, 1
je .a20_enabled
mov bx, .str_no_A20
call puts
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

jmp 0x08:0x8000

.end:
jmp .end

.data:
.str_no_A20:
db "A20 not enabled"
db 0


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

; print padding nullbytes
times 510 - ($ - $$) db 0

; write magic string
dw 0xAA55