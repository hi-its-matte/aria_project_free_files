; kernel.asm - Kernel che passa in modalità protetta e carica la UI C
; Compila con: nasm -f bin kernel.asm -o kernel.bin

[BITS 16]
[ORG 0x0000]

kernel_start:
    ; Messaggio di avvio kernel
    mov si, kernel_msg
    call print_string_16

    ; Carica la GDT (Global Descriptor Table)
    lgdt [gdt_descriptor]

    ; Abilita modalità protetta
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Salta in modalità protetta
    jmp CODE_SEG:init_pm

; Funzione per stampare in modalità reale
print_string_16:
    pusha
.loop:
    lodsb
    cmp al, 0
    je .done
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp .loop
.done:
    popa
    ret

; GDT (Global Descriptor Table)
gdt_start:
    ; Descrittore NULL
    dd 0x0
    dd 0x0

    ; Descrittore codice
    dw 0xFFFF       ; Limite (bits 0-15)
    dw 0x0          ; Base (bits 0-15)
    db 0x0          ; Base (bits 16-23)
    db 10011010b    ; Flags
    db 11001111b    ; Flags + limite (bits 16-19)
    db 0x0          ; Base (bits 24-31)

    ; Descrittore dati
    dw 0xFFFF       ; Limite (bits 0-15)
    dw 0x0          ; Base (bits 0-15)
    db 0x0          ; Base (bits 16-23)
    db 10010010b    ; Flags
    db 11001111b    ; Flags + limite (bits 16-19)
    db 0x0          ; Base (bits 24-31)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_start + 0x8
DATA_SEG equ gdt_start + 0x10

[BITS 32]
init_pm:
    ; Inizializza registri segmento in modalità protetta
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Carica e avvia la UI C
    call load_c_ui

    ; Ferma il sistema
    hlt

; Funzione per caricare la UI C
load_c_ui:
    ; Carica il desktop environment dal disco
    mov ah, 0x02           ; Funzione read sectors
    mov al, 16             ; Numero di settori da leggere (desktop)
    mov ch, 0              ; Cilindro 0
    mov cl, 18             ; Settore 18 (dopo 16 settori del kernel)
    mov dh, 0              ; Testa 0
    mov dl, 0x80           ; Drive 0
    mov bx, 0x1000         ; Segmento dove caricare
    mov es, bx
    mov bx, 0x0000         ; Offset
    int 0x13               ; Interrupt BIOS per leggere
    
    jc load_error          ; Se errore, salta
    
    ; Copia il codice all'indirizzo corretto
    mov esi, 0x10000       ; Sorgente
    mov edi, 0x100000      ; Destinazione
    mov ecx, 8192          ; Numero di byte da copiare
    rep movsb              ; Copia
    
    ; Chiama il desktop environment
    call 0x100000
    ret

load_error:
    ; Gestione errore caricamento
    mov edi, 0xB8000
    mov esi, error_msg
    mov ecx, 20
    mov ah, 0x0C           ; Rosso brillante
.loop:
    lodsb
    cmp al, 0
    je .done
    mov [edi], al
    mov [edi+1], ah
    add edi, 2
    jmp .loop
.done:
    hlt

error_msg db 'Errore caricamento UI!', 0

; Messaggi
kernel_msg db 'Kernel avviato! Passaggio a modalità protetta...', 0x0D, 0x0A, 0

; Padding
times 4096-($-$) db 0