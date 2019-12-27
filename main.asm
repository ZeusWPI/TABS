org 0x7C00                      
bits 16                         

jmp .start

;;; Print string at bx
.puts:
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
.puthex:

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
mov dl, 0x81
int 0x13

; print result on error
jnc .reset_disk_no_error
mov bl, ah
call .puthex
.reset_disk_no_error:

; Read sector 1 into memory
mov ah, 0x02
mov al, 0x01    ; number of sectors
mov ch, 0x00    ; cylinder number, low 8 bits
mov cl, 0x01    ; cylinder number, high 2 bits + sector number (6 bits)
mov dh, 0x00    ; head number
mov dl, 0x81    ; drive number
mov bx, .data   ; data buffer
int 0x13

; print result on error
jnc .read_disk_no_error
mov bl, ah
call .puthex
.read_disk_no_error:

; print string
mov bx, .data
call .puts

.end:
hlt

.data:

times 510 - ($ - $$) db 0
dw 0xAA55