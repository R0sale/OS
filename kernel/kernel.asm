bits 16
org 0x0


start:
    ; set up stack
    mov     ax, cs
    mov     ds, ax
    mov     es, ax

    mov     ss, ax
    mov     sp, 0

    mov     si, msg_my_message
    call    print

.jump
    jmp     $




;
; Prints the string from si
; Parameters:
;   - si: adress of the string to print
;
print:
    push    bx
    push    si
.loop:
    lodsb
    or      al, al
    je      .done

    mov     ah, 0x0E
    mov     bh, 0
    int     0x10

    jmp     .loop

.done:
    pop     si
    pop     bx
    ret


msg_my_message db "Hello from kernel!", 0

times 510 - ($ - $$) db 0
dw 0xaa55