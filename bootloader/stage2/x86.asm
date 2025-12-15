bits 16

section _TEXT class=CODE

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


; move cursor 1 char right or to the next row (if column is more that 80)
cursor_move:
    push    bp
    push    ax
    push    bx
    push    dx

    mov     bp, sp

    mov     ah, 0x03
    mov     bh, 0
    int     0x10        ; dl - column, dh - row

    ; if dl is more than 80, go to next row
    mov     al, 80
    cmp     dl, al
    ja      .bigger

    inc     dl
    mov     ah, 0x02
    int     0x10
    jmp     .done

.bigger:
    inc     dh
    xor     dl, dl
    mov     ah, 0x02
    int     0x10

.done:
    pop     dx
    pop     bx
    pop     ax
    mov     sp, bp
    pop     bp
    ret