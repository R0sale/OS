bits 16

section _TEXT class=CODE

global __U4D
__U4D:

    shl edx, 16         ; dx to upper half of edx
    mov dx, ax          ; edx - dividend
    mov eax, edx        ; eax - dividend
    xor edx, edx

    shl ecx, 16         ; cx to upper half of ecx
    mov cx, bx          ; ecx - divisor

    div ecx             ; eax - quot, edx - remainder
    mov ebx, edx
    mov ecx, edx
    shr ecx, 16

    mov edx, eax
    shr edx, 16

    ret 
;
; U4M
; Operation:      integer four byte multiply
; Inputs:         DX;AX   integer M1
;                 CX;BX   integer M2
; Outputs:        DX;AX   product
; Volatile:       CX, BX destroyed
;
global __U4M
__U4M:
    shl edx, 16         ; dx to upper half of edx
    mov dx, ax          ; m1 in edx
    mov eax, edx        ; m1 in eax

    shl ecx, 16         ; cx to upper half of ecx
    mov cx, bx          ; m2 in ecx

    mul ecx             ; result in edx:eax (we only need eax)
    mov edx, eax        ; move upper half to dx
    shr edx, 16

    ret

global _x86_div64_32
_x86_div64_32:
    ; make new call frame
    push bp             ; save old call frame
    mov bp, sp          ; initialize new call frame

    push bx

    ; divide upper 32 bits
    mov eax, [bp + 8]   ; eax <- upper 32 bits of dividend
    mov ecx, [bp + 12]  ; ecx <- divisor
    xor edx, edx
    div ecx             ; eax - quot, edx - remainder

    ; store upper 32 bits of quotient
    mov bx, [bp + 16]
    mov [bx + 4], eax

    ; divide lower 32 bits
    mov eax, [bp + 4]   ; eax <- lower 32 bits of dividend
                        ; edx <- old remainder
    div ecx

    ; store results
    mov [bx], eax
    mov bx, [bp + 18]
    mov [bx], edx

    pop bx

    ; restore old call frame
    mov sp, bp
    pop bp
    ret

global _x86_Disk_Read
_x86_Disk_Read:
    push    bp
    mov     bp, sp
    push    dx
    push    cx
    push    es
    push    bx

    mov     dl, [bp + 4]

    mov     ax, [bp + 10]
    mov     dh, al

    mov     ax, [bp + 6]
    xor     cl, cl
    or      cl, al

    mov     ax, [bp + 8]
    mov     ch, al
    shl     ah, 6
    or      cl, ah

    mov     ax, [bp + 16]
    mov     es, ax
    mov     bx, [bp + 14]

    mov     al, [bp + 12]
    mov     ah, 0x02

    int     0x13

    jnc     .done

.error:
    xor     ax, ax

.done:
    pop     bx
    pop     es
    pop     cx
    pop     dx
    mov     sp, bp
    pop     bp
    ret

global _x86_Disk_Write
_x86_Disk_Write:
    push   bp
    mov    bp, sp
    push   dx
    push   cx
    push   es
    push   bx

    ; get drive number in dl, heads to dh
    mov    dl, [bp + 4]
    mov    dh, [bp + 10] 

    ; get cylinders and sectors in the right spots
    mov    ax, [bp + 6] 
    xor    cx, cx 
    or     cl, al

    mov    ax, [bp + 8]
    mov    ch, al
    shl    ah, 6 
    or     cl, ah

    ; moving pointer to data into right spot
    mov     ax, [bp + 16]
    mov     es, ax
    mov     bx, [bp + 14]


    ; get count into al, executing write command
    mov    al, [bp + 12] 
    mov    ah, 0x03 
    int    0x13
    mov    ax, 1
    jnc    .done

.error:
    xor    ax, ax
.done:
    pop     bx
    pop     es
    pop     cx
    pop     dx
    pop     bp
    ret


global _x86_Disk_Reset
_x86_Disk_Reset:
    push    bp
    mov     bp, sp
    push    dx

    mov     ah, 0x00
    mov     dl, [bp + 4]
    int     0x13

    jnc      .done

.error:
    xor     ax, ax

.done:
    pop     dx 
    mov     sp, bp
    pop     bp
    ret

global _x86_Disk_Get_Drive_Params
_x86_Disk_Get_Drive_Params:
    push    bp
    mov     bp, sp
    push    dx
    push    bx
    push    si

.start:
    mov     dl, [bp + 4]
    mov     ah, 0x08
    int     0x13

    jc     .error

    mov     si, [bp + 10]   ; number of heads

    xor     ax, ax
    mov     al, dh
    mov     [si], ax

    mov     al, ch          ; now al has lower bits of cylinders
    mov     bl, cl
    shr     bl, 6

    xor     ah, ah
    or      ah, bl          ; now ax is the cylinder number

    mov     si, [bp + 8]
    mov     [si], ax

    and     cl, 0x3F        ; now cl has only sectors

    mov     si, [bp + 6]
    xor     ch, ch
    mov     [si], cx        

    jmp     .done

.error:
    xor     ax, ax

.done:
    pop     si
    pop     bx
    pop     dx
    mov     sp, bp
    pop     bp
    ret
    

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
