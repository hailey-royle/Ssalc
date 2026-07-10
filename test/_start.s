section .text

extern start

global _start
_start:
	mov rdi, [rsp] ; set argument.data
	shl rdi, 4
	neg rdi
	add rdi, rsp
	pop rsi        ; set argument.count
	mov r8, rsi    ; loop through arguments, moving data and count to the stack
	               ; r8 is the _start_argument iterator
	mov r9, rdi    ; r9 is the stack write location
_start_argument_loop:
	cmp r8, 0
	jz _start_call_ssal
	dec r8
	pop r10        ; r10 is the pointer to the nth argument's data, push it to the stack
	mov [r9], r10
	add r9, 8
	xor r11, r11   ; loop though the string to find the length
	               ; r11 is the string iterator/length
_start_argument_count:
	cmp byte [r10], 0
	jz _start_argument_count_found_end
	inc r11
	inc r10
	jmp _start_argument_count
_start_argument_count_found_end:
	mov [r9], r11
	add r9, 8
	jmp _start_argument_loop
_start_call_ssal:      ; final prep for ssal entry
	mov rsp, rdi
	call start     ; fallthrough to exit

global exit_syscall
exit_syscall:
	mov rdi, rax
	mov rax, 60
	syscall

global write_syscall
write_syscall:
	mov rax, 1
	syscall
	ret

