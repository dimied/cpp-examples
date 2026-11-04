	.intel_syntax
	
	.section .text
	
	.globl inflate_asm_x64_clear
	.type inflate_asm_x64_clear, @function
	#
	.globl inflate_test1
	.type inflate_test1, @function
	
# 1st = %rdi : address        ; 2nd = %esi : count
inflate_asm_x64_clear:
	pushq %rcx
	mov %ecx, %esi
	xorq %rax, %rax
	std
	repnz stosq
	popq %rcx
	ret
	
	
# rdi = ptr to test array
# rsi = bits
# rdx = count
inflate_test1:
	movzx %rcx, %dx
	mov %rbx, %rsi
	mov %rdx, 8
	mov %rsi, %rdi
	#
	xor %rax, %rax
	#
	test %rcx, %rcx
	jz .inflate_test1_xyz
	#
.inflate_test1_loop:
	# test %dl, %dl
	# jz .inflate_test1_error
	#
	mov %al, %bl
	#
	lea %rdi, [%rsi + %rdx]
	and %al, 0x7
	#
	stosb
	#
	shr %rbx, 3
	#
	test %rbx, %rbx # any other bits set?
	jz .inflate_test1_xyz
	#
	xor %dl, 0xF
	bt %dx, 3
	adc %dl, 0
	#
	loopnz .inflate_test1_loop
.inflate_test1_loop_end:
	jmp .inflate_test1_xyz
.inflate_test1_error:
	mov %eax, - 1
.inflate_test1_xyz:
	ret
	
supports_64_bit:
	mov %eax, 0x80000001
	cpuid
	mov %eax, %edx
	bt %eax, 29
	jc .supports_64_bit_end
	xor %eax, %eax
	ret
.supports_64_bit_end:
	mov %eax, 1
	ret
	
	
.block_data_compute_values2:
	.if 0
	mov %cl, 8
	mov %bx, %ax
	add %bl, 3
	#
	cmp %al, %cl
	ja .sym_greater8
	xor %ax, %ax # no length bits
	jmp .sym_check
.sym_greater8:
	cmp %al, 28
	jne .sym_not28
	xor %ax, %ax
	mov %bx, 258
	jmp .sym_check
.sym_not28:
	#
	sub %al, 8 # sym minus 8
	cmp %al, 16
	jae .ae16
	# if (sym < 16)
	cmp %al, 8
	jae .ae8
	# if (sym < 8)
	# {
	# v = INFLATE_CHARS_AS_UINT64(1, 2, 3, 4ul, (7ul), (10ul), (13ul), (16ul));
	mov %rdx, (1| (2<<8)|(3<<16)|(4<<24)|(7<<32)|(10<<40)|(13<<48)|(16<<56))
	# }
	jmp .comp
.ae8:
	# else
	# {
	# / * 20, 21, 22, 23, 24, 25, 26, 27 * / 
	# v = INFLATE_CHARS_AS_UINT64(23, 30, 37ul, 44, (59ul), (74ul), (89ul), (104ul));
	mov %rdx, (23| (30<<8)| (37<<16)|(44<<24)|(59<<32)|(74<<40)|(89<<48)|(104<<56))
	# }
	
	jmp .comp
.ae16:
	# / * 28, 29, 30, 31 * / 
	# uint64_t v = INFLATE_CHARS_AS_UINT64(135, 166, 197, 227ul, 0ul, 0ul, 0ul, 0ul);
	mov %rdx, (135| (166<<8)| (197<<16)| (227<<24))
.comp:
	#
	# unsigned int s = (sym & 0x7) << 3;
	# length + = (v >> s)        ;
	movzx %ecx, %ax
	and %cx, 0x7
	# sym >>= 2
	shr %ax, 2 # we have lengths in 4 - value sequences
	shl %cx, 3 # times 8 to shift by bytes
	inc %ax # sym increment
	shr %rdx, %cl
	#
	and %dx, 0xFF # need a single byte value
	add %bx, %dx # bx = length
	jmp .sym_check_not_zero
	.endif
	#
.block_data_compute_values_end2:
	
	
	.if 0
	#
	mov %r15b, %al #
	# xchg %ax, %cx # cx = num of requested bits
	mov %cl, %al
	# mov %di, %ax
	mov %r14, %rbx
	
	sub %cl, %al # cl = num additional bits
	add %cl, 7
	shr %cl, 3 # cl = num bytes needed
	#
	cmp %cl, 1
	jb .get_bits_error
	#
	cmp %ecx, %r10d
	ja .get_bits_error
	#
	#
	mov %rsi, %r8
	#
	xor %rbx, %rbx
	#
.l111:
	xor %rax, %rax
	lodsb
	dec %cl
	shl %cl, 3
	inc %rsi
	shl %rax, %cl
	shr %cl, 3
	inc %cl
	or %rbx, %rax
	loopnz .l111
	
.l111_end:
	mov %rdx, %r12
	mov %al, %r15b
	movzx %ecx, %dl
	and %cl, 0x3F
	shr %rdx, 6
	shl %rbx, %cl
	#
	or %rdx, %rbx
	#
	mov %r8, %rsi
	mov %rbx, %r14
	
	.endif
	#
	#
	#
