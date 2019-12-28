[bits 32]

mov byte [ds:0B8000h], 0h      ; Move the ASCII-code of 'P' into first video memory
mov byte [ds:0B8001h], 0h      ; Assign a color code

.loop:
inc byte [ds:0B8000h]
inc byte [ds:0B8001h]
jmp .loop