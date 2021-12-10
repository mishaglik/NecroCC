aip END_OF_FILE:
pop bx
jmp MAIN:
LESS:
pop ax
jb TRUE:
jmp FALSE:

LESS_EQ:
pop ax
jbe TRUE:
jmp FALSE:

GRTR: 
pop ax
ja TRUE:
jmp FALSE:

GRTR_EQ:
pop ax
jae TRUE:
jmp FALSE:

EQUAL:
pop ax:
je TRUE:
jmp FALSE:

NON_EQ:
pop ax:
jne TRUE:
jmp FALSE:

FALSE:
push 0
jmp COMP_RET:
TRUE:
push 1
COMP_RET:
push ax
ret
MAIN:	;ENDL
		;FUNC
		jmp FUNC_END_1:
		FUNC_BEG_1:
		pop [bx+4]
			;TERN_Q
				;LESS
					;#2
					push [bx+0]; var 2
					;0
					push 0; 0
				call LESS:
			push 0
			je ELSE_0x603000000430:
				;SUB
					push 0; NULL node
					;1
					push 1; 1
				sub
			jmp END_IF_0x603000000430:
			ELSE_0x603000000430:
				;ENDL
					;VAR
						;0
						push 0; 0
					pop [bx+8]
					push[bx+8]
				pop dx; Empty pop
					;WHILE
					push bx
					push 12
					add
					pop bx
					WHILE_BEG_0x603000000610:
						;LESS
							;MUL
								;#3
								push [bx+-4]; var 3
								;#3
								push [bx+-4]; var 3
							mul
							;#2
							push [bx+-12]; var 2
						call LESS:
					push 0
					je WHILE_END_0x603000000610:
						;SET
							;ADD
								;#3
								push [bx+-4]; var 3
								;1
								push 1; 1
							add
						pop [bx+-4]; SET 3
						push [bx+-4];
					pop [bx+0]
					jmp WHILE_BEG_0x603000000610:
					WHILE_END_0x603000000610:
					push [bx+0]
					push bx
					push 12
					sub
					pop bx
			END_IF_0x603000000430:
		push [bx+4]
		ret
		FUNC_END_1:
		push bx + 1
	pop dx; Empty pop
		;ENDL
			;VAR
				;100
				push 100; 100
			pop [bx+0]
			push[bx+0]
		pop dx; Empty pop
			;ENDL
				;VAR
					;1
					push 1; 1
				pop [bx+4]
				push[bx+4]
			pop dx; Empty pop
				;ENDL
					;VAR
						;1
						push 1; 1
					pop [bx+8]
					push[bx+8]
				pop dx; Empty pop
					;CALL
					push bx
					push 12
					add
					pop bx
						;#4
						push [bx+-12]; var 4
					pop [bx+0]
					call FUNC_BEG_1:
					push bx
					push 12
					sub
					pop bx
out
hlt
END_OF_FILE:
