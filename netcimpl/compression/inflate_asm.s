	#
	#
	#
	
	.section .text
	
	.globl inflate_asm_x64
	.type inflate_asm_x64, @function

_inflate_asm_x64_dummy:
	push %rbp
	mov %rsp, %rbp
	
	pop %rbp
	movl $10, %eax
	ret

inflate_asm_x64:
	push %rbp
	mov %rsp, %rbp
	
	pop %rbp
	movl $1010, %eax
	ret


