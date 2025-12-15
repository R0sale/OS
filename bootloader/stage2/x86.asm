bits 16

section _TEXT class=CODE

global _x86_Video_Clear
_x86_Video_Clear:
    push    bp
    push    bx
    push    cx
    push    dx

    mov     bp, sp

    ; clear the screen
    xor     al, al
    mov     ah, 0x06
    xor     cx, cx
    mov     dx, 0x184F
    mov     bh, 0x07
    int     0x10

    ; move cursor to the start
    xor     bh, bh
    xor     dx, dx
    mov     ah, 0x02

    int     0x10

    mov     sp, bp
    pop     dx
    pop     cx
    pop     bx
    pop     bp
    ret


global _x86_Video_Write
_x86_Video_Write:

    push    bp
    mov     bp, sp
    sub     sp, 4

    ;
    ; [bp + 0] - old stack frame
    ; [bp + 2] - return address
    ; [bp + 4] - first argument
    ; [bp + 6] - last argument
    ;

    push    bx

    mov     al, [bp + 4]
    mov     ah, 0x0E
    mov     bh, [bp + 6]

    int     0x10

    pop     bx
    mov     sp, bp
    pop     bp
    ret