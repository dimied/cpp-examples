	#
	#
	# ## CONSTANTS ##
	.equ ASM_INFLATE_AIO_STATE_RAW, 0
	.equ ASM_INFLATE_AIO_STATE_FIXED, 1
	.equ ASM_INFLATE_AIO_STATE_DYN, 2
	.equ ASM_INFLATE_AIO_STATE_ERROR, 3
	.equ ASM_INFLATE_AIO_STATE_NEW_STATE, 4
	.equ ASM_INFLATE_AIO_STATE_GET_BITS, 5
	.equ ASM_INFLATE_AIO_STATE_READ_HEADER, 6
	.equ ASM_INFLATE_AIO_STATE_READ_HEADER_AFTER, 7
	.equ ASM_INFLATE_AIO_STATE_INVALID, 28
	.equ ASM_INFLATE_AIO_STATE_BITSHIFT, 5
	.equ ASM_INFLATE_AIO_STATE_BITMASK, 0x1F
	
	# STACK POSITIONS
	.equ IN_AVAIL, 4
	.equ IN_SIZE, 8
	.equ OUT_AVAIL, 12
	.equ OUT_SIZE, 16
	.equ IN_DATA, 24
	.equ OUT_DATA, 32
	#
	.equ SRC_START, 40
	.equ DST_START, 48
	.equ BUFFER_PTR, 56
	# .equ COMP_STATE, - 64
	#
	.equ FIRST_TO_ZERO_POS, 72
	.equ FIRST_TO_ZERO, - 72
	.equ LAST_FLAG, (FIRST_TO_ZERO_POS)
	.equ RAX_BACKUP, (FIRST_TO_ZERO_POS + 8)
	.equ RBX_BACKUP, (FIRST_TO_ZERO_POS + 16)
	.equ RCX_BACKUP, (FIRST_TO_ZERO_POS + 24)
	.equ RDX_BACKUP, (FIRST_TO_ZERO_POS + 32)
	.equ RSI_BACKUP, (FIRST_TO_ZERO_POS + 40)
	.equ RDI_BACKUP, (FIRST_TO_ZERO_POS + 48)
	.equ R12_BACKUP, (FIRST_TO_ZERO_POS + 56)
	.equ R13_BACKUP, (FIRST_TO_ZERO_POS + 64)
	.equ LAST_BACKUP, ( - R13_BACKUP)
	#
	.equ LOCAL_VAR_OFFSET, (LAST_BACKUP - 8)
	.equ IN_BITS, (LOCAL_VAR_OFFSET - 8)
	.equ LAST_SYM, (LOCAL_VAR_OFFSET - 16)
	.equ CUR_TREE, (LOCAL_VAR_OFFSET - 24)
	.equ LENGTH_TREE, (LOCAL_VAR_OFFSET - 32)
	.equ DISTANCE_TREE, (LOCAL_VAR_OFFSET - 40)
	.equ LENGTH_PTR, (LOCAL_VAR_OFFSET - 48)
	.equ OFFSETS_PTR, (LOCAL_VAR_OFFSET - 56)
	.equ BUILD_TREE_LEN_PTR, (LOCAL_VAR_OFFSET - 64)
	.equ BUILT_TREE_SIZE, (LOCAL_VAR_OFFSET - 72)
	# 3 short values, 4.th value is sum
	.equ HLIT_HDIST_HCLEN, (LOCAL_VAR_OFFSET - 80)
	.equ BLOCK_DATA_LEN, (LOCAL_VAR_OFFSET - 88)
	.equ DECODE_TREE_IDX, (LOCAL_VAR_OFFSET - 96)
	
	.equ LAST_TO_ZERO, (DECODE_TREE_IDX)
	.equ NUM_TO_ZERO, - (LAST_TO_ZERO - FIRST_TO_ZERO)
	.equ NUM_TO_ZERO_8, (NUM_TO_ZERO / 8)
	#
	.equ HLIT_ADD, 257
	.equ HDIST_ADD, 1
	.equ HCLEN_ADD, 4
	.equ LENGTH_LEN, 320
	.equ OFFSETS_OFFSET, LENGTH_LEN
	.equ OFFSETS_LEN, 32
	#
	.equ CHECK_MMX, 1

	.section .data
	.extern somedata
	 # .extern inflate_fixed_pc
	 # .comm inflate_fixed_pc_asm, 258, 8
	
	.section .text
	
	.globl inflate_asm_x64
	.globl inflate_asm_x64_diffs
	.globl inflate_asm_x64_clear
	.globl inflate_asm_x64_fixed_addr
	.type inflate_asm_x64, @function
	.type inflate_asm_x64_diffs, @function
	.type inflate_asm_x64_clear, @function
	.type inflate_asm_x64_fixed_addr, @function
	
	#
	# .extern inflate_fixed_pc, 1232
		
	#
	# Parameters
# xDI : 1st, xSI : 2nd, xDX : 3rd, xCX : 4th, r8 : 5th
	#
# From search engine:
	# Register Preservation Rules
# - Caller - Saved (Volatile) Registers:
	# - %rax, %rcx, %rdx, %rsi, %rdi, %r8, %r9, %r10, %r11
	# - (can be overwritten by the callee).
# - Callee - Saved (Non - Volatile) Registers:
	# - %rbx, %rsp, %rbp, %r12, %r13, %r14, %r15
	# - (must be restored by the callee if used)
	#
	# R8D–R15D are the lowermost 32 bits of each register.
	# R8W–R15W are the lowermost 16 bits of each register.
	# R8B–R15B are the lowermost 8 bits of each register.
	#
# Let's use these additional 64 - bit registers:
# - %r8 : current in - position
# - %r9 : current out - position
# - %r10 : in - data available
# - %r11 : out - data available
	#
# - %r12 : bit - buffer, lowest byte is the bit count
# - %r13 : buffer ptr
# - %r14 : temp reg
# - %r15 : temp reg
	#
	# And we can use also XMM registers (XMM0 - XMM7), 
	# because all 64 - bit processors support XMM0.
	# And we don't need to deal with MMX - FP87 aliasing
# - xmm0 : temp
# - mm1 : temp
# - xmm7 : comp state
# commands:
	# - movd r32, xmm # movd xmm, r32
	# - movq r64, xmm # movq xmm, r64
	#
	#
	#
	.equ FUNC_LEN, (_inflate_asm_x64_end - inflate_asm_x64)
	.equ GET_BITS_LEN, (.get_bits_end - .get_bits)
	.equ RAW_JUMP, (.raw_mode - .inflate_aio_jmp_start_3)
	.equ FIXED_JUMP, (.fixed_mode - .inflate_aio_jmp_start_3)
	.equ DYN_JUMP, (.dyn_mode - .inflate_aio_jmp_start_3)
	.equ MODE_ADDRS, (RAW_JUMP | (FIXED_JUMP<<8) | (DYN_JUMP<<16))
	.equ SPACE_FOR_STACK, 16
	.equ MMX_SUPPORT_BIT, 23
	#
	#
	# Testing if 64-bit?
	# Use that REX.W in 32bit code is a DEC:
    # x48 x90
    # Which in 64bit code is:
    # rex.w nop   ; still a nop
    # and in 32bit code:
    # dec eax
	# nop
	#
	#
	
	.intel_syntax
	
inflate_asm_x64:
	.ifndef INFLATE_STACK_PTR_CHECK
	.equ INFLATE_STACK_PTR_CHECK, 1
	.endif
	#
	.ifndef INFLATE_RANGE_CHECK
	.equ INFLATE_RANGE_CHECK, 1
	.endif
	#
	.ifndef INFLATE_STACK_OVERLAP_CHECK
	.equ INFLATE_STACK_OVERLAP_CHECK, 1
	.endif
	#
	.ifndef CHECK_MEMORY_UNDERFLOW
	.equ CHECK_MEMORY_UNDERFLOW, 1
	.endif
	#
	.if INFLATE_STACK_PTR_CHECK
	cmp %esp, 1024 # unrealistic stack having so small address, but still...
	ja .enough_stack
	mov %al, 100
	movzx %eax, %al
	ret
.enough_stack:
	.endif
	push %rbp # callee saved
	push %rbx # callee saved
	#
	push %r12 # callee saved
	push %r13 # callee saved
	#
	push %r14 # callee saved
	push %r15 # callee saved
	#
	# Make some room on stack, just for the case ...
	mov %rbp, %rsp
	sub %rbp, 64
	#
	# Test MMX - Support
	.if CHECK_MMX
	#
	push %rdx
	push %rcx
	mov %eax, 1
	cpuid
	mov %eax, %edx
	pop %rcx
	pop %rdx
	bt %eax, 26 # 26.th bit for SSE2 - support
	jnc .error_state_not_implemented_mmx
	# jc .support_detected
	#
	.endif
	#
	#
.support_detected:
	# Validate inputs
	#
	xor %rax, %rax
	xor %rbx, %rbx
	cmp %rax, %rdi # dst addr
	je .error_state_invalid_params
	cmp %rax, %rdx # src addr
	je .error_state_invalid_params
	cmp %eax, %ecx # in size
	je .error_state_invalid_params
	cmp %eax, %esi # out size
	je .error_state_invalid_params
	cmp %rax, %r8 # valid buffer ptr
	je .error_state_invalid_params
	#
	# Validate ranges | paranoia ; - ) |
	# We store end pointers to make range check shorter
	# I.e. if addr + size >= end of address space, then it's an error
	# We also check for any overlaps with the stack space
	.if INFLATE_RANGE_CHECK
	.if INFLATE_STACK_OVERLAP_CHECK
	sub %rbp, 512
	.endif
	#
	mov %rax, %rdx
	mov %ebx, %ecx
	add %rax, %rbx # unsigned add, i.e. carry flag to check
	jc .error_state_not_supported_range
	.if INFLATE_STACK_OVERLAP_CHECK
	cmp %rdx, %rsp
	ja .stack_below_dst
	cmp %rax, %rbp
	ja .error_state_not_supported_range # overlap with the stack?
.stack_below_dst:
	.endif
	#
	mov %rax, %rdi
	mov %ebx, %esi
	add %rax, %rbx # unsigned add, i.e. carry flag to check
	jc .error_state_not_supported_range
	.if INFLATE_STACK_OVERLAP_CHECK
	cmp %rdi, %rsp
	ja .stack_below_src
	cmp %rax, %rbp
	ja .error_state_not_supported_range # overlap with the stack?
.stack_below_src:
	.endif
	#
	.if INFLATE_STACK_OVERLAP_CHECK
	add %rbp, 512
	.endif
	.endif
	#
	#
	.ifdef STACK_64B_ALIGNED
	.if STACK_64B_ALIGNED
	# make our 'private stack'(=rbp) be 64 - byte aligned
	mov %rax, %rbp
	and %ax, 63
	test %ax, %ax
	jz .inputs_on_stack # only 0 bits
	movzx %rax, %ax
	# mov %bx, %ax
	# xor %rax, %rax
	# mov %ax, %bx
	sub %rbp, %rax # adjust
	.endif
	.endif
	#
.inputs_on_stack:
	xor %rax, %rax
	xor %rbx, %rbx
	#
	mov [%rbp - OUT_DATA], %rdi # ptr to out data
	xor %r12, %r12 # clear bit buffer
	mov [%rbp - IN_DATA], %rdx # ptr to in data
	xor %r14, %r14
	mov [%rbp - IN_SIZE], %ecx
	mov [%rbp - OUT_SIZE], %esi
	mov %r13, %r8 # ptr to buffer
	#
	mov [%rbp - DST_START], %rdi # backup for dest
	mov [%rbp - SRC_START], %rdx # ptr to in data
	xor %r15, %r15
	#
	mov %eax, %ecx
	mov %ebx, %esi
	mov [%rbp - IN_AVAIL], %ecx
	mov [%rbp - OUT_AVAIL], %esi
	#
	add %rax, %rdx # rax = src end ptr
	add %rbx, %rdi # rbx = dst end ptr
	mov %r8, %rdx # src data
	mov %r9, %rdi # dst data
	mov %r10d, %ecx # in - avail
	mov %r11d, %esi # out - avail
	#
	movq %xmm5, %rax # input end ptr
	movq %xmm6, %rbx # output end ptr
	#
	xor %rdx, %rdx
	#
	# set all variables to 0
	xor %rax, %rax
	xor %rbx, %rbx
	xor %rcx, %rcx
	mov %rdi, %rbp # rdi = ptr to stack
	# nullify MMX registers
	movq %xmm0, %rax
	movq %xmm1, %rbx
	movq %xmm2, %rcx
	movq %xmm3, %rdx
	movq %xmm4, %rax
	#
	sub %rdi, FIRST_TO_ZERO_POS # adjust stack offset
.nullify_loop_1:
	mov %cx, NUM_TO_ZERO_8 # number of 8 - byte values
	js .error_state_internal_2
.nullify_loop: # set all vars on the stack to 0
	# mov [%rdi], %rax
	# sub %rdi, 8
	# loopnz .nullify_loop
	std
	repnz stosq
	#
	#
.algo_start:
	mov %dl, ASM_INFLATE_AIO_STATE_NEW_STATE
	movzx %edx, %dl
	# movl %edx, COMP_STATE(%rbp)
	movd %xmm7, %edx
	jmp .loop_start_edx
	#
	#
.loop_start:
	# movl COMP_STATE(%rbp), %edx
	movd %edx, %xmm7
	#
.loop_start_edx:
	.equ RANGE_CALC, 1
	
	.if RANGE_CALC
	movq %rax, %xmm5
	movq %rbx, %xmm6
	sub %rax, %r8
	.if CHECK_MEMORY_UNDERFLOW
	jc .error_state_internal_1
	.endif
	sub %rbx, %r9
	.if CHECK_MEMORY_UNDERFLOW
	jc .error_state_internal_1
	.endif
	
	.else
	
	xor %rcx, %rcx
	#
	mov %rbx, [%rbp - SRC_START]
	mov %rax, %r8 # rax = in_data_ptr
	#
	mov %rsi, [%rbp - DST_START]
	mov %rdi, %r9 # rdi = out_data_ptr
	mov %ecx, [%rbp - IN_SIZE]
	#
	# cmp %rax, %rbx
	# jb .error_state_internal_1
	sub %rax, %rbx
	.if CHECK_MEMORY_UNDERFLOW
	jc .error_state_internal_1
	.endif
	#
	# cmp %rdi, %rsi
	# jb .error_state_internal_1
	sub %rdi, %rsi
	.if CHECK_MEMORY_UNDERFLOW
	jc .error_state_internal_1
	.endif
	#
	#
	mov %ebx, [%rbp - OUT_SIZE]
	#
	sub %ecx, %eax # ecx = in_bytes avail
	.if CHECK_MEMORY_UNDERFLOW
	jc .error_state_internal_2
	.endif
	mov %eax, %ecx # eax = in_bytes avail
	#
	# cmp %ecx, %edi
	# jb .error_state_internal_2
	sub %ebx, %edi # ebx = bytes available to write
	.if CHECK_MEMORY_UNDERFLOW
	jc .error_state_internal_2
	.endif
	# mov %edi, %ebx # edi = bytes available to write
	#
	.endif
	#
	mov %ecx, %edx # command
	mov %r10d, %eax # in_bytes left
	mov [%rbp - IN_AVAIL], %eax # stack backup
	mov %r11d, %ebx # out_bytes avail
	mov [%rbp - OUT_AVAIL], %ebx # stack backup
	#
	# Not sizes are checked
	# And we process the command
	#
	and %dl, ASM_INFLATE_AIO_STATE_BITMASK # current command to do
	shr %ecx, ASM_INFLATE_AIO_STATE_BITSHIFT # next command for next iteration
	movzx %edx, %dl # just to make comparison with (e)dx possible
	# movl %ecx, COMP_STATE(%rbp)
	movd %xmm7, %ecx # store comp state for the next iteration
	
	# Invalid command?
	cmp %dl, ASM_INFLATE_AIO_STATE_INVALID
	jge .error_state
	# header = 3? I.e. error
	cmp %dl, ASM_INFLATE_AIO_STATE_ERROR
	je .error_state
	#
	xor %rcx, %rcx
	#
.inflate_aio_jmp_start:
	cmp %dl, ASM_INFLATE_AIO_STATE_READ_HEADER_AFTER
	ja .inflate_aio_jmp_start_2b
	#
	.ifndef COMP_JUMP_ADDRESS
	.equ COMP_JUMP_ADDRESS, 1
	.endif
	.if COMP_JUMP_ADDRESS
	cmp %dl, ASM_INFLATE_AIO_STATE_DYN
	ja .inflate_aio_jmp_start_2
	mov %ebx, MODE_ADDRS
	mov %cl, %dl
	shl %cl, 3 # mul by 8
	shr %ebx, %cl
	movzx %ebx, %bl
	call .getrip
	.endif
.inflate_aio_jmp_start_3:
	.if COMP_JUMP_ADDRESS
	add %rax, %rbx
	jmp %rax
	.else
	# Here we begin with valid commands
	cmp %dl, ASM_INFLATE_AIO_STATE_RAW
	je .raw_mode
	cmp %dl, ASM_INFLATE_AIO_STATE_FIXED
	je .fixed_mode
	cmp %dl, ASM_INFLATE_AIO_STATE_DYN
	je .dyn_mode
	.endif
	#
.inflate_aio_jmp_start_2:
	# error already checked
	cmp %dl, ASM_INFLATE_AIO_STATE_NEW_STATE
	je .new_state
	cmp %dl, ASM_INFLATE_AIO_STATE_GET_BITS
	je .get_bits
	cmp %dl, ASM_INFLATE_AIO_STATE_READ_HEADER
	je .read_header
	cmp %dl, ASM_INFLATE_AIO_STATE_READ_HEADER_AFTER
	je .read_header_after
	
	#
.inflate_aio_jmp_start_2b:
	jmp .error_state
	
	
.raw_mode:
	mov %eax, 100
	jmp .restore_and_return
	.ifndef CHECK_RAW_SIZES
	.equ CHECK_RAW_SIZES, 1
	.endif
	
	xor %rcx, %rcx
	mov %rsi, %r8 # rsi = src ptr
	xor %r12d, %r12d # reset bit buffer
	mov %cl, 4
	cmp %r10d, %ecx # enough bytes to check the length?
	jb .error_state
	mov %edx, [%rsi]
	sub %r10d, %ecx # in_avail - 4
	mov %eax, %edx
	add %rsi, %rcx # adjust input ptr(r8)
	#
	.if CHECK_RAW_SIZES
	# simple validity check, low 16 bits =?= high 16 bits negated
	shr %edx, 16
	movzx %eax, %ax
	not %dx
	cmp %dx, %ax # equal high / low 16 - bits ?
	jne .error_state
	.endif
	#
	cmp %r10d, %eax # enough input data ?
	jb .error_state
	mov %ecx, %eax # ecx = counter for copying
	cmp %r11d, %eax # enough output space ?
	jb .error_state
	# testl %eax, %eax
	# jz .loop_start_edx # unprobable ...
	jecxz .raw_mode_jmp_new_state # can't jump too far ...
	#
	mov %rdi, %r9 # rdi = dest ptr
	.ifdef RAW_MODE_COPY_8
	.if RAW_MODE_COPY_8
	mov %ebx, %r8d # ebx = src ptr
	
	#
	cmp %cx, 16
	jbe .raw_mode_copy_bytes
	#
	or %bx, %di # mix bits
	test %cx, 0x7 # 8 - byte - aligned?
	jnz .raw_mode_copy_bytes
	# alignment checks, maybe we can copy larger chunks per "rep movsX" - iteration
	test %bx, 0x7 # 8 - byte - aligned si|di ?
	jnz .raw_mode_copy_bytes
	shr %ecx, 3 # i.e div by 8
	cld # forward copy!
	repnz movsq
	jmp .raw_mode_copy_done
	#
	.endif
	.endif
	
.raw_mode_copy_bytes:
	cld # forward copy!
	repnz movsb
.raw_mode_copy_done:
	sub %r10d, %eax # in - avail
	mov %r8, %rsi # update src ptr(r8)
	#
	# no jump to main loop, can adjust in / out available counts here
	sub %r11d, %eax # out - avail
	mov %r9, %rdi # update dst ptr(r9)
.raw_mode_jmp_new_state:
	jmp .new_state
	#
.fixed_mode:
	nop
	jmp .error_state_not_implemented
	#
.dyn_mode:
	nop
	jmp .error_state_not_implemented
	#
	
.new_state:
	mov %eax, [%rbp - LAST_FLAG]
	xor %edx, %edx
	# cmp $1, %al # last bit set ?
	bt %ax, 0 # test 0.th bit, if bit set, then carry flag also set
	jc .ok_state # then we just processed the last block
.read_header:
	xor %edx, %edx
	xor %eax, %eax
	xor %ebx, %ebx
	# set next command after get_bits
	mov %dl, ASM_INFLATE_AIO_STATE_READ_HEADER_AFTER
	mov %al, 3 # 3 bits for the block header, actually it's 1 bit and then 2
	movd %xmm7, %edx
	#
# eax: #bits, %ebx: offset
.get_bits:
	.ifdef gb_func
	.if gb_func
	mov %r14, %rsp, 
	sub %rsp, 256
	call inf_get_bits
	mov %rsp, %r14
	.endif
	.endif
	#
	
	# backup values
	mov [%rbp - RDX_BACKUP], %edx
	mov [%rbp - RCX_BACKUP], %ecx
	mov %edx, %r12d # packed bitbuf and count
	mov [%rbp - RSI_BACKUP], %rsi
	#
	movzx %ecx, %dl # cl = dl, i.e. cl=bit count
	shr %edx, 8 # now edx contains the real bits
	cmp %al, %cl # enough bits in the bit buffer?
	jbe .get_bits_ok
	#
	# fill more bits!
	# cmpl $0, %r10d # enough in - avail ?
	test %r10d, %r10d
	# je .error_state_no_bits
	jz .error_state_no_bits
	#
	movq %rsi, %r8 # rsi = src data ptr
	#
	mov %r15d, %ebx # ebx - > temp
	mov %edi, %r10d # edi = in - avail
	#
	xor %ebx, %ebx
	jmp .get_bits_more_2
	#
.get_bits_more:
	test %edi, %edi
	jz .error_state
.get_bits_more_2:
	mov %bl, [%rsi]
	dec %edi # avail - 1
	shl %edx, 8 # bit buf <<
	add %cl, 8 #
	inc %rsi
	or %dl, %bl # new byte to bit buf
	cmp %cl, %al # enough in the bit buffer ?
	ja .get_bits_more
	#
	mov %ebx, %r15d
	mov %r8, %rsi # src data
	mov [%rbp - IN_DATA], %rsi
	#
.get_bits_ok:
	xor %esi, %esi
	mov %r15d, %edx # bit buf
	inc %esi
	xchg %al, %cl # cl = requested bits
	# movl %eax, %ecx
	shl %esi, %cl # <<in_bits
	dec %esi # => esi = ((1<<in_bits) - 1)
	and %edx, %esi # use bitmask to get the bits
	add %ebx, %edx # add to offset
	#
	cmp %cl, %al
	# jne .get_bits_pack
	jb .error_state_no_bits
	ja .get_bits_pack
	# equal means, bitbuf is empty
	xor %r12d, %r12d # empty bit buffer!
	jmp .get_bits_pack_store_end
	#
.get_bits_pack:
	mov %edx, %r15d
	sub %cl, %al # reduce bit count
	shr %edx, %cl # remove consumed bits
	shl %edx, 8 # pack bit buf
	or %dl, %al # pack bit count
.get_bits_pack_store:
	mov %r12d, %edx # store bitbuf and bit count
.get_bits_pack_store_end:
	mov %eax, %ebx # eax = result
	mov [%rbp + IN_BITS], %ebx
	mov %edx, [%rbp - RDX_BACKUP]
	movq %rsi, [%rbp - RSI_BACKUP]
	#
	movd %ebx, %xmm7 # ebx = comp state
	mov %ecx, [%rbp - RCX_BACKUP]
	#
	cmp %bl, ASM_INFLATE_AIO_STATE_READ_HEADER_AFTER
	jne .loop_start
	mov %edx, %eax
	jmp .read_header_after_edx
.get_bits_end:
	# now eax contains the required bits
	# jmp .loop_start
.read_header_after:
	mov %edx, [%rbp + IN_BITS]
.read_header_after_edx:
	mov [%rbp + LAST_FLAG], %edx # don't need to filter out just the lowest bit
	shr %dx, 1
	jmp .loop_start_edx
	#
	
.loop_end:
	
	# local variables rbp - 32
.error_state_4:
	# movl $4, %eax
	mov %al, 4
	jmp .restore_and_return
.error_state:
.error_state_no_bits:
	mov %al, 3
	jmp .restore_and_return
.error_state_invalid_params:
	# movl $6, %eax
	mov %al, 6
	jmp .restore_and_return
.error_state_internal_1:
	mov %al, 9
	jmp .restore_and_return
.error_state_internal_2:
	mov %al, 10
	jmp .restore_and_return
	#
	.if INFLATE_RANGE_CHECK
.error_state_not_supported_range:
	mov %al, 8
	jmp .restore_and_return
	.endif
	#
.error_state_not_implemented_mmx:
	mov %al, 7
	jmp .restore_and_return_no_mmx
.error_state_not_implemented:
	mov %al, 5
	jmp .restore_and_return
.ok_state:
	xor %eax, %eax
.restore_and_return:
	# emms # clear MMX state
	movzx %eax, %al
.restore_and_return_no_mmx:
	pop %r15
	pop %r14
	#
	pop %r13
	pop %r12
	#
	pop %rbx
	pop %rbp
	ret
.getrip:
	mov %rax, [%rsp]
	ret
inf_get_bits:
	ret
_inflate_asm_x64_end:
	ret
	
	
inflate_asm_x64_diffs:
	push %rbp # callee saved
	mov %rbp, %rsp
	
	mov %eax, FUNC_LEN
	mov [%rdi], %eax
	add %rdi, 4
	#
	mov %eax, GET_BITS_LEN
	mov [%rdi], %eax
	add %rdi, 4
	#
	mov %eax, NUM_TO_ZERO
	mov [%rdi], %eax
	add %rdi, 4
	#
	mov %eax, NUM_TO_ZERO_8
	mov [%rdi], %eax
	add %rdi, 4
	#
	mov %eax, LAST_TO_ZERO
	neg %eax
	mov [%rdi], %eax
	add %rdi, 4
	#
	mov %eax, RAW_JUMP
	mov [%rdi], %eax
	add %rdi, 4
	#
	mov %eax, FIXED_JUMP
	mov [%rdi], %eax
	add %rdi, 4
	#
	mov %eax, DYN_JUMP
	mov [%rdi], %eax
	add %rdi, 4
	#
	mov %eax, MODE_ADDRS
	mov [%rdi], %eax
	add %rdi, 4
	#
	pop %rbp
	ret
	
# 1st = %rdi : address        ; 2nd = %esi : count
inflate_asm_x64_clear:
	pushq %rcx
	mov %ecx, %esi
	xorq %rax, %rax
	std
	repnz stosq
	popq %rcx
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

inflate_asm_x64_fixed_addr:
	# mov %eax, somedata
	# mov [%rdi], %eax
	ret

	 

