.intel_syntax noprefix
.global _start
_start:
push rbp
	mov rbp, rsp
	sub rsp, 8
	;//ENDL
	;//FUNC
	jmp L2
	L1:
	push rbp
	mov rbp, rsp
	sub rsp, 8
	mov [rbp -8], rdi
	;//TERN_Q
	;//#2
	mov rax, [rbp -8]
	test rax, rax
	jz L3
	push rbp
	mov rbp, rsp
	sub rsp, 0
	;//MUL
	;//#2
	mov rax, [rbp +8]
	;//CALL
	push rax
	;//SUB
	;//#2
	mov rax, [rbp +8]
	;//1
	mov rdi, 1
	sub rax, rdi
	push rax
	pop rdi
	call L1
	mov rdi, rax
	pop rdi
	imul rax, rdi
	mov rax, rax
	mov rsp, rbp
	pop rbp
	jmp L4
	L3:
	push rbp
	mov rbp, rsp
	sub rsp, 0
	;//1
	mov rax, 1
	mov rax, rax
	mov rsp, rbp
	pop rbp
	L4:
	mov rsp, rbp
	pop rbp
	ret
	L2:
	xor rax, rax
	;//ENDL
	;//VAR
	;//SHL
	;//1
	mov rax, 1
	;//ADD
	;//9
	mov rdi, 9
	;//9
	mov rsi, 9
	add rdi, rsi
	push rcx
	mov rcx, rdi
	shl rax, cl
	pop rcx
	mov [rbp -8], rax
	;//ENDL
	;//WHILE
	push rbp
	mov rbp, rsp
	sub rsp, 0
	L5:
	;//DEC
	mov rax, [rbp +8]
	dec rax
	mov [rbp +8], rax
	test rax, rax
	jz L6
	;//CALL
	;//9
	mov rax, 9
	push rax
	pop rdi
	call L1
	mov rax, rax
	jmp L5
	L6:
	mov rsp, rbp
	pop rbp
	mov rax, rax
	mov rsp, rbp
	pop rbp
	xor rdi, rdi
	mov rax, 0x3c
	syscall
	