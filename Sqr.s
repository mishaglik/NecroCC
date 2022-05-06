.intel_syntax noprefix
.global _start
_start:
	push rbp
	mov rbp, rsp
	sub rsp, 0
	;//ENDL
	;//FUNC
	jmp L2
	L1:
	push rbp
	mov rbp, rsp
	sub rsp, 8
	mov [rbp -8], rdi
	;//TERN_Q
	;//LESS
	;//#2
	push [rbp - 0x8]
	;//0
	mov rax, 0
	push rax
	pop rbx
	pop rax
	xor rcx, rcx
	cmp rax, rbx
	setl cl
	push rcx
	pop rax
	test rax, rax
	jz L3
	push rbp
	mov rbp, rsp
	sub rsp, 0
	;//SUB
	push rax
	;//1
	mov rax, 1
	push rax
	pop rcx
	pop rax
	sub rax, rcx
	push rax
	pop rax
	mov rsp, rbp
	pop rbp
	jmp L4
	L3:
	push rbp
	mov rbp, rsp
	sub rsp, 0
	;//ENDL
	;//VAR
	;//0
	mov rax, 0
	push rax
	pop rax
	mov [rbp -8], rax
	push rax
	pop rax
	;//WHILE
	sub rbp, 8
	L5:
	;//LESS
	;//MUL
	;//#3
	push [rbp + 0x8]
	;//#3
	push [rbp + 0x8]
	pop rcx
	pop rax
	imul rax, rcx
	push rax
	;//#2
	push [rbp + 0x18]
	pop rbx
	pop rax
	xor rcx, rcx
	cmp rax, rbx
	setl cl
	push rcx
	pop rax
	test rax, rax
	jz L6
	;//INC
	mov rax, [rbp +8]
	inc rax
	mov [rbp +8], rax
	push rax
	pop [rbp - 0x8]
	jmp L5
	L6:
	push [rbp - 0x8]
	add rbp, 8
	pop rax
	mov rsp, rbp
	pop rbp
	L4:
	push rax
	pop rax
	mov rsp, rbp
	pop rbp
	ret
	L2:
	xor rax, rax
	push rax
	pop rax
	;//ENDL
	;//FUNC
	jmp L8
	L7:
	push rbp
	mov rbp, rsp
	sub rsp, 16
	mov [rbp -8], rdi
	mov [rbp -16], rsi
	;//ENDL
	;//VAR
	;//100
	mov rax, 100
	push rax
	pop rax
	mov [rbp -24], rax
	push rax
	pop rax
	;//TERN_Q
	;//#5
	push [rbp - 0x8]
	pop rax
	test rax, rax
	jz L9
	push rbp
	mov rbp, rsp
	sub rsp, 0
	;//ENDL
	;//OUT
	;//DIV
	;//SUB
	push rax
	;//MUL
	;//#7
	push [rbp + 0x8]
	;//#6
	push [rbp + 0x10]
	pop rcx
	pop rax
	imul rax, rcx
	push rax
	pop rcx
	pop rax
	sub rax, rcx
	push rax
	;//#5
	push [rbp + 0x18]
	pop rax
	cqo
	pop rbx
	idiv rbx
	push rax
	pop rdi
	call ncc_out
	push rax
	pop rax
	;//1
	mov rax, 1
	push rax
	pop rax
	mov rsp, rbp
	pop rbp
	jmp L10
	L9:
	push rbp
	mov rbp, rsp
	sub rsp, 0
	;//TERN_Q
	;//#6
	push [rbp + 0x10]
	pop rax
	test rax, rax
	jz L11
	push rbp
	mov rbp, rsp
	sub rsp, 0
	;//0
	mov rax, 0
	push rax
	pop rax
	mov rsp, rbp
	pop rbp
	jmp L12
	L11:
	push rbp
	mov rbp, rsp
	sub rsp, 0
	;//SUB
	push rax
	;//1
	mov rax, 1
	push rax
	pop rcx
	pop rax
	sub rax, rcx
	push rax
	pop rax
	mov rsp, rbp
	pop rbp
	L12:
	push rax
	pop rax
	mov rsp, rbp
	pop rbp
	L10:
	push rax
	pop rax
	mov rsp, rbp
	pop rbp
	ret
	L8:
	xor rax, rax
	push rax
	pop rax
	;//ENDL
	;//FUNC
	jmp L14
	L13:
	push rbp
	mov rbp, rsp
	sub rsp, 24
	mov [rbp -8], rdi
	mov [rbp -16], rsi
	mov [rbp -24], rdx
	;//ENDL
	;//VAR
	;//100
	mov rax, 100
	push rax
	pop rax
	mov [rbp -32], rax
	push rax
	pop rax
	;//TERN_Q
	;//#5
	push [rbp - 0x8]
	pop rax
	test rax, rax
	jz L15
	push rbp
	mov rbp, rsp
	sub rsp, 0
	;//ENDL
	;//VAR
	;//SUB
	;//MUL
	;//#6
	push [rbp + 0x18]
	;//#6
	push [rbp + 0x18]
	pop rcx
	pop rax
	imul rax, rcx
	push rax
	;//MUL
	;//ADD
	;//1
	mov rax, 1
	push rax
	;//ADD
	;//1
	mov rax, 1
	push rax
	;//ADD
	;//1
	mov rax, 1
	push rax
	;//1
	mov rax, 1
	push rax
	pop rcx
	pop rax
	add rax, rcx
	push rax
	pop rcx
	pop rax
	add rax, rcx
	push rax
	pop rcx
	pop rax
	add rax, rcx
	push rax
	;//MUL
	;//#5
	push [rbp + 0x20]
	;//#9
	push [rbp + 0x10]
	pop rcx
	pop rax
	imul rax, rcx
	push rax
	pop rcx
	pop rax
	imul rax, rcx
	push rax
	pop rcx
	pop rax
	sub rax, rcx
	push rax
	pop rax
	mov [rbp -8], rax
	push rax
	pop rax
	;//TERN_Q
	;//LESS
	;//#10
	push [rbp - 0x8]
	;//0
	mov rax, 0
	push rax
	pop rbx
	pop rax
	xor rcx, rcx
	cmp rax, rbx
	setl cl
	push rcx
	pop rax
	test rax, rax
	jz L17
	push rbp
	mov rbp, rsp
	sub rsp, 0
	;//0
	mov rax, 0
	push rax
	pop rax
	mov rsp, rbp
	pop rbp
	jmp L18
	L17:
	push rbp
	mov rbp, rsp
	sub rsp, 0
	;//ENDL
	;//SET
	;//CALL
	;//MUL
	;//#7
	push [rbp - 0x8]
	;//MUL
	;//#7
	push [rbp - 0x8]
	;//#10
	push [rbp - 0x18]
	pop rcx
	pop rax
	imul rax, rcx
	push rax
	pop rcx
	pop rax
	imul rax, rcx
	push rax
	pop rdi
	call L1
	push rax
	pop [rbp - 0x18]
	push [rbp - 0x18]
	pop rax
	;//TERN_Q
	;//EQUAL
	;//#10
	push [rbp - 0x18]
	;//0
	mov rax, 0
	push rax
	pop rbx
	pop rax
	xor rcx, rcx
	cmp rax, rbx
	sete cl
	push rcx
	pop rax
	test rax, rax
	jz L19
	push rbp
	mov rbp, rsp
	sub rsp, 0
	;//ENDL
	;//OUT
	;//MUL
	;//#7
	push [rbp - 0x18]
	;//DIV
	;//SUB
	push rax
	;//#6
	push [rbp - 0x8]
	pop rcx
	pop rax
	sub rax, rcx
	push rax
	;//MUL
	;//ADD
	;//1
	mov rax, 1
	push rax
	;//1
	mov rax, 1
	push rax
	pop rcx
	pop rax
	add rax, rcx
	push rax
	;//#5
	push [rbp + 0]
	pop rcx
	pop rax
	imul rax, rcx
	push rax
	pop rax
	cqo
	pop rbx
	idiv rbx
	push rax
	pop rcx
	pop rax
	imul rax, rcx
	push rax
	pop rdi
	call ncc_out
	push rax
	pop rax
	;//1
	mov rax, 1
	push rax
	pop rax
	mov rsp, rbp
	pop rbp
	jmp L20
	L19:
	push rbp
	mov rbp, rsp
	sub rsp, 0
	;//ENDL
	;//OUT
	;//DIV
	;//ADD
	;//MUL
	;//#7
	push [rbp - 0x18]
	;//SUB
	push rax
	;//#6
	push [rbp - 0x8]
	pop rcx
	pop rax
	sub rax, rcx
	push rax
	pop rcx
	pop rax
	imul rax, rcx
	push rax
	;//#10
	push [rbp - 0x28]
	pop rcx
	pop rax
	add rax, rcx
	push rax
	;//MUL
	;//ADD
	;//1
	mov rax, 1
	push rax
	;//1
	mov rax, 1
	push rax
	pop rcx
	pop rax
	add rax, rcx
	push rax
	;//#5
	push [rbp + 0]
	pop rcx
	pop rax
	imul rax, rcx
	push rax
	pop rax
	cqo
	pop rbx
	idiv rbx
	push rax
	pop rdi
	call ncc_out
	push rax
	pop rax
	;//ENDL
	;//OUT
	;//DIV
	;//SUB
	;//MUL
	;//#7
	push [rbp - 0x18]
	;//SUB
	push rax
	;//#6
	push [rbp - 0x8]
	pop rcx
	pop rax
	sub rax, rcx
	push rax
	pop rcx
	pop rax
	imul rax, rcx
	push rax
	;//#10
	push [rbp - 0x28]
	pop rcx
	pop rax
	sub rax, rcx
	push rax
	;//MUL
	;//ADD
	;//1
	mov rax, 1
	push rax
	;//1
	mov rax, 1
	push rax
	pop rcx
	pop rax
	add rax, rcx
	push rax
	;//#5
	push [rbp + 0]
	pop rcx
	pop rax
	imul rax, rcx
	push rax
	pop rax
	cqo
	pop rbx
	idiv rbx
	push rax
	pop rdi
	call ncc_out
	push rax
	pop rax
	;//ADD
	;//1
	mov rax, 1
	push rax
	;//1
	mov rax, 1
	push rax
	pop rcx
	pop rax
	add rax, rcx
	push rax
	pop rax
	mov rsp, rbp
	pop rbp
	L20:
	push rax
	pop rax
	mov rsp, rbp
	pop rbp
	L18:
	push rax
	pop rax
	mov rsp, rbp
	pop rbp
	jmp L16
	L15:
	push rbp
	mov rbp, rsp
	sub rsp, 0
	;//CALL
	;//#6
	push [rbp + 0x18]
	;//#9
	push [rbp + 0x10]
	pop rdi
	pop rsi
	call L7
	push rax
	pop rax
	mov rsp, rbp
	pop rbp
	L16:
	push rax
	pop rax
	mov rsp, rbp
	pop rbp
	ret
	L14:
	xor rax, rax
	push rax
	pop rax
	;//CALL
	;//IN
	call ncc_in
	push rax
	;//IN
	call ncc_in
	push rax
	;//IN
	call ncc_in
	push rax
	pop rdi
	pop rsi
	pop rdx
	call L13
	push rax
	mov rsp, rbp
	pop rbp
	xor rdi, rdi
	mov rax, 0x3c
	syscall
	