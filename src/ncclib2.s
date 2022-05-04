.global ncc_in
.global ncc_out
.intel_syntax noprefix
.text

;///============\
;//|   Consts   |
;//\============/
BUFFER_SIZE = 128

SYS_WRITE  = 0x01
SYS_STDOUT = 0x01

.global _start
;// _start:
;//     call ncc_in
;//     mov rdi, rax
;//     call ncc_out
;//     xor rdi, rdi
;//     mov rax, 0x3c
;//     syscall

ncc_in:
    
;//     mov rax, 5
;//     ret

    push rbp
    mov rbp, rsp
    sub rsp, 16

    mov QWORD PTR [rbp - 0x08], 0
    mov QWORD PTR [rbp - 0x10], 0
    
    xor rax, rax
    xor rdi, rdi
    lea rsi, [rbp - 0x10]
    mov rdx, 0xf
    syscall

    lea rdi, [rbp - 0x10]
    call atoi
    mov rsp, rbp
    pop rbp
    ret

ncc_out:
    push rbp
    mov rbp, rsp
    sub rsp, 64
    push rdi
    mov rsi, rdi
    lea rdi, [rbp - 64]
    call itoa10
    mov rsi, rax
    lea rdi, [rbp - 64]
    call puts
    pop rax
    mov rsp, rbp
    pop rbp
    ret

;// rdi - string Null ended
atoi:
        xor rax, rax
        mov rdx, 1
        xor rsi, rsi
atoi_loop:
        mov sil, BYTE PTR [rdi]
        inc rdi
        
        test sil, sil
        jz atoi_loop_end

        cmp sil, '\n'
        je atoi_loop_end

        cmp sil, ' '
        je atoi_loop_end

        cmp sil, '-'
        jne atoi_digit
        mov rdx, -1

atoi_digit:
        sub sil, '0'
        imul rax, 10
        add rax, rsi

        jmp atoi_loop
atoi_loop_end:
        imul rax, rdx
        ret

;// =====================================================================
;//  itoa10 - prints integer to string. Require string to include all characters. Unsigned
;//  Entry  - rdi - destination to print
;//           rsi - integer to print
;//  Destroy rax, rdx, rdi, rsi, r8
;//  Return length of string
;// =====================================================================
itoa10:

        mov rax, rsi
        xor r8, r8
        mov rsi, rdi

        push rbx
itoa10_LoopDivStart:                # Div by 10 in loop. 
        mov rbx, 10

        xor rdx, rdx
        div rbx

        lea rbx, [rip + TNXTABLE]

        mov dl, [rbx + rdx]
        mov byte ptr [rdi], dl
        inc rdi

        test rax, rax
        jnz itoa10_LoopDivStart


        add r8, rdi                 # length of string
        sub r8, rsi
        dec rdi
        pop rbx

itoa10_LoopReverseStart:              # Reverse num order
        xor al, al

        mov al, byte ptr [rsi]
        mov ah, byte ptr [rdi]
        mov byte ptr [rdi], al
        mov byte ptr [rsi], ah
        inc rsi
        dec rdi 

        cmp rsi, rdi
        jb itoa10_LoopReverseStart

        mov rax, r8
        ret

;// =====================================================================
;//  itoa2 - for binary operands. Unsigned by default
;//  Entry:  rdi - source to write
;//          rsi - number to write
;//          rdx - shift to do
;//          r8  - mask
;//  Return: rax - strlen
;//  Destroy: rax rdi rsi rdx r8
;// =====================================================================
itoa2:
        push rcx
        push rbx
        mov rcx, rdx

        push rdi
itoa2_LoopDivStart:
    
        mov rax, rsi
        and rax, r8

        lea rbx, [rip + TNXTABLE]
        mov al, byte ptr [rbx + rax]
        stosb

        shr rsi, cl
        jnz itoa2_LoopDivStart

        pop rsi
        mov rax, rdi
        sub rax, rsi
        dec rdi

itoa2_LoopReverseStart:              # Reverse num order
        mov dl, byte ptr [rsi]
        mov dh, byte ptr [rdi]
        mov byte ptr [rdi], dl
        mov byte ptr [rsi], dh
        inc rsi
        dec rdi 

        cmp rsi, rdi
        jb itoa2_LoopReverseStart

    pop rbx
    pop rcx
    ret

;//--------------------------------------------------------------------
;// putc - put char from dil to buffer
;// Entry - dil character to put 
;// Return rax = 0 in case of success <0 otherwise
;// Destroy rax
;//--------------------------------------------------------------------
putc:
        cmp qword ptr [rip + bufferSz], BUFFER_SIZE
        je putc_Flush

putc_FlushBack:
        mov rax, [rip + bufferSz]
        push rbx
        lea rbx, [rip + buffer]
        mov [rbx + rax], dil
        pop rbx
        inc rax
        mov [rip + bufferSz], rax
        xor rax, rax
        ret

putc_Flush:
        push rdi
        push rsi
        push rdx

        call flush
        
        pop rdx
        pop rsi
        pop rdi

        cmp rax, -1
        jne putc_FlushBack
        ret

;// ====================================================================
;// Flush buffer to stdout
;// Return : rax -1 if errored
;// Destroy: rdi, rsi, rdx, rcx, r9, r10 ,r11
;// ====================================================================
flush:
        mov rax, SYS_WRITE      # Call system function write
        mov rdi, SYS_STDOUT
        lea rsi, [rip + buffer]
        mov rdx, [rip + bufferSz]

        syscall

        mov qword ptr [rip + bufferSz], 0       # Reset buffer size        
        ret

;// ====================================================================
;// Puts - put string to stdout
;// Entry: rdi - string, rsi - len
;// Destroy: rdx
;// ====================================================================
puts:
    push rdi
    push rsi

    call flush
    
    pop rdx
    pop rsi
    mov rax, SYS_WRITE
    mov rdi, SYS_STDOUT
    syscall

    mov rax, SYS_WRITE
    mov rdi, SYS_STDOUT
    lea rsi, [rip + endl]
    mov rdx, 1
    syscall
    ret
;// ====================================================================
;// Put n chars c to stdout 
;// Entry:   dil - character
;//          rsi - n
;// Destroy: rdi, rsi, rdx, rax
;// ====================================================================
putNc:
        xor rax, rax
        test rsi, rsi
        jz putNc_Ret

        mov al, dil
        push rcx
        mov rcx, rsi
        lea rdi, [rip + buffer]
        add rdi, [rip + bufferSz]
        add [rip + bufferSz], rcx

putNc_Loop:
        push rbx
        lea rbx, [rip + buffer + BUFFER_SIZE]
        cmp rdi, rbx
        pop rbx
        je putNc_Flush

putNc_FlushEnd:
        stosb
        loop putNc_Loop

        pop rcx
putNc_Ret:
        ret

putNc_Flush:
        push rax
        push rdi

        call flush
        
        pop rdi
        pop rax
        sub qword ptr [rip + bufferSz], BUFFER_SIZE
        jmp putNc_FlushEnd
.data

TNXTABLE:
        .ascii "0123456789ABCDEF" ;// Easter egg for good debug
buffer:
        .fill BUFFER_SIZE
bufferRd:
        .fill BUFFER_SIZE
bufferSz:
        .long 0, 0
endl:
        .ascii "\n"
