	.file	"execute.c"
	.text
	.globl	machine_code
	.data
	.type	machine_code, @object
	.size	machine_code, 6
machine_code:
	.byte	-72
	.byte	120
	.byte	120
	.byte	52
	.byte	18
	.byte	-61
	.globl	r
	.align 8
	.type	r, @object
	.size	r, 12
r:
	.long	1065353216
	.long	1073741824
	.long	1077936128
	.section	.rodata
.LC0:
	.string	"mmap"
.LC1:
	.string	"mprotect"
	.text
	.globl	allocate_and_copy
	.type	allocate_and_copy, @function
allocate_and_copy:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movl	%esi, -28(%rbp)
	movl	$0, %r9d
	movl	$-1, %r8d
	movl	$34, %ecx
	movl	$3, %edx
	movl	$6, %esi
	movl	$0, %edi
	call	mmap@PLT
	movq	%rax, -8(%rbp)
	cmpq	$-1, -8(%rbp)
	jne	.L2
	leaq	.LC0(%rip), %rdi
	call	perror@PLT
	movl	$0, %eax
	jmp	.L3
.L2:
	movq	-8(%rbp), %rax
	movl	machine_code(%rip), %edx
	movl	%edx, (%rax)
	movzwl	4+machine_code(%rip), %edx
	movw	%dx, 4(%rax)
	movq	-8(%rbp), %rax
	movl	$5, %edx
	movl	$6, %esi
	movq	%rax, %rdi
	call	mprotect@PLT
	cmpl	$-1, %eax
	jne	.L4
	leaq	.LC1(%rip), %rdi
	call	perror@PLT
	movl	$0, %eax
	jmp	.L3
.L4:
	movq	-8(%rbp), %rax
.L3:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	allocate_and_copy, .-allocate_and_copy
	.globl	deallocate
	.type	deallocate, @function
deallocate:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	movl	-12(%rbp), %edx
	movq	-8(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	munmap@PLT
	cmpl	$-1, %eax
	sete	%al
	movzbl	%al, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	deallocate, .-deallocate
	.globl	f
	.type	f, @function
f:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movss	%xmm0, -4(%rbp)
	movss	%xmm1, -8(%rbp)
	movq	%rdi, -16(%rbp)
	movss	-4(%rbp), %xmm0
	addss	-8(%rbp), %xmm0
	movq	-16(%rbp), %rax
	movss	%xmm0, (%rax)
	addq	$4, -16(%rbp)
	movss	-4(%rbp), %xmm0
	mulss	-8(%rbp), %xmm0
	movq	-16(%rbp), %rax
	movss	%xmm0, (%rax)
	addq	$4, -16(%rbp)
	movl	-4(%rbp), %eax
	movl	%eax, -20(%rbp)
	movss	-20(%rbp), %xmm0
	call	cosf@PLT
	movss	%xmm0, -20(%rbp)
	movl	-8(%rbp), %eax
	movl	%eax, -24(%rbp)
	movss	-24(%rbp), %xmm0
	call	sinf@PLT
	mulss	-20(%rbp), %xmm0
	movq	-16(%rbp), %rax
	movss	%xmm0, (%rax)
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	f, .-f
	.section	.rodata
.LC2:
	.string	"result = 0x%x\n"
.LC5:
	.string	"1: %.2lf\n"
.LC6:
	.string	"2: %.2lf\n"
.LC7:
	.string	"3: %.2lf\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movl	%edi, -36(%rbp)
	movq	%rsi, -48(%rbp)
	movl	$6, %esi
	leaq	machine_code(%rip), %rdi
	call	allocate_and_copy
	movq	%rax, -16(%rbp)
	cmpq	$0, -16(%rbp)
	jne	.L9
	movl	$1, %eax
	jmp	.L10
.L9:
	movq	-16(%rbp), %rax
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rdx
	movl	$0, %eax
	call	*%rdx
	movl	%eax, -20(%rbp)
	movl	-20(%rbp), %eax
	movl	%eax, %esi
	leaq	.LC2(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	leaq	r(%rip), %rdi
	movss	.LC3(%rip), %xmm1
	movss	.LC4(%rip), %xmm0
	call	f
	movss	r(%rip), %xmm0
	cvtss2sd	%xmm0, %xmm0
	leaq	.LC5(%rip), %rdi
	movl	$1, %eax
	call	printf@PLT
	movss	4+r(%rip), %xmm0
	cvtss2sd	%xmm0, %xmm0
	leaq	.LC6(%rip), %rdi
	movl	$1, %eax
	call	printf@PLT
	movss	8+r(%rip), %xmm0
	cvtss2sd	%xmm0, %xmm0
	leaq	.LC7(%rip), %rdi
	movl	$1, %eax
	call	printf@PLT
	movq	-16(%rbp), %rax
	movl	$6, %esi
	movq	%rax, %rdi
	call	deallocate
.L10:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	main, .-main
	.section	.rodata
	.align 4
.LC3:
	.long	1073741824
	.align 4
.LC4:
	.long	1065353216
	.ident	"GCC: (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0"
	.section	.note.GNU-stack,"",@progbits
