bits 16

section _TEXT class=CODE

global _x86_Move_Cursor
_x86_Move_Cursor:
    push    bp
    mov     bp, sp
    push    ax
    push    bx
    push    dx

    ;
    ; [bp + 4] - first argument
    ; [bp + 6] - second argument
    ;

    mov     ah, 0x03
    mov     bh, 0
    int     0x10    ; dh = row of the cursor, dl = column of the cursor

    add     dl, [bp + 4]
    add     dh, [bp + 6]

    cmp     dl, 79
    jl      .set_cursor

    sub     dl, 80
    inc     dh

.set_cursor:
    mov     bh, 0
    mov     ah, 0x02
    int     0x10

.done:
    pop     dx
    pop     bx
    pop     ax
    mov     sp, bp
    pop     bp
    ret

global _x86_Shutdown
_x86_Shutdown:
    mov     ah, 0x53
    mov     al, 0x07
    mov     bx, 0x0001
    mov     cx, 0x0003
    int     0x15

    ret

global _x86_Video_Read_Char
_x86_Video_Read_Char:
    push    bx
    ; wait for key press
    mov     ah, 0x0
    int     0x16

    xor     ah, ah

.done:
    pop     bx
    ret


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
