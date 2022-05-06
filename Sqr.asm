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
				;LESS
					;#2
					push [bx+0]; var 2
					;0
					push 0; 0
				call LESS:
			push 0
			je ELSE_0x622000000220:
				;SUB
					push 0; NULL node
					;1
					push 1; 1
				sub
			jmp END_IF_0x622000000220:
			ELSE_0x622000000220:
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
					WHILE_BEG_0x622000000310:
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
					je WHILE_END_0x622000000310:
						;INC
						push [bx+-4]
						push 1
						add
						pop [bx+-4]
						push [bx+-4]
					pop [bx+0]
					jmp WHILE_BEG_0x622000000310:
					WHILE_END_0x622000000310:
					push [bx+0]
					push bx
					push 12
					sub
					pop bx
			END_IF_0x622000000220:
		push [bx+4]
		ret
		FUNC_END_1:
		push bx + 1
	pop dx; Empty pop
		;ENDL
			;FUNC
			jmp FUNC_END_4:
			FUNC_BEG_4:
			pop [bx+8]
				;ENDL
					;VAR
						;100
						push 100; 100
					pop [bx+12]
					push[bx+12]
				pop dx; Empty pop
					;TERN_Q
						;#5
						push [bx+0]; var 5
					push 0
					je ELSE_0x6220000005e0:
						;ENDL
							;OUT
								;DIV
									;SUB
										push 0; NULL node
										;MUL
											;#7
											push [bx+12]; var 7
											;#6
											push [bx+4]; var 6
										mul
									sub
									;#5
									push [bx+0]; var 5
								div
							pop dx
							push dx
							push dx
							out
						pop dx; Empty pop
							;1
							push 1; 1
					jmp END_IF_0x6220000005e0:
					ELSE_0x6220000005e0:
						;TERN_Q
							;#6
							push [bx+4]; var 6
						push 0
						je ELSE_0x6220000007a8:
							;0
							push 0; 0
						jmp END_IF_0x6220000007a8:
						ELSE_0x6220000007a8:
							;SUB
								push 0; NULL node
								;1
								push 1; 1
							sub
						END_IF_0x6220000007a8:
					END_IF_0x6220000005e0:
			push [bx+8]
			ret
			FUNC_END_4:
			push bx + 4
		pop dx; Empty pop
			;ENDL
				;FUNC
				jmp FUNC_END_8:
				FUNC_BEG_8:
				pop [bx+12]
					;ENDL
						;VAR
							;100
							push 100; 100
						pop [bx+16]
						push[bx+16]
					pop dx; Empty pop
						;TERN_Q
							;#5
							push [bx+0]; var 5
						push 0
						je ELSE_0x622000000a60:
							;ENDL
								;VAR
									;SUB
										;MUL
											;#6
											push [bx+4]; var 6
											;#6
											push [bx+4]; var 6
										mul
										;MUL
											;ADD
												;1
												push 1; 1
												;ADD
													;1
													push 1; 1
													;ADD
														;1
														push 1; 1
														;1
														push 1; 1
													add
												add
											add
											;MUL
												;#5
												push [bx+0]; var 5
												;#9
												push [bx+8]; var 9
											mul
										mul
									sub
								pop [bx+20]
								push[bx+20]
							pop dx; Empty pop
								;TERN_Q
									;LESS
										;#10
										push [bx+20]; var 10
										;0
										push 0; 0
									call LESS:
								push 0
								je ELSE_0x622000000d00:
									;0
									push 0; 0
								jmp END_IF_0x622000000d00:
								ELSE_0x622000000d00:
									;ENDL
										;SET
											;CALL
											push bx
											push 24
											add
											pop bx
												;MUL
													;#7
													push [bx+-8]; var 7
													;MUL
														;#7
														push [bx+-8]; var 7
														;#10
														push [bx+-4]; var 10
													mul
												mul
											pop [bx+0]
											call FUNC_BEG_1:
											push bx
											push 24
											sub
											pop bx
										pop [bx+20]; SET 10
										push [bx+20];
									pop dx; Empty pop
										;TERN_Q
											;EQUAL
												;#10
												push [bx+20]; var 10
												;0
												push 0; 0
											call EQUAL:
										push 0
										je ELSE_0x622000000ee0:
											;ENDL
												;OUT
													;MUL
														;#7
														push [bx+16]; var 7
														;DIV
															;SUB
																push 0; NULL node
																;#6
																push [bx+4]; var 6
															sub
															;MUL
																;ADD
																	;1
																	push 1; 1
																	;1
																	push 1; 1
																add
																;#5
																push [bx+0]; var 5
															mul
														div
													mul
												pop dx
												push dx
												push dx
												out
											pop dx; Empty pop
												;1
												push 1; 1
										jmp END_IF_0x622000000ee0:
										ELSE_0x622000000ee0:
											;ENDL
												;OUT
													;DIV
														;ADD
															;MUL
																;#7
																push [bx+16]; var 7
																;SUB
																	push 0; NULL node
																	;#6
																	push [bx+4]; var 6
																sub
															mul
															;#10
															push [bx+20]; var 10
														add
														;MUL
															;ADD
																;1
																push 1; 1
																;1
																push 1; 1
															add
															;#5
															push [bx+0]; var 5
														mul
													div
												pop dx
												push dx
												push dx
												out
											pop dx; Empty pop
												;ENDL
													;OUT
														;DIV
															;SUB
																;MUL
																	;#7
																	push [bx+16]; var 7
																	;SUB
																		push 0; NULL node
																		;#6
																		push [bx+4]; var 6
																	sub
																mul
																;#10
																push [bx+20]; var 10
															sub
															;MUL
																;ADD
																	;1
																	push 1; 1
																	;1
																	push 1; 1
																add
																;#5
																push [bx+0]; var 5
															mul
														div
													pop dx
													push dx
													push dx
													out
												pop dx; Empty pop
													;ADD
														;1
														push 1; 1
														;1
														push 1; 1
													add
										END_IF_0x622000000ee0:
								END_IF_0x622000000d00:
						jmp END_IF_0x622000000a60:
						ELSE_0x622000000a60:
							;CALL
							push bx
							push 24
							add
							pop bx
								;#6
								push [bx+-20]; var 6
							pop [bx+0]
								;#9
								push [bx+-16]; var 9
							pop [bx+4]
							call FUNC_BEG_4:
							push bx
							push 24
							sub
							pop bx
						END_IF_0x622000000a60:
				push [bx+12]
				ret
				FUNC_END_8:
				push bx + 8
			pop dx; Empty pop
				;CALL
				push bx
				push 0
				add
				pop bx
					;IN
					in
				pop [bx+0]
					;IN
					in
				pop [bx+4]
					;IN
					in
				pop [bx+8]
				call FUNC_BEG_8:
				push bx
				push 0
				sub
				pop bx
out
draw
in
hlt
END_OF_FILE:
