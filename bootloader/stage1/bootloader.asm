bits 16
org 0x7c00

%define ENDL 0x0D, 0x0A


;
; FAT16 header
; 
jmp short start
nop

bdb_oem:                    db 'MSWIN4.1'           ; 8 bytes
bdb_bytes_per_sector:       dw 512
bdb_sectors_per_cluster:    db 4                    ; 4 sectors (2KB) per cluster. 
                                                    ; (Standard for FAT16 drives < 128MB)
bdb_reserved_sectors:       dw 1
bdb_fat_count:              db 2
bdb_dir_entries_count:      dw 512                  ; 0x0200. Standard for FAT16 Root Directory
bdb_total_sectors:          dw 0                    ; Set to 0. FAT16 usually uses bdb_large_sector_count
bdb_media_descriptor_type:  db 0F8h                 ; F8 = Fixed Disk / Hard Drive
bdb_sectors_per_fat:        dw 64                   ; Depends on disk size! 
                                                    ; 64 sectors * 512 = 32KB FAT. 
                                                    ; Enough to map ~16,000 clusters (approx 32MB disk)
bdb_sectors_per_track:      dw 32                   ; Geometry varies by HDD image type
bdb_heads:                  dw 64                   ; Geometry varies by HDD image type
bdb_hidden_sectors:         dd 0
bdb_large_sector_count:     dd 65536                ; 0x10000. Total sectors (32MB disk size)

; extended boot record
ebr_drive_number:           db 80h                  ; 0x80 = Hard Drive (C:), 0x00 = Floppy
                            db 0                    ; reserved
ebr_signature:              db 29h
ebr_volume_id:              db 12h, 34h, 56h, 78h   ; serial number
ebr_volume_label:           db 'MY       OS'        ; 11 bytes, padded with spaces
ebr_system_id:              db 'FAT16   '           ; 8 bytes (Must match exactly)

start:
    mov     ax, 0
    mov     ds, ax
    mov     es, ax

    mov     ss, ax
    mov     sp, 0x7c00

    mov     [ebr_drive_number], dl

    mov     si, msg_loading

    call    puts

    ; read drive parameters from bios
    push    es
    mov     ah, 0x08
    int     0x13
    
    pop     es

    and     cl, 0x3F    ; remove top 2 bits
    xor     ch, ch

    mov     [bdb_sectors_per_track], cx

    inc     dh
    mov     [bdb_heads], dh

    ; compute lba of the root Directory
    mov     ax, [bdb_sectors_per_fat]
    mov     cl, [bdb_fat_count]
    xor     ch, ch
    mul     cl

    add     ax, [bdb_reserved_sectors]

    push    ax  ; now in stack we have the lba of the root dir

    ; compute size of the root Directory
    mov     ax, [bdb_dir_entries_count]
    shl     ax, 5   ; *32
    xor     dx, dx
    div     word [bdb_bytes_per_sector]

    cmp     dx, 0
    je      .root_dir_after

    inc     ax

.root_dir_after:
    ; read root Directory
    push    ax      ; push the size of the root dir in sectors
    mov     cl, al

    pop     dx      ; dx is size of root dir in sectors
    pop     ax      ; ax is lba of the root dir

    push    ax

    add     ax, dx      ; now ax is the start of the data KERNEL_LOAD_SEGMENT
    mov     [data_start], ax
    pop     ax


    mov     dl, [ebr_drive_number]
    mov     bx, buffer

    call    disk_read

    xor     bx, bx
    mov     di, buffer

.search_stage2:
    mov     si, file_stage2_bin
    mov     cx, 11
    push    di

    repe    cmpsb

    pop     di

    je      .found_stage2

    ; going via all the directory entries
    add     di, 32
    inc     bx
    cmp     bx, [bdb_dir_entries_count]
    jl      .search_stage2
    
    jmp     stage2_not_found 

.found_stage2:

    mov     ax, [di + 26]   ; first logical cluster field (offset 26)
    mov     [stage2_cluster], ax

    ; Load FAT from disk into memory
    mov     ax, [bdb_reserved_sectors]
    mov     bx, buffer
    mov     cl, [bdb_sectors_per_fat]
    mov     dl, [ebr_drive_number]
    call    disk_read

    mov     bx, STAGE2_LOAD_SEGMENT
    mov     es, bx
    mov     bx, STAGE2_LOAD_OFFSET

.load_stage2_loop:
    ; Read next cluster
    mov     ax, [stage2_cluster]
    sub     ax, 2
    xor     cx, cx
    mov     cl, [bdb_sectors_per_cluster]
    mul     cx
    add     ax, [data_start]            ; now ax is the target sector of the kernel_cluster

    mov     cl, [bdb_sectors_per_cluster]
    mov     dl, [ebr_drive_number]
    call    disk_read

    xor     ax, ax
    mov     al, [bdb_sectors_per_cluster]
    mov     cx, 512
    mul     cx                      ; AX = Bytes per cluster
    add     bx, ax                  ; Advance ES:BX

    mov     ax, [stage2_cluster]
    shl     ax, 1
    mov     si, buffer
    add     si, ax
    mov     ax, [si]
    mov     [stage2_cluster], ax

    cmp     ax, 0xFFF8
    jl      .load_stage2_loop

    mov dl, [ebr_drive_number]  ; Pass boot drive to stage2
    
    mov ax, STAGE2_LOAD_SEGMENT
    mov ds, ax
    mov es, ax
    
    jmp STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET


;
; Error handlers
;

disk_error:
    mov     si, msg_read_failed
    call    puts
    jmp     wait_key_and_reboot

stage2_not_found:
    mov     si, msg_stage2_not_found
    call    puts
    jmp     wait_key_and_reboot


wait_key_and_reboot:
    mov     ah, 0
    int     0x16    ; wait for key press
    jmp     0xFFFF:0x0000   ; start of the BIOS




;
; ax - lba address
; cx bits[0-5] - sector number
; cx bits[6-15] - cylinder number
; dh - heads number
;
lba_to_chs:
    push    ax
    push    dx
    
    xor     dx, dx
    div     word [bdb_sectors_per_track]
    inc     dx

    mov     cx, dx

    xor     dx, dx
    div     word [bdb_heads]

    mov     ch, al
    shl     ah, 6
    or      cl, ah
    shl     dx, 8

    pop     ax
    mov     dl, al
    pop     ax
    ret


;
; Reads sectors from a disk
; Parameters:
;   - ax: LBA address
;   - cl: number of sectors to read (up to 128)
;   - dl: drive number
;   - es:bx: memory address where to store read data
;
disk_read:

    push ax                             ; save registers we will modify
    push bx
    push cx
    push dx
    push di

    push cx                             ; temporarily save CL (number of sectors to read)
    call lba_to_chs                     ; compute CHS
    pop  ax                             ; AL = number of sectors to read
    
    mov ah, 02h
    mov di, 3                           ; retry count

.retry:
    pusha                               ; save all registers, we don't know what bios modifies
    stc                                 ; set carry flag, some BIOS'es don't set it
    int 13h                             ; carry flag cleared = success
    jnc .done                           ; jump if carry not set

    ; read failed
    popa
    call disk_reset

    dec di
    test di, di
    jnz .retry

.fail:
    ; all attempts are exhausted
    jmp disk_error

.done:
    popa

    pop di
    pop dx
    pop cx
    pop bx
    pop ax                             ; restore registers modified
    ret


;
; Resets disk controller
; Parameters:
;   dl: drive number
;
disk_reset:
    pusha
    mov ah, 0
    stc
    int 13h
    jc disk_error
    popa
    ret

puts:
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


msg_loading db "Loading...", ENDL, 0
read_msg db "read completed", 0
file_stage2_bin db "STAGE2  BIN"
STAGE2_LOAD_SEGMENT equ 0x2000
STAGE2_LOAD_OFFSET equ 0
stage2_cluster dw 0
data_start dw 0
msg_stage2_not_found db "There is no stage2!!!", ENDL, 0
msg_read_failed db "Read failed.", ENDL, 0

times 510 - ($ - $$) db 0
dw 0xaa55

buffer: