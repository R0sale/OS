bits 16
org 0x7c00

start:
    ; set up stack
    mov     ax, 0
    mov     ds, ax
    mov     es, ax

    mov     ss, ax
    mov     sp, 0x7c00

    mov     si, msg_my_message
    call    print

    mov     bx, 0x1000
    call    disk_read

    jmp     0x1000

.jump
    jmp     0x2000


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



;
; Read from disk
;   - es:bx: address where the data goes
;

disk_read:
    ; read kernel (1 sector) into memory
    push    ax
    push    cx
    push    dx

    mov     ah, 02h
    mov     al, 1       ; 1 sector
    mov     ch, 0       ; cylinder number
    mov     cl, 2       ; 1 is boot, second is kernel
    mov     dh, 0       ; head number
    mov     dl, 0       ; disk number

    int     0x13
    jc      .read_failed

    pop     dx
    pop     cx
    pop     ax
    ret


.read_failed:
    mov     si, msg_read_failed
    call    print
    jmp     $



msg_my_message db "Hello world!", 0
msg_read_failed db "Read failed, reboot the system", 0

times 510 - ($ - $$) db 0
dw 0xaa55