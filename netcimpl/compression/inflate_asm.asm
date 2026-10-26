	#
	#
	# ## CONSTANTS ##
	.equ ASM_INFLATE_STATE_RAW, 0
	.equ ASM_INFLATE_STATE_FIXED, 1
	.equ ASM_INFLATE_STATE_DYN, 2
	#
	.equ ASM_INFLATE_STATE_ERROR, 3
	#
	.equ ASM_INFLATE_STATE_NEW_STATE, 4
	.equ ASM_INFLATE_STATE_GET_BITS, 5
	.equ ASM_INFLATE_STATE_READ_HEADER, 6
	.equ ASM_INFLATE_STATE_READ_HEADER_AFTER, 7
	.equ ASM_INFLATE_STATE_B_LT1, 8 # build length tree 1
	.equ ASM_INFLATE_STATE_L_LT, 9 # get lengths for length tree
	.equ ASM_INFLATE_STATE_B_LT, 10 # build length tree
	.equ ASM_INFLATE_STATE_B_DT, 11 # build data tree
	.equ ASM_INFLATE_STATE_BLD, 12 # block data
	
	
	.equ ASM_INFLATE_STATE_GET_SYMBOL, 8
	.equ ASM_INFLATE_STATE_GET_BITS_GET_SYMBOL, 9
	.equ ASM_INFLATE_STATE_GET_BITS_BLOCK_DATA, 10
	.equ ASM_INFLATE_STATE_GET_BITS_DATA_OFFSET, 11
	.equ ASM_INFLATE_STATE_GET_BITS_BUILD_TREES, 12
	.equ ASM_INFLATE_STATE_GET_BITS_LENGTH_LOOP, 13
	.equ ASM_INFLATE_STATE_GET_BITS_LENGTH_LOOP_FIRST, 14
	.equ ASM_INFLATE_STATE_LAST_VALID, 14
	.equ ASM_INFLATE_BITSHIFT, 5
	.equ ASM_INFLATE_BITSHIFT2, 10
	.equ ASM_INFLATE_BITSHIFT3, 15
	.equ ASM_INFLATE_BITSHIFT4, 20
	.equ ASM_INFLATE_BITMASK, 0x1F
	#
	#
	.equ DYN_TREE_SEQUENCE, (ASM_INFLATE_STATE_B_LT1|(ASM_INFLATE_STATE_L_LT<<ASM_INFLATE_BITSHIFT)|(ASM_INFLATE_STATE_B_LT<<ASM_INFLATE_BITSHIFT2)|(ASM_INFLATE_STATE_B_DT<<ASM_INFLATE_BITSHIFT3)|(ASM_INFLATE_STATE_BLD<<ASM_INFLATE_BITSHIFT4))
	#
	#
	# STACK POSITIONS
	.equ IN_SIZE, 4
	.equ OUT_SIZE, 8
	.equ IN_AVAIL, 12
	.equ OUT_AVAIL, 16
	.equ IN_DATA, 24
	.equ OUT_DATA, 32
	#
	.equ SRC_START, 40
	.equ DST_START, 48
	#
	.equ FIRST_TO_ZERO_POS, 64 # 72
	# .equ FIRST_TO_ZERO, - 72
	.equ LAST_FLAG, (FIRST_TO_ZERO_POS)
	.equ RAX_BACKUP, (FIRST_TO_ZERO_POS + 8)
	.equ RBX_BACKUP, (FIRST_TO_ZERO_POS + 16)
	.equ RCX_BACKUP, (FIRST_TO_ZERO_POS + 24)
	.equ RDX_BACKUP, (FIRST_TO_ZERO_POS + 32)
	.equ RSI_BACKUP, (FIRST_TO_ZERO_POS + 40)
	# .equ RDI_BACKUP, (FIRST_TO_ZERO_POS + 48)
	.equ LAST_BACKUP, (RSI_BACKUP)
	# .equ R12_BACKUP, (FIRST_TO_ZERO_POS + 56)
	# .equ R13_BACKUP, (FIRST_TO_ZERO_POS + 64)
	# .equ LAST_BACKUP, (R13_BACKUP)
	#
	.equ LOCAL_VAR_OFFSET, (LAST_BACKUP + 8)
	.equ IN_BITS, (LOCAL_VAR_OFFSET + 8)
	# .equ LAST_SYM, (LOCAL_VAR_OFFSET + 16)
	# .equ CUR_TREE, (LOCAL_VAR_OFFSET + 24)
	.equ LENGTH_PTR, (LOCAL_VAR_OFFSET + 16)
	.equ OFFSETS_PTR, (LOCAL_VAR_OFFSET + 24)
	.equ BUILD_TREE_LEN_PTR, (LOCAL_VAR_OFFSET + 32)
	.equ BUILT_TREE_SIZE, (LOCAL_VAR_OFFSET + 40)
	# 3 short values, 4.th value is sum
	.equ HLIT_HDIST_HCLEN, (LOCAL_VAR_OFFSET + 48)
	.equ BLOCK_DATA_LEN, (LOCAL_VAR_OFFSET + 56)
	.equ DECODE_TREE_IDX, (LOCAL_VAR_OFFSET + 64)
	
	.equ LAST_TO_ZERO, (DECODE_TREE_IDX)
	.equ NUM_TO_ZERO, (LAST_TO_ZERO - FIRST_TO_ZERO_POS)
	.equ NUM_TO_ZERO_8, (NUM_TO_ZERO / 8)
	#
	.equ LENGTH_TREE_OFFSET, 384
	.equ TREE_SIZE, 640
	.equ DISTANCE_TREE_OFFSET, (LENGTH_TREE_OFFSET + TREE_SIZE)
	#
	.equ HLIT_ADD, 257
	.equ HDIST_ADD, 1
	.equ HCLEN_ADD, 4
	.equ LENGTH_LEN, 320
	.equ OFFSETS_OFFSET, LENGTH_LEN
	.equ OFFSETS_LEN, 32
	#
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
# - xmm1 : get bits temp 1
# - xmm2 : get bits temp 2
# - xmm3 : get bits temp 3
# - xmm4 : get bits temp 4
# - xmm5 : src end ptr
# - xmm6 : dest end ptr
# - xmm7 : comp state
# commands:
	# - movd r32, xmm # movd xmm, r32
	# - movq r64, xmm # movq xmm, r64
	#
	.equ FUNC_LEN, (_inflate_asm_x64_end - inflate_asm_x64)
	.equ GET_BITS_LEN, (.get_bits_end - .get_bits)
	.equ RAW_JUMP, (.raw_mode - .inflate_aio_jmp_start_3)
	.equ FIXED_JUMP, (.fixed_mode - .inflate_aio_jmp_start_3)
	.equ DYN_JUMP, (.dyn_mode - .inflate_aio_jmp_start_3)
	.equ NEW_STATE_JUMP, (.new_state - .inflate_aio_jmp_start_3)
	.equ MODE_ADDRS, (RAW_JUMP | (FIXED_JUMP<<8) | (DYN_JUMP<<16))
	#
	.equ MMX_SUPPORT_BIT, 23
	# clear reg flags
	.equ CLR_RAX, 1
	.equ CLR_RAX_BIT, 0
	#
	.equ CLR_RBX, 2
	.equ CLR_RBX_BIT, 1
	#
	.equ CLR_RCX, 4
	.equ CLR_RCX_BIT, 2
	#
	.equ CLR_RDX, 8
	.equ CLR_RDX_BIT, 3
	#
	.equ CLR_RSI, 16
	.equ CLR_RSI_BIT, 4
	#
	# .equ CLR_RDI, 32
	# .equ CLR_RDI_BIT, 5
	#
	#
	# Testing if 64 - bit?
# Use that REX.W in 32bit code is a DEC:
	# x48 x90
# Which in 64bit code is:
	# rex.w nop                  ; still a nop
# and in 32bit code:
	# dec eax
	# nop
	#
	#
	
	.intel_syntax
	
inflate_asm_x64:
	.ifndef INFLATE_ASM_CHECK_ERRORS
	.equ INFLATE_ASM_CHECK_ERRORS, 1
	.endif
	#
	.ifndef INFLATE_ASM_CHECK_SSE2
	.equ INFLATE_ASM_CHECK_SSE2, 1
	.endif
	#
	.ifndef INFLATE_ASM_STACK_PTR_CHECK
	.equ INFLATE_ASM_STACK_PTR_CHECK, 0
	.endif
	#
	.ifndef INFLATE_ASM_STACK_64B_ALIGNED
	.equ INFLATE_ASM_STACK_64B_ALIGNED, 1
	.endif
	#
	.ifndef INFLATE_ASM_RANGE_CHECK
	.equ INFLATE_ASM_RANGE_CHECK, 1
	.endif
	#
	.ifndef INFLATE_STACK_OVERLAP_CHECK
	.equ INFLATE_STACK_OVERLAP_CHECK, 0
	.endif
	#
	.ifndef INFLATE_ASM_NULLIFY_STACK
	.equ INFLATE_ASM_NULLIFY_STACK, 1
	.endif
	#
	.ifndef CHECK_MEMORY_UNDERFLOW
	.equ CHECK_MEMORY_UNDERFLOW, 0
	.endif
	#
	.ifndef COMP_JUMP_ADDRESS
	.equ COMP_JUMP_ADDRESS, 0
	.endif
	# for raw values
	.ifndef CHECK_RAW_SIZES
	.equ CHECK_RAW_SIZES, 1
	.endif
	# algo prop for fixed
	.ifndef COMPACT_IMPL
	.equ COMPACT_IMPL, 0
	.endif
	# algo prop for fixed
	.ifndef COMPACT_IMPL2
	.equ COMPACT_IMPL2, 1
	.endif
	# algo prop for fixed
	.ifndef COMPACT_IMPL_MUL
	.equ COMPACT_IMPL_MUL, 1
	.endif
	# use functions to clear registers?
	.ifndef USE_CLEAR_REGS
	.equ USE_CLEAR_REGS, 1
	.endif
	#
	# experiment
	.equ DADA, 0
	#
	.if INFLATE_ASM_STACK_PTR_CHECK
	cmp %rsp, 512 # unrealistic stack having so small address, but still...
	ja .enough_stack
	jmp .error_state_not_supported_range
.enough_stack:
	.endif
	#
	push %rbp # callee saved
	push %rbx # callee saved
	#
	push %r12 # callee saved
	push %r13 # callee saved
	#
	push %r14 # callee saved
	push %r15 # callee saved
	#
	# Test MMX - Support
	.if INFLATE_ASM_CHECK_SSE2
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
	#
	.endif
	#
	# Validate inputs
	#
	test %rdi, %rdi # dst addr
	jz .error_state_invalid_params
	test %rdx, %rdx # src addr
	jz .error_state_invalid_params
	test %ecx, %ecx # in size
	jz .error_state_invalid_params
	test %esi, %esi # out size
	jz .error_state_invalid_params
	test %r8, %r8 # valid buffer ptr
	jz .error_state_invalid_params
	#
	#
	# Make some room on stack, just for the case ...
	mov %rbp, %rsp
	sub %rbp, 64
	#
	# Limit ranges to 2^32 - 1
	#
	xor %rbx, %rbx
	dec %rbx # minus 1, i.e. all bits sets
	shr %rbx, 32 # rbx should be now 2^32 - 1
	and %rcx, %rbx # in count
	and %rsi, %rbx # out count
	#
	.if INFLATE_ASM_STACK_64B_ALIGNED
	# make our 'private stack'(=rbp) be 64 - byte aligned
	mov %rax, %rbp
	and %ax, 63
	test %ax, %ax
	jz .inputs_on_stack_aligned # only 0 bits
	movzx %rax, %ax
	sub %rbp, %rax # adjust
.inputs_on_stack_aligned:
	.endif
	#
	xor %rax, %rax
	#
	# Validate ranges | paranoia ; - ) |
	# We store end pointers to make range check shorter
	# I.e. if addr + size >= end of address space, then it's an error
	# We also check for any overlaps with the stack space
	sub %rbp, 256
	.if INFLATE_ASM_RANGE_CHECK
	#
	mov %rax, %rdx
	add %rax, %rcx # unsigned add, i.e. carry flag for unsigned overflow
	jc .error_state_not_supported_range
	#
	.if INFLATE_STACK_OVERLAP_CHECK
	cmp %rdx, %rsp
	ja .stack_below_src
	cmp %rax, %rbp # data overlaps with stack?
	ja .error_state_not_supported_range # overlap with the stack?
.stack_below_src:
	.endif
	#
	mov %rax, %rdi
	add %rax, %rsi # unsigned add, i.e. carry flag for unsigned overflow
	jc .error_state_not_supported_range
	#
	.if INFLATE_STACK_OVERLAP_CHECK
	cmp %rdi, %rsp
	ja .stack_below_dest
	cmp %rax, %rbp # data overlaps with stack?
	ja .error_state_not_supported_range # overlap with the stack?
.stack_below_dest:
	.endif
	#
	.endif
	#
	add %rbp, 256
	
	.if INFLATE_ASM_NULLIFY_STACK
	xor %ebx, %ebx
	xor %rax, %rax
	mov %ebx, %ecx
	mov %r15, %rdi
	mov %cx, 32
	mov %rdi, %rbp
	movzx %rcx, %cx
	sub %rdi, %rcx
	#
	std
	rep stosq
	#
	mov %ecx, %ebx
	mov %rdi, %r15
	.endif
	#
	#
.inputs_on_stack:
	xor %rax, %rax
	xor %rbx, %rbx
	#
	mov [%rbp - IN_SIZE], %ecx
	mov [%rbp - OUT_SIZE], %esi
	mov [%rbp - IN_AVAIL], %ecx
	mov [%rbp - OUT_AVAIL], %esi
	#
	mov [%rbp - IN_DATA], %rdx # ptr to in data
	mov [%rbp - OUT_DATA], %rdi # ptr to out data
	xor %r14, %r14
	xor %r12, %r12 # clear bit buffer
	#
	mov [%rbp - SRC_START], %rdx # ptr to in data
	mov [%rbp - DST_START], %rdi # backup for dest
	xor %r15, %r15
	#
	mov %eax, %ecx # in size
	mov %ebx, %esi # out size
	mov %r13, %r8 # ptr to buffer
	
	#
	add %rax, %rdx # rax = src end ptr
	.if INFLATE_ASM_CHECK_ERRORS
	jc .error_state_not_supported_range
	.endif
	add %rbx, %rdi # rbx = dst end ptr
	.if INFLATE_ASM_CHECK_ERRORS
	jc .error_state_not_supported_range
	.endif
	mov %r8, %rdx # src data ptr
	mov %r9, %rdi # dst data ptr
	#
	mov %r10, %rcx # in - avail
	mov %r11, %rsi # out - avail
	#
	movq %xmm5, %rax # input end ptr
	movq %xmm6, %rbx # output end ptr
	#
	xor %rdx, %rdx
	#
	# set all variables to 0
	.if USE_CLEAR_REGS
	mov %al, (CLR_RAX|CLR_RBX|CLR_RCX|CLR_RDX)
	call .clear_regs
	.else
	xor %rax, %rax
	xor %rbx, %rbx
	xor %rcx, %rcx
	xor %rdx, %rdx
	.endif
	
	# nullify MMX registers
	# movq %xmm0, %rax
	pxor %xmm0, %xmm0
	# movq %xmm1, %rbx
	pxor %xmm1, %xmm1
	# movq %xmm2, %rax
	pxor %xmm2, %xmm2
	#
	# movq %xmm3, %rbx
	pxor %xmm3, %xmm3
	# movq %xmm4, %rax
	pxor %xmm4, %xmm4
	#
	#
.algo_start:
	mov %dl, ASM_INFLATE_STATE_NEW_STATE
	# movzx %rdx, %dl
	movd %xmm7, %edx # store comp state
	jmp .loop_start_edx
	#
	#
.loop_start:
	movd %rdx, %xmm7 # load comp state into edx
	#
.loop_start_edx:
	xor %r10, %r10
	xor %r11, %r11
	#
	movq %rax, %xmm5 # src end ptr
	movq %rbx, %xmm6 # dest end ptr
	#
	sub %rax, %r8
	sub %rbx, %r9
	#
	mov %ecx, %edx # command
	mov %r10d, %eax # in_bytes left
	# mov [%rbp - IN_AVAIL], %eax # stack backup
	mov %r11d, %ebx # out_bytes avail
	# mov [%rbp - OUT_AVAIL], %ebx # stack backup
	#
	# Not sizes are checked
	# And we process the command
	#
	and %dl, ASM_INFLATE_BITMASK # current command to do
	shr %ecx, ASM_INFLATE_BITSHIFT # next command for next iteration
	movzx %edx, %dl # just to make comparison with (e)dx possible
	# movl %ecx, COMP_STATE(%rbp)
	movd %xmm7, %ecx # store comp state for the next iteration
	
	# Invalid command?
	cmp %dl, ASM_INFLATE_STATE_LAST_VALID
	jae .error_state
	# header = 3? I.e. error
	cmp %dl, ASM_INFLATE_STATE_ERROR
	je .error_state
	#
	xor %rcx, %rcx
	#
	.if COMP_JUMP_ADDRESS
	cmp %dl, ASM_INFLATE_STATE_DYN
	ja .inflate_aio_jmp_start_2
	#
	mov %cl, %dl
	mov %ebx, MODE_ADDRS
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
	cmp %dl, ASM_INFLATE_STATE_RAW
	je .raw_mode
	cmp %dl, ASM_INFLATE_STATE_FIXED
	je .fixed_mode
	cmp %dl, ASM_INFLATE_STATE_DYN
	je .dyn_mode
	.endif
	#
.inflate_aio_jmp_start_2:
	# error already checked
	cmp %dl, ASM_INFLATE_STATE_NEW_STATE
	je .new_state
	cmp %dl, ASM_INFLATE_STATE_GET_BITS
	je .get_bits
	# cmp %dl, ASM_INFLATE_STATE_READ_HEADER
	# je .read_header
	cmp %dl, ASM_INFLATE_STATE_READ_HEADER_AFTER
	je .read_header_after
	#
	#
.inflate_aio_jmp_start_2b:
	jmp .error_state
	#
	#
	#
.raw_mode:
	#
	xor %rax, %rax
	xor %rcx, %rcx
	mov %rsi, %r8 # rsi = src ptr
	mov %cl, 4
	xor %r12d, %r12d # reset bit buffer
	#
	# cmp %r10d, %ecx # enough bytes to check the length?
	# jb .error_state
	sub %r10d, %ecx # in_avail minus 4
	jc .error_state
	mov %edx, [%rsi]
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
	#
	jecxz .raw_mode_jmp_new_state # can't jump too far ...
	#
	mov %rdi, %r9 # rdi = dest ptr
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
	#
.fixed_mode:
	jmp .fixed_mode_build_trees
.fixed_mode_process:
	jmp .inflate_aio_block_data
	#
.dyn_mode:
	nop
	#
	jmp .error_state_not_implemented
	#
	#
# .inflate_copy_data:
	# jecxz .inflate_copy_data_done
	# cld
	# rep movsb
# .inflate_copy_data_done:
	# ret
	#
	#
.new_state:
	mov %ax, [%rbp - LAST_FLAG]
	xor %edx, %edx
	# cmp $1, %al # last bit set ?
	bt %ax, 0 # test 0.th bit, if bit set, then carry flag also set
	jc .ok_state # then we just processed the last block
# .read_header:
	xor %rdx, %rdx
	xor %eax, %eax
	xor %ebx, %ebx
	# set next command after get_bits
	mov %dl, ASM_INFLATE_STATE_READ_HEADER_AFTER
	mov %al, 3 # 3 bits for the block header, actually it's 1 bit and then 2
	movd %xmm7, %edx
	#
# eax: #bits, %ebx: offset
.get_bits:
	.ifndef GET_BIT_BACKUP_ON_STACK
	.equ GET_BIT_BACKUP_ON_STACK, 0
	.endif
	#
	.ifndef GET_BIT_USE_SHIFTS
	.equ GET_BIT_USE_SHIFTS, 1
	.endif
	#
	movzx %eax, %ax
	#
	test %ax, %ax # any bits ?
	jz .get_bits_no_bits
	#
	cmp %ax, 24 # more than 24 bits ?
	ja .error_state_4
	
	# backup values
	.if GET_BIT_BACKUP_ON_STACK
	mov [%rbp - RDX_BACKUP], %edx
	mov [%rbp - RCX_BACKUP], %ecx
	.else
	#
	.if GET_BIT_USE_SHIFTS
	movq %xmm2, %rdx
	movq %xmm3, %rcx
	psllq %xmm2, 64
	por %xmm2, %xmm3
	.else
	movq %xmm1, %rdx
	movq %xmm2, %rcx
	.endif
	#
	.endif
	
	mov %edx, %r12d # packed bitbuf and count
	#
	.if GET_BIT_BACKUP_ON_STACK
	mov [%rbp - RSI_BACKUP], %rsi
	.else
	#
	movq %xmm3, %rsi
	movq %xmm4, %rdi
	#
	.endif
	#
	#
	#
	movzx %ecx, %dl # cl = dl, i.e. cl = bit count
	shr %edx, 8 # now edx contains the real bits
	cmp %al, %cl # enough bits in the bit buffer?
	jbe .get_bits_ok
	#
	# fill more bits!
	#
	mov %edi, %ebx # ebx = temp
	#
	xor %ebx, %ebx
	#
.get_bits_more:
	test %r10d, %r10d # enough available
	jz .error_state_no_bits
.get_bits_more_2:
	mov %bl, [%r8] # single byte
	dec %r10d # avail - 1
	shl %edx, 8 # bit buf <<
	add %cl, 8 # inc bit buf count
	inc %r8 # next addr
	mov %dl, %bl # new byte to bit buf
	cmp %al, %cl # enough in the bit buffer ?
	ja .get_bits_more # repeat if not
	#
	mov %ebx, %edi
	# mov [%rbp - IN_DATA], %r8 # src data
	#
.get_bits_ok:
	xor %esi, %esi
	mov %edi, %edx # bit buf
	inc %si
	xchg %al, %cl # cl = requested bits
	#
	shl %esi, %cl # << in_bits
	dec %esi # => esi = ((1<<in_bits) - 1)
	and %edx, %esi # use bitmask to get the bits
	add %ebx, %edx # add to offset
	#
	cmp %cl, %al
	ja .error_state_no_bits
	jb .get_bits_pack
	# equal means, bitbuf is empty
	xor %r12d, %r12d # empty bit buffer!
	jmp .get_bits_pack_store_end
	#
.get_bits_pack:
	mov %edx, %edi
	sub %cl, %al # reduce bit count
	shr %edx, %cl # remove consumed bits
	shl %edx, 8 # pack bit buf
	or %dl, %al # pack bit count
.get_bits_pack_store:
	mov %r12d, %edx # store bitbuf and bit count
.get_bits_pack_store_end:
	mov %eax, %ebx # eax = result
	mov [%rbp - IN_BITS], %ebx
	
	.if GET_BIT_BACKUP_ON_STACK
	mov %edx, [%rbp - RDX_BACKUP]
	movq %rsi, [%rbp - RSI_BACKUP]
	mov %ecx, [%rbp - RCX_BACKUP]
	.else
	#
	.if GET_BIT_USE_SHIFTS
	movq %rcx, %xmm2
	movq %rsi, %xmm3
	psrlq %xmm2, 64 # rdx is stored in high quadword
	movq %rdi, %xmm4
	movq %rdx, %xmm2
	.else
	movq %rdx, %xmm1
	movq %rcx, %xmm2
	movq %rsi, %xmm3
	movq %rdi, %xmm4
	.endif
	#
	.endif
	#
.get_bits_no_bits:
	movd %ebx, %xmm7 # ebx = comp state
	# maybe called from other places ...
	cmp %dl, ASM_INFLATE_STATE_GET_BITS_BLOCK_DATA
	je .after_get_bits_block_data
	cmp %dl, ASM_INFLATE_STATE_GET_BITS_DATA_OFFSET
	je .after_get_bits_data_offset
	#
	#
	cmp %dl, ASM_INFLATE_STATE_GET_BITS_BUILD_TREES
	je .after_get_bits_build_trees
	#
	.if DADA
	cmp %dl, ASM_INFLATE_STATE_GET_BITS_LENGTH_LOOP_FIRST
	je .after_get_bits_length_loop_first
	.endif
	#
	cmp %dl, ASM_INFLATE_STATE_GET_BITS_LENGTH_LOOP
	je .after_get_bits_length_loop
	
	cmp %bl, ASM_INFLATE_STATE_READ_HEADER_AFTER
	jne .loop_start
	
	mov %edx, %eax
	jmp .read_header_after_edx
.get_bits_end:
	# now eax contains the required bits
	# jmp .loop_start
.read_header_after:
	mov %edx, [%rbp - IN_BITS]
.read_header_after_edx:
	mov [%rbp - LAST_FLAG], %edx # don't need to filter out just the lowest bit
	shr %dx, 1
	jmp .loop_start_edx
	#
	#
	#
.inflate_aio_block_data:
	#
	# mov %esi, [%rbp - OUT_AVAIL]
	xor %rcx, %rcx
	#
.block_data_loop_start:
	# decode symbol
	#
	# mov %cx, 256
	mov %rdi, %r13
	add %rdi, 384 # length tree
	mov %r14w, 1
	# int sym = inflate_decode_symbol(d, len_tree);
	jmp .inflate_aio_decode_symbol
.block_data_loop_got_symbol:
	#
	cmp %ax, 256 # end of block?
	jne .block_data_loop_not_end
	# new iteration!
	mov %edx, ASM_INFLATE_STATE_NEW_STATE
	movd %xmm7, %edx
	jmp .loop_start_edx
	#
.block_data_loop_not_end:
	test %r11, %r11
	jz .error_state_4 # no bytes available to write
	#
	cmp %ax, 256 # literal 0..255 ?
	ja .block_data_loop_not_literal
	#
	mov [%r9], %al
	inc %r9
	dec %r11d
	jmp .block_data_loop_start
.block_data_loop_not_literal:
	mov %bx, [%rdi]
	cmp %ax, %bx # sym > len_tree_max_sym
	ja .error_state_4
	mov %dx, [%rdi + 640]
	cmp %dx, ( - 1) # distance tree empty ?
	je .error_state_4
	#
	# for easier indexing
	#
	sub %ax, 257 # sym - 257
	#
	# / * Extra bits and base tables for length codes * / 
	# / / static const unsigned char length_bits[29] = {
	# / / 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 
	# / / 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 
	# / / 4, 4, 4, 4, 5, 5, 5, 5, 0};
	#
	# ax = sym, %bx = length
	#
	mov %cl, 8
	mov %bx, %ax
	add %bl, 3
	cmp %al, %cl
	ja .sym_greater8
	xor %ax, %ax #
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
	#
.sym_check:
	mov %r15d, %ebx # length
	test %ax, %ax
	jz .sym_check_skip
.sym_check_not_zero:
# here: ax = sym, bx = length
	# / * Possibly get more bits from length code * / 
	# length = inflate_getbits(d, sym, length);
	mov %dx, ASM_INFLATE_STATE_GET_BITS_BLOCK_DATA
	jmp .get_bits
.after_get_bits_block_data:
	mov %r15d, %eax # length after get bits with sym and length
	mov %ebx, %eax
.sym_check_skip:
	mov %rdi, %r13
	add %rdi, 384 + 640 # distance tree
	# int dist = inflate_decode_symbol(d, distance_tree);
	#
	mov %r14w, 2
	jmp .inflate_aio_decode_symbol
.block_data_loop_got_dt_symbol:
	
	# #if INFLATE_DO_CHECK_FOR_ERRORS
	# / * dist is within range? (dist 30, 31 are invalid) * / 
	# if (dist > 29 || dist > distance_tree - >max_sym)
	# {
	# result = INFLATE_ERROR_DATA;
	# break                      ;
	# }
	mov %rdi, %r13
	add %rdi, 384 + 640
	cmp %ax, 29
	ja .error_state_4
	mov %bx, [%rdi] # bx = max_sym
	cmp %ax, %bx # sym > max_sym ?
	jg .error_state_4 # signed comparison!
	#
# We try to compute the following tables:
	# static const unsigned char dist_bits[30] = {
	# 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 
	# 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 
	# 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
	# static const unsigned short dist_base[30] = {
	# 1, 2, 3, 4, 
	# 5, 7, 9, 13, 
	# 17, 25, 33, 49, 
	# 65, 97, 129, 193, 
	# 257, 385, 513, 769, 
	# 1025, 1537, 2049, 3073, 
	# 4097, 6145, 8193, 12289, 16385, 24577
	cmp %ax, 4
	ja .dist_ge4
	# less or equal 4
	mov %bx, %ax # bx = data offset
	xor %ax, %ax
	inc %bx
	jmp .has_data_offset
	#
.dist_ge4:
	# dist - = 4                 ;
	# sub %ax, 4
	mov %dx, %ax
	# const unsigned int o = dist & 0x1;
	# dist >>= 1                 ;
	shr %ax, 1
	# and %dx, 0x1
	# dist + +                   ;
	dec %ax # sub 2 plus 1
	# inc %ax
	#
	xor %ebx, %ebx # data offset
	#
	# data_offset = dist + 1     ;
	mov %cx, %ax
	mov %bl, 1
	inc %cx
	# data_offset = 0x1 << data_offset;
	shl %ebx, %cl
	# data_offset |= (data_offset >> o);
	bt %dx, 0 # need to perform OR with shifted values?
	jnc .dist_no_shift
	mov %edx, %ebx
	shr %edx, 1
	or %ebx, %edx
.dist_no_shift:
	# data_offset + +            ;
	inc %ebx
	xor %edx, %edx
	#
	# / * Possibly get more bits from distance code * / 
	# data_offset = inflate_getbits(d, dist, data_offset);
	mov %dx, ASM_INFLATE_STATE_GET_BITS_DATA_OFFSET
	jmp .get_bits
	#
.after_get_bits_data_offset:
.has_data_offset:
	# eax = data offset
	# check space left
	#
	# int64_t addr_dist = (d - >destination - d - >destination_start);
	# / * Possible to go back? * / 
	# if (data_offset > (int)addr_dist)
	# {
	# return INFLATE_ERROR_DATA  ; ;
	# }
	mov %ecx, %r15d # data length
	cmp %eax, 0x7FFF
	ja .error_state_4
	mov %esi, %r11d # esi = dest space avail
	movzx %rax, %ax
	mov %rbx, %r9 # rdi = dst ptr
	mov %rdx, [%rbp - DST_START]
	mov %rdi, %rbx
	cmp %rbx, %rdx
	jb .error_state_internal_2 # should be impossible
	sub %rbx, %rdx # rbx = bytes written
	cmp %rax, %rbx # out of range ?
	ja .error_state_4
	#
	# addr_dist = (d - >destination_end - d - >destination);
	# if ((int)addr_dist < length)
	# {
	# return INFLATE_ERROR_BUFFER; ;
	# }
	#
	cmp %esi, %ecx # enough space left to write?
	jb .error_state_4
	#
	#
	mov %rsi, %rdi # src = dst
	sub %rsi, %rax # src = dst - data_offset
	# mov %eax, %ecx # store count
	cld
	rep movsb # copy rcx bytes
	mov %r9, %rdi # save dst ptr
	jmp .block_data_loop_start
	#
	#
.fixed_mode_build_trees:
	call .clear_buffer
	# jmp .clear_buffer
	#
	#
.fixed_mode_build_trees_2:
	.if USE_CLEAR_REGS
	mov %al, (CLR_RAX| CLR_RBX|CLR_RCX)
	call .clear_regs
	.else
	xor %rbx, %rbx
	xor %rax, %rax
	xor %rcx, %rcx
	.endif
	#
	mov %rdi, %r13 # buffer ptr
	mov %bx, LENGTH_TREE_OFFSET
	mov %ax, 285 # max symbol
	# 64 bytes for offsets, 320 for lengths
	add %rdi, %rbx # skip area before, s.t. trees starts at this offset
	#
	mov %ecx, ((152)|(112<<16)) # 2 shorts
	shr %bx, 4 # div by 16
	mov [%rdi], %rax # store max symbol for length tree
	# mov %al, %cl
	# shr %ecx, 8
	mov [%rdi + 8 + 7 * 2], %bx
	# shr %cx, 8
	mov [%rdi + 8 + 8 * 2], %ecx
	# mov [%rdi + 8 + 9 * 2], %cl
	#
	xor %ax, %ax
	add %rdi, 40 # 8 + 16 * 2
	xor %bx, %bx
	xor %rcx, %rcx
	#
	#
	.if COMPACT_IMPL
	.if COMPACT_IMPL2
	#
	# pairs of counts&offsets packed into a single 64 - bit value
	mov %r15, ((6)|(256<<6)|(36<<16)|(2<<32)|(280<<38)|(28<<48)|(144<<54))
	mov %r14, 0x3000200010000 # offsets for the start
	mov %rsi, 0x4000400040004 # increment for every iteration
	#
.compact2_start:
	test %r15d, %r15d
	jz .compact2_end
	#
	movzx %rax, %r15w
	movzx %rcx, %r15w
	#
	.if COMPACT_IMPL_MUL
	.else
	mov %ebx, %eax
	.endif
	#
	shr %ax, 6 # offset for values
	and %cl, 0x3F # count
	#
	.if COMPACT_IMPL_MUL
	mul %rsi # result is in rdx|rax, we know that the result will be in rax
	shr %rax, 2 # we multiplied by 0x4...4...4...4, need to divide by 4
	.else
	shl %eax, 16
	or %eax, %ebx
	mov %ebx, %eax
	shl %rax, 32
	or %rax, %rbx
	.endif
	#
	add %rax, %r14 # add offsets for 1.st iteration
	
.compact2_inner:
	mov [%rdi], %rax
	add %rax, %rsi # add 0x4...4...4...4 on every iteration
	add %rdi, 8
	loopnz .compact2_inner
	shr %r15, 16
	test %r15d, %r15d
	jnz .compact2_start
.compact2_end:
	#
	.else
	#
	mov %si, 2
	mov %ebx, (24|(144<<8)|(8<<16)|(112<<24))
	mov %edx, (128|(140<<16)|(72<<24)) # need to multiply by 2
	movzx %rsi, %si
	#
	xor %ax, %ax
.compact_start:
	mov %al, %dl
	mov %cl, %bl
	shl %ax, 1 # times 2
	#
.compact_start_inner:
	mov [%rdi], %ax
	inc %ax
	add %rdi, %rsi
	loopnz .compact_start_inner
	shr %ebx, 8
	shr %edx, 8
	xor %ax, %ax
	test %bl, %bl
	jnz .compact_start
.compact_end:
	#
	.endif
	#
	.else # else for COMPACT version
	#
	xor %rax, %rax
	xor %rsi, %rsi
	mov %cl, 24
	mov %ax, 256
	mov %si, 2
	# loops
.fixed_set_1:
	mov [%rdi], %bx
	add %rdi, %rsi
	inc %ax
	loopnz .fixed_set_1
	#
	xor %ax, %ax
	mov %cl, 144
.fixed_set_2:
	mov [%rdi], %ax
	add %rdi, %rsi
	inc %ax
	loopnz .fixed_set_2
	#
	mov %ax, 280
	mov %cl, 8
.fixed_set_3:
	mov [%rdi], %ax
	add %rdi, %rsi
	inc %ax
	loopnz .fixed_set_3
	#
	mov %ax, 144
	mov %cl, 122
.fixed_set_4:
	mov [%rdi], %ax
	add %rdi, %rsi
	inc %ax
	loopnz .fixed_set_4
	#
	.endif
	#
	#
	#
	.if USE_CLEAR_REGS
	mov %al, (CLR_RAX|CLR_RCX|CLR_RSI)
	call .clear_regs
	.else
	xor %rsi, %rsi
	xor %rax, %rax
	xor %rcx, %rcx
	.endif
	mov %rdi, %r13
	mov %si, 40
	add %rdi, 384 + 640
	mov %ax, 29 # max symbol
	mov %cl, 32 # only 32 symbols
	mov [%rdi], %rax
	mov [%rdi + 8 + 5 * 2], %cx # for counts at [5]
	xor %ax, %ax
	# add %rdi, 40
	add %rdi, %rsi
	sub %si, 38 # si = 2
.fixed_set_5: # for (i = 0|i < 32|i_inc ) { symbols.us16[i] = i; }
	mov [%rdi], %ax
	inc %ax
	add %rdi, %rsi
	loopnz .fixed_set_5
	jmp .fixed_mode_process
	#
	#
.clear_buffer:
	.if USE_CLEAR_REGS
	mov %al, (CLR_RAX|CLR_RCX)
	call .clear_regs
	.else
	xor %rcx, %rcx
	xor %rax, %rax
	.endif
	#
	mov %rdi, %r13 # buffer ptr
	mov %cl, 210
	cld
	rep stosq
	#
	# Sets max symbols to - 1
	#
	mov %cx, TREE_SIZE
	# xor %rax, %rax
	mov %rdi, %r13 # buffer ptr
	dec %rax # now it's minus 1
	add %rdi, 384
	mov [%rdi], %ax
	add %rdi, %rcx
	mov [%rdi], %ax
	ret
	# .if 0
	# cmp %dl, 1
	# je .fixed_mode_build_trees_2
	# cmp %dl, 2
	# je .dyn_mode_build_trees_2
	# jmp .error_state
	# .endif
	#
	#
	
.inflate_aio_decode_symbol:
	# r14w = next step after decode symbol
	# rdi = tree start
	# eax = result
	# ebx = offset
	# int base = 0, offs = 0, len;
	.if USE_CLEAR_REGS
	mov %al, (CLR_RAX|CLR_RBX|CLR_RCX|CLR_RDX)
	call .clear_regs
	.else
	xor %rcx, %rcx
	xor %rdx, %rdx
	xor %eax, %eax
	xor %ebx, %ebx
	.endif
	#
	add %rdi, 8 # rdi = ptr to code counts
	#
	# for (len = 1               ; len < 16; + + len)
	# {
	mov %cl, 2 # len
	movq %r15, %xmm5 # src end ptr
	mov %rsi, %rdi # rsi = code counts ptr
	#
	# ebx = offset
	# eax = temp
	# edx = base
	# cx = loop counter
.decode_symbol_loop_start:
	# offs = (offs << 1) | inflate_getbits(d, 1, 0);
	shl %ebx, 1
	
	# get bits
	mov %dl, %r12b # bit count
	mov %eax, %r12d # full bit buffer
	test %dl, %dl
	jnz .get_bit_for_symbol
	# get more bits
	# mov %rsi, %r8
	xor %dx, %dx
	xor %eax, %eax
	cmp %r8, %r15
	jae .error_state # overflow
	mov %al, [%r8] # single byte
	mov %dl, 8 # 8 bits for bit buffer count
	shl %ax, 8
	inc %r8 # new src ptr
	or %ax, %dx
	# mov %r8, %rsi
	#
.get_bit_for_symbol:
	bt %eax, 8
	jnc .get8bits_for_symbol_no_bit
	inc %ebx # if lowest bit in bit buffer is set
	#
.get8bits_for_symbol_no_bit:
	shr %eax, 1 # remove 1 bit from buffer
	dec %dl
	mov %al, %dl # now 1.st byte count and rest the buffer
	# get bits end
	#
	# const int c = t - >code_counts.us16[len];
	# if (offs < c)
	# {
	# break                      ;
	# }
	# base + = c                 ;
	# offs - = c                 ;
	# shl %cl, 1 # len offset
	mov %r12d, %eax # set new bit buffer
	add %rsi, %rcx
	# xor %eax, %eax # no need, bit buffer + bit count at most 15 bits
	mov %ax, [%rsi]
	# shr %cl, 1
	cmp %ebx, %eax # offset less than code count?
	jb .decode_symbol_loop_end
	add %edx, %eax # base plus code count
	add %cl, 2
	sub %ebx, %eax # offset minus code count
	# inc %cl
	cmp %cl, 32
	jb .decode_symbol_loop_start # next iteration
	jmp .error_state # overflow!
	# }
.decode_symbol_loop_end:
	# / / assert(base + offs >= 0 && base + offs < 288);
	# base + = offs              ;
	# if (base < 0 || base > 287)
	# {
	# return 300
	# }
	add %edx, %ebx
	js .error_state
	# cmp %edx, 0
	# jl .error_state
	cmp %edx, 287
	ja .error_state
	#
	# return t - >symbols.us16[base];
	# add %rdi, 32 # rdi = symbols
	shl %dx, 1 # for shorts
	add %dx, 32 # offset for symbols
	add %rdi, %rdx
	mov %ax, [%rdi] # ax = symbol
# sym > 285 : error
	cmp %ax, 285
	ja .error_state
	# decide which way to go next
	cmp %r14b, 1
	je .block_data_loop_got_symbol
	cmp %r14b, 2
	je .block_data_loop_got_dt_symbol
	cmp %r14b, 3
	je .build_trees_after
	#
	jmp .error_state_4
	#
	#
	#
.dyn_mode_build_trees:
	# jmp .clear_buffer
	call .clear_buffer
.dyn_mode_build_trees_2:
	# Decode trees
	 / * Get 5 bits HLIT (257 - 286) * / 
	# unsigned int hlit = inflate_getbits(d, 5, 257);
	
	# / * Get 5 bits HDIST (1 - 32) * / 
	# unsigned int hdist = inflate_getbits(d, 5, 1);
	
	# / * Get 4 bits HCLEN (4 - 19) * / 
	# unsigned int hclen = inflate_getbits(d, 4, 4);
	xor %rax, %rax
	xor %rbx, %rbx
	mov %al, 14 #
	jmp .get_bits
.after_get_bits_build_trees:
	.ifdef CHECK_GET_BITS_OVERFLOW
	.if CHECK_GET_BITS_OVERFLOW
	xor %rbx, %rbx
	inc %bx
	shl %bx, 14
	cmp %rax, %rbx
	jae .error_state
	.endif
	.endif
	# ax = hlit
	# bx = hdist
	# dx = hclen
	mov %edx, %eax
	mov %ebx, %eax
	
	and %ax, 0x1F
	shr %bx, 5
	shr %dx, 10
	and %bx, 0x1F
	and %dl, 0xF
	add %ax, 257
	inc %bx
	add %dl, 4
	#
	# if (hlit > 286 || hdist > 30)
	# {
	# INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "(hlit > 286 || hdist > 30)");
	# return INFLATE_ERROR_DATA  ;
	# }
	cmp %ax, 286
	ja .error_state
	cmp %bx, 30
	ja .error_state
	#
	# Now we need to use the lengths array
	#
	#
	xor %rcx, %rcx
	mov %cx, %ax # hlit
	shl %ecx, 16
	or %ecx, %ebx # hdist
	shl %rcx, 8
	or %ecx, %edx # hclen
	#
	mov [%rbp - HLIT_HDIST_HCLEN], %ecx
	#
	
	movq %xmm0, %r14 # backup temp
	# mov %cx, %dx
	xor %rcx, %rcx
	movzx %r14, %dx
	# static const unsigned char my_clcidx[19] = {
	# 16, 17, 18, 
	# 0, 8, 7, 9, 
	# 6, 10, 5, 11, 
	# 4, 12, 3, 13, 
	# 2, 14, 1, 15}
	.if DADA
	# backjump value for get_bits
	mov %dl, ASM_INFLATE_STATE_GET_BITS_LENGTH_LOOP_FIRST
	xor %ebx, %ebx
	mov %al, 12
	jmp .get_bits
.after_get_bits_length_loop_first:
	mov %rdi, %r13
	xor %ebx, %ebx
	xor %ecx, %ecx
	add %rdi, 64 # offset for lengths in buffer
	.if 0
	mov %cl, 3
.after_get_bits_length_loop_first2:
	mov %dx, %ax
	shl %ebx, 8
	and %dx, 0x7
	shr %ax, 3
	or %ebx, %edx
	loopnz .after_get_bits_length_loop_first2
	bswap %ebx
	shr %edx, 8
	mov [%rdi + 16], %ebx
	mov [%rdi], %al
	.else
	mov %bx, %ax
	xor %edx, %edx
	mov %cx, %ax
	and %bx, 0x7 # bits 0 - 2
	and %cx, 0x1C # bits 3, 4, 5
	shr %ax, 6
	shl %cx, 5
	mov %dx, %ax
	or %bx, %cx #
	and %dx, 0x1C # bits 9, 10, 11
	and %ax, 0x7 # bits 6, 7, 8
	shl %ebx, 8
	shr %dx, 3
	# or %ax, %dx
	or %ebx, %eax #
	
	mov [%rdi + 16], %ebx
	mov [%rdi], %dl
	.endif
	mov %cl, 3
	.else
	.endif
	# pack 16 nibbles
	mov %rsi, (0|(8<<4)|(7<<8)|(9<<12)|(6<<16)|(10<<20)|(5<<24)|(11<<28)|(4<<32)|(12<<36)|(3<<40)|(13<<44)|(2<<48)|(14<<52)|(1<<56)|(15<<60))
	# backjump value for get_bits
	mov %dl, ASM_INFLATE_STATE_GET_BITS_LENGTH_LOOP
.build_trees_length_loop:
	# for (i = 0                 ; i < hclen; + + i)
	# {
	# / * Get 3 bits code length (0 - 7) * / 
	# unsigned int clen = inflate_getbits(d, 3, 0);
	xor %ebx, %ebx
	mov %al, 3
	jmp .get_bits
.after_get_bits_length_loop:
	mov %rdi, %r13
	xor %rbx, %rbx
	add %rdi, 64 # offset for lengths in buffer
	.if DADA
	.else
	cmp %cx, 3
	jae .length_loop_i_ge3
	mov %bx, %cx
	add %bx, 16 # 16, 17, 18
	jmp .length_loop_has_index
.length_loop_i_ge3:
	.endif
	# get the right nibble
	mov %rbx, %rsi
	shl %cx, 3
	shr %rbx, %cl
	shr %cx, 3
	and %bx, 0xF # single nibble
	movzx %rbx, %bx
	#
	# lengths[my_clcidx[i]] = (unsigned char)clen;
	# }
.length_loop_has_index:
	# or %bl, 64 # add offset for lengths
	add %rbx, %rdi
	mov [%rbx], %al # store the byte
	inc %cx
	cmp %cx, %r14w
	jb .build_trees_length_loop
	# loopnz .build_trees_length_loop
	#
	movq %r14, %xmm0
	xor %edx, %edx
	# restore values
	mov %ecx, [%rbp - HLIT_HDIST_HCLEN]
	xor %ebx, %ebx
	xor %eax, %eax
	# mov %dl, %cl # hclen
	shr %ecx, 8
	mov %bl, %cl # hdist
	shr %ecx, 8
	mov %ax, %cx # hlit
	#
	# Build length tree
	mov %rdi, %r13
	xor %rcx, %rcx
	mov %rsi, %rdi
	add %rdi, 384 # ptr to tree
	add %rsi, 64 # ptr to length
	mov %cx, 19 # length size
	#
	call .build_tree
	#
	test %eax, %eax
	jnz .error_state
	#
	#
	mov %rdi, %r13
	mov %rsi, %rdi
	add %rdi, 384
	mov %ax, [%rdi]
	cmp %ax, 0
	jl .error_state
	#
	#
	# / * Decode code lengths for the dynamic trees * / 
	#
	mov %ecx, [%rbp - HLIT_HDIST_HCLEN]
	mov %eax, %ecx
	shr %ecx, 8
	shr %eax, 16
	and %cx, 0xFF
	movzx %ecx, %cl
	add %ecx, %eax # ecx = hlit + hdist
	
	xor %rdx, %rdx # dx = num
	#
	# for (num = 0               ; num < hlit + hdist;)
	# {
.loop_num_hlit_hdist:
	# int sym = inflate_decode_symbol(d, lt);
	mov %rdi, %r13
	# mov %rsi, %rdi
	mov %r14w, 3 # state after
	add %rdi, 384 # length tree
	jmp .inflate_aio_decode_symbol
.build_trees_after:
	# eax = result
	# #if INFLATE_DO_CHECK_FOR_ERRORS
	# if (sym > lt_max_sym)
	# {
	# return INFLATE_ERROR_DATA  ;
	# }
	# #endif
	mov %rdi, %r13
	add %rdi, 384 # length tree
	mov %ebx, [%rdi]
	cmp %eax, %ebx
	ja .error_state
	#
	# unsigned int length = 1    ;
	xor %ebx, %ebx
	
	inc %bx # length 1
# default:
	# / * Values 0 - 15 represent the actual code lengths * / 
	# / / length = 1             ;
	# break                      ;
	# }
	#
	# switch (sym)
	# {
	cmp %ax, 16
	jne .sym_not_16
# case 16:
	# / * Copy previous code length 3 - 6 times (read 2 bits) * / 
	# #if INFLATE_DO_CHECK_FOR_ERRORS
	# if (num == 0)
	# {
	# return INFLATE_ERROR_DATA  ;
	# }
	# #endif
	test %dx, %dx
	jz .error_state
	#
	#
	mov %rdi, %r13
	add %rdi, 64 # length array
	#
	
	dec %dx
	xor %ax, %ax
	add %rdi, %rdx
	inc %dx
	# sym = lengths[num - 1]     ;
	mov %al, [%rdi]
	# length = inflate_getbits(d, 2, 3);
	mov %bx, 0x203
	# break                      ;
	jmp .sym_switch_end
.sym_not_16:
	cmp %ax, 17
	je .sym_is_17
	cmp %ax, 18
	je .sym_is_18
.sym_is_17:
# case 17:
	# / * Repeat code length 0 for 3 - 10 times (read 3 bits) * / 
	# sym = 0                    ;
	xor %ax, %ax
	# length = inflate_getbits(d, 3, 3);
	mov %bx, 0x303
	# break
	jmp .sym_switch_end
.sym_is_18:
# case 18:
	# / * Repeat code length 0 for 11 - 138 times (read 7 bits) * / 
	# sym = 0                    ;
	xor %ax, %ax
	# length = inflate_getbits(d, 7, 11);
	mov %bx, 0x70B
	# break
	#
.sym_switch_end:
	mov %r14w, %ax # store sym
	shl %r14d, 16
	or %r14d, %edx # count
	
	cmp %bx, 1
	je .dont_compute_length
	mov %ax, %bx
	and %bx, 0xF
	shr %ax, 8
	#
	mov %dl, 100
	jmp .get_bits
.build_after_get_bits:
	mov %bx, %ax # bx = length
.dont_compute_length:
	#
	# #if INFLATE_DO_CHECK_FOR_ERRORS
	# if (length > hlit + hdist - num)
	# {
	# return INFLATE_ERROR_DATA  ;
	# }
	# #endif
	#
	mov %dx, %r14w # restore count
	sub %cx, %dx
	cmp %bx, %cx
	jg .error_state
	#
	add %cx, %dx
	mov %eax, %r14d
	shr %eax, 16 # al = sym
	#
	#
	mov %rdi, %r13
	add %rdi, 64 # length array
	add %rdi, %rdx
	xchg %cx, %bx # cx = length
	# while (length - - != 0)
	# {
	# lengths[num + + ] = sym    ;
	# }
	cld
	rep stosb
	#
	xchg %cx, %bx # cx = limit
	#
	inc %dx # for next loop iteration
	#
	cmp %dx, %cx
	jb .loop_num_hlit_hdist
	# }
	#
	mov %rdi, %r13
	mov %rsi, %r13
	
	# Is end_of_block present ?
	add %rsi, (64 + 256)
	mov %al, [%rsi]
	test %eax, %eax
	jz .error_state
	#
	mov %ecx, [%rbp - HLIT_HDIST_HCLEN]
	sub %rsi, 256 # length ptr
	shr %ecx, 16 # cx = hlit
	add %rdi, 384 # rdi = len tree ptr
	#
	call .build_tree
	#
	test %eax, %eax
	jnz .error_state
	#
	xor %rax, %rax
	mov %rdi, %r13
	mov %eax, [%rbp - HLIT_HDIST_HCLEN]
	xor %rcx, %rcx
	mov %rsi, %rdi
	mov %cl, %ah # cl = ah = hdist
	add %rdi, (384 + 640) # distance tree
	shr %eax, 16 # ax = hlit
	add %rsi, %rax # length + hlit
	#
	call .build_tree
	#
	test %eax, %eax
	jnz .error_state
	#
	jmp .inflate_aio_block_data
	#
	#
	# jmp .error_state
	# mov %edx, DYN_TREE_SEQUENCE
	# movd %xmm7, %edx
	# jmp .loop_start
	# jmp .error_state
	#
.build_tree:
	# rdi = tree ptr
	# rsi = length array ptr
	# cx = length of length array
	ret
	#
	#
	.if USE_CLEAR_REGS
.clear_regs: # used to clear regs
	.ifdef CLR_RDI
	bt %ax, 5
	jnc .clear_regs_no_rdi
	xor %rdi, %rdi
.clear_regs_no_rdi:
	.endif
.clear_regs_rsi:
	bt %ax, 4
	jnc .clear_regs_no_rsi
	xor %rsi, %rsi
.clear_regs_no_rsi:
	bt %ax, 3
	jnc .clear_regs_no_rdx
	xor %rdx, %rdx
.clear_regs_no_rdx:
	bt %ax, 2
	jnc .clear_regs_no_rcx
	xor %rcx, %rcx
.clear_regs_no_rcx:
	bt %ax, 1
	jnc .clear_regs_no_rbx
	xor %rbx, %rbx
.clear_regs_no_rbx:
	# bt %ax, 0
	# jnc .clear_regs_no_rax
	xor %rax, %rax
.clear_regs_no_rax:
	ret
	#
	.endif
	#
	#
.loop_end:
	#
	# return values
	#
.error_state:
.error_state_4:
	mov %al, 4
	jmp .restore_and_return
.error_state_no_bits:
	mov %al, 3
	jmp .restore_and_return
.error_state_invalid_params:
	mov %al, 6
	jmp .restore_and_return
.error_state_internal_1:
.error_state_internal_2:
	mov %al, 9
	jmp .restore_and_return
	# mov %al, 10
	# jmp .restore_and_return
	#
	.if INFLATE_ASM_RANGE_CHECK
.error_state_not_supported_range:
	mov %al, 8
	jmp .restore_and_return
	.endif
	#
.error_state_not_implemented_mmx:
	mov %al, 7
	jmp .restore_and_return
.error_state_not_implemented:
	mov %al, 5
	jmp .restore_and_return
.ok_state:
	xor %eax, %eax
.restore_and_return:
	movzx %eax, %al
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
	mov %eax, NEW_STATE_JUMP
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
	
	#
	# .ifdef RAW_MODE_COPY_8
	# .if RAW_MODE_COPY_8
	# mov %ebx, %r8d # ebx = src ptr
	
	#
	# cmp %cx, 16
	# jbe .raw_mode_copy_bytes
	#
	# or %bx, %di # mix bits
	# test %cx, 0x7 # 8 - byte - aligned?
	# jnz .raw_mode_copy_bytes
	# alignment checks, maybe we can copy larger chunks per "rep movsX" - iteration
	# test %bx, 0x7 # 8 - byte - aligned si|di ?
	# jnz .raw_mode_copy_bytes
	# shr %ecx, 3 # i.e div by 8
	# cld # forward copy!
	# repnz movsq
	# jmp .raw_mode_copy_done
	#
	# .endif
	# .endif
