; boot.asm - Bootloader che carica il kernel
; Compila con: nasm -f bin boot.asm -o boot.bin

[BITS 16]
[ORG 0x7C00]

start:
    ; Inizializza i registri
    cli                     ; Disabilita interrupt
    xor ax, ax             ; AX = 0
    mov ds, ax             ; DS = 0
    mov es, ax             ; ES = 0
    mov ss, ax             ; SS = 0
    mov sp, 0x7C00         ; Stack pointer
    sti                    ; Riabilita interrupt

    ; Stampa messaggio di boot
    mov si, boot_msg
    call print_string

    ; Carica il kernel dal disco
    mov ah, 0x02           ; Funzione read sectors
    mov al, 8              ; Numero di settori da leggere (kernel)
    mov ch, 0              ; Cilindro 0
    mov cl, 2              ; Settore 2 (dopo il bootloader)
    mov dh, 0              ; Testa 0
    mov dl, 0x80           ; Drive 0 (primo hard disk)
    mov bx, 0x1000         ; Indirizzo dove caricare il kernel
    mov es, bx
    mov bx, 0
    int 0x13               ; Interrupt BIOS per leggere

    jc disk_error          ; Se carry flag è set, errore

    ; Stampa messaggio di successo
    mov si, kernel_loaded_msg
    call print_string

    ; Salta al kernel
    jmp 0x1000:0x0000

disk_error:
    mov si, disk_error_msg
    call print_string
    hlt

; Funzione per stampare stringhe
print_string:
    pusha
.loop:
    lodsb                  ; Carica byte da SI in AL
    cmp al, 0              ; Controlla se è NULL
    je .done
    mov ah, 0x0E           ; Funzione BIOS teletype
    mov bh, 0              ; Pagina 0
    int 0x10               ; Interrupt video BIOS
    jmp .loop
.done:
    popa
    ret

; Messaggi
boot_msg db 'Aria Bootloader v1.0', 0x0D, 0x0A, 'Caricamento kernel...', 0x0D, 0x0A, 0
kernel_loaded_msg db 'Kernel caricato con successo!', 0x0D, 0x0A, 'Avvio kernel...', 0x0D, 0x0A, 0
disk_error_msg db 'ERRORE: Impossibile caricare il kernel!', 0x0D, 0x0A, 0

; Padding e firma del bootloader
times 510-($-$$) db 0
dw 0xAA55