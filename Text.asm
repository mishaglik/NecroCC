aip END_OF_FILE:
pop bx
	;ENDL
		;FUNC
		jmp FUNC_END_1:
		FUNC_BEG_1:
		pop [bx+4]
			;TERN_Q
				;#2
				push [bx+0]; var 2
			push 0
			je ELSE_0x603000000370:
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
			jmp END_IF_0x603000000370:
			ELSE_0x603000000370:
				;1
				push 1; 1
			END_IF_0x603000000370:
		push [bx+4]
		ret
		FUNC_END_1:
		push bx + 1
	pop dx; Empty pop
		;CALL
		push bx
		push 0
		add
		pop bx
			;ADD
				;MUL
					;ADD
						;1
						push 1; 1
						;1
						push 1; 1
					add
					;ADD
						;1
						push 1; 1
						;1
						push 1; 1
					add
				mul
				;1
				push 1; 1
			add
		pop [bx+0]
		call FUNC_BEG_1:
		push bx
		push 0
		sub
		pop bx
out
hlt
END_OF_FILE:
