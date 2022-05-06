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
pop ax
je TRUE:
jmp FALSE:

NON_EQ:
pop ax
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
				;#2
				push [bx+0]; var 2
			push 0
			je ELSE_0x619000000140:
				;MUL
					;#2
					push [bx+0]; var 2
					;CALL
					push bx
					push 8
					add
					pop bx
						;SUB
							;#2
							push [bx+-8]; var 2
							;1
							push 1; 1
						sub
					pop [bx+0]
					call FUNC_BEG_1:
					push bx
					push 8
					sub
					pop bx
				mul
			jmp END_IF_0x619000000140:
			ELSE_0x619000000140:
				;1
				push 1; 1
			END_IF_0x619000000140:
		push [bx+4]
		ret
		FUNC_END_1:
		push bx + 1
	pop dx; Empty pop
		;ENDL
			;VAR
				;SHL
					;1
					push 1; 1
					;ADD
						;9
						push 9; 9
						;9
						push 9; 9
					add
				shl
			pop [bx+0]
			push[bx+0]
		pop dx; Empty pop
			;ENDL
				;WHILE
				push bx
				push 4
				add
				pop bx
				WHILE_BEG_0x6190000003b0:
					;DEC
					push [bx+-4]
					push 1
					sub
					pop [bx+-4]
					push [bx+-4]
				push 0
				je WHILE_END_0x6190000003b0:
					;CALL
					push bx
					push 4
					add
					pop bx
						;9
						push 9; 9
					pop [bx+0]
					call FUNC_BEG_1:
					push bx
					push 4
					sub
					pop bx
				pop [bx+0]
				jmp WHILE_BEG_0x6190000003b0:
				WHILE_END_0x6190000003b0:
				push [bx+0]
				push bx
				push 4
				sub
				pop bx
out
draw
in
hlt
END_OF_FILE:
