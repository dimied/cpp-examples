	#
	#
	# ## CONSTANTS ##
	.equ ASM_INFLATE_STATE_RAW, 0
	.equ ASM_INFLATE_STATE_FIXED, 1
	.equ ASM_INFLATE_STATE_DYN, 2
	#
	.equ ASM_INFLATE_STATE_ERROR, 3
	#
	#
	# STACK POSITIONS
	.equ RSP_BACKUP, 8
	# some space in - between
	.equ IN_SIZE, 20
	.equ OUT_SIZE, 24
	# .equ IN_AVAIL, 28
	# .equ OUT_AVAIL, 32
	#
	.equ IN_DATA, 40
	.equ OUT_DATA, 48
	#
	.equ SRC_START, 56
	.equ DST_START, 64
	#
	.equ FIRST_TO_ZERO_POS, 72
	.equ LAST_FLAG, (FIRST_TO_ZERO_POS)
	# .equ IN_BITS, (FIRST_TO_ZERO_POS + 8)
	.equ HLIT_HDIST_HCLEN, (FIRST_TO_ZERO_POS + 8)
	# .equ RAX_BACKUP, (FIRST_TO_ZERO_POS + 8)
	# .equ RBX_BACKUP, (FIRST_TO_ZERO_POS + 16)
	.equ RCX_BACKUP, (FIRST_TO_ZERO_POS + 16)
	.equ RDX_BACKUP, (FIRST_TO_ZERO_POS + 24)
	.equ RDI_BACKUP, (FIRST_TO_ZERO_POS + 32)
	.equ RSI_BACKUP, (FIRST_TO_ZERO_POS + 40)
	#
	# 3 short values, 4.th value is sum
	# .equ LAST_TO_ZERO, (HLIT_HDIST_HCLEN)
	.equ LAST_TO_ZERO, (RSI_BACKUP)
	# .equ NUM_TO_ZERO, (LAST_TO_ZERO - FIRST_TO_ZERO_POS)
	.equ NUM_TO_ZERO, (LAST_TO_ZERO)
	.equ NUM_TO_ZERO_8, (NUM_TO_ZERO / 8)
	#
	.equ OFFSETS_LEN, 32
	.equ LENGTH_ARRAY_OFFSET, 64
	.equ LENGTH_ARRAY_SIZE, 320
	.equ LENGTH_TREE_OFFSET, 384
	.equ TREE_SIZE, 640
	.equ DISTANCE_TREE_OFFSET, (LENGTH_TREE_OFFSET + TREE_SIZE)
	.equ BUFFER_LEN, (LENGTH_TREE_OFFSET + (2 * TREE_SIZE))
	.equ BUFFER_LEN_8, (BUFFER_LEN) / 8
	#
	#
	#
	.section .text
	#
	.globl inflate_asm_x64
	.globl inflate_asm_x64_diffs
	#
	.type inflate_asm_x64, @function
	.type inflate_asm_x64_diffs, @function
	#
	# Parameters
	# xDI = 1st, xSI = 2nd, xDX = 3rd, xCX = 4th, r8 = 5th
	#
	# Register Preservation Rules for Linux
# Caller - Saved (Volatile) Registers:
	# - %rax, %rcx, %rdx, %rsi, %rdi, %r8, %r9, %r10, %r11
	# - (can be overwritten by the callee).
# Callee - Saved (Non - Volatile) Registers:
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
# - %r14 : decode_sym temp
# - %r15 : temp reg
	#
	# And we can use also XMM registers (XMM0 - XMM7), 
	# because all 64 - bit processors support XMM0.
	# And we don't need to deal with MMX - FP87 aliasing
# - xmm0 : temp
# - xmm0 : block data temp
	#
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
	# .equ RAW_JUMP, (.raw_mode - .inflate_aio_jmp_start_3)
	# .equ FIXED_JUMP, (.fixed_mode - .inflate_aio_jmp_start_3)
	# .equ DYN_JUMP, (.dyn_mode - .inflate_aio_jmp_start_3)
	# .equ NEW_STATE_JUMP, (.new_state - .inflate_aio_jmp_start_3)
	# .equ MODE_ADDRS, (RAW_JUMP | (FIXED_JUMP<<8) | (DYN_JUMP<<16))
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
	#
	#
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
	# for raw values
	.ifndef CHECK_RAW_SIZES
	.equ CHECK_RAW_SIZES, 1
	.endif
	# algo prop for fixed
	.ifndef COMPACT_IMPL
	.equ COMPACT_IMPL, 1
	.endif
	# algo prop for fixed
	.ifndef COMPACT_IMPL2
	.equ COMPACT_IMPL2, 0
	.endif
	# algo prop for fixed
	.ifndef COMPACT_IMPL_MUL
	.equ COMPACT_IMPL_MUL, 0
	.endif
	.ifndef FIXED_LOOP_IMPL
	.equ FIXED_LOOP_IMPL, 0
	.endif
	# use functions to clear registers?
	.ifndef USE_CLEAR_REGS
	.equ USE_CLEAR_REGS, 1
	.endif
	#
	#
	.ifndef GET_BITS_BACKUP_ON_STACK
	.equ GET_BITS_BACKUP_ON_STACK, 0
	.endif
	#
	.ifndef GET_BITS_64B_STORE
	.equ GET_BITS_64B_STORE, 1
	.endif
	#
	#
	xor %rax, %rax
	#
	.if INFLATE_ASM_STACK_PTR_CHECK
	mov %ax, 512
	# unrealistic stack having so small stack address, 
	cmp %rsp, %rax
	# but still ...
	ja .inflate_asm_x64_enough_stack
	shr %ax, 6 # div by 64. i.e. ax = 8
	ret
.inflate_asm_x64_enough_stack:
	.endif
	#
	.equ INFLATE_ASM_CHECK_SSSE3, 0
	
	.if INFLATE_ASM_CHECK_SSE2
	.equ INFLATE_ASM_CHECK_CPUID, 1
	.else
	.if INFLATE_ASM_CHECK_SSSE3
	.equ INFLATE_ASM_CHECK_CPUID, 1
	.else
	.equ INFLATE_ASM_CHECK_CPUID, 0
	.endif
	.endif
	#
	# SSE2 ?
	.if INFLATE_ASM_CHECK_CPUID
	#
	push %rdx
	push %rcx
	mov %ax, 1
	cpuid
# edx : 4 TSC
# edx : 8 CMPXCHG8B
# edx : 15 CMOV
# edx : 19 CLFLUSH
# edx : 25 SSE
# edx : 26 SSE2
# ecx : 0 SSE3
# ecx : 9 SSSE3 (e.g. PSHUFB)
# ecx : 13 CMPXCHG16B
# ecx : 19 SSE4.1
# ecx : 20 SSE4.2
# ecx : 23 POPCNT
# ecx : 28 AVX
	.if INFLATE_ASM_CHECK_SSSE3
	mov %eax, %edx
	shr %eax, 26
	shl %ecx, 1
	or %eax, %ecx
	pop %rcx
	pop %rdx
	bt %eax, 0 # 26.th bit for SSE2 - support
	jc .inflate_sse2_supported
	xor %rax, %rax
	mov %al, 7
	ret
	.else
	mov %eax, %edx # clears upper bits
	pop %rcx
	pop %rdx
	bt %eax, 26 # 26.th bit for SSE2 - support
	jc .inflate_sse2_supported
	# xor %rax, %rax
	mov %al, 7
	ret
	.endif
	#
.inflate_sse2_supported:
	.endif
	#
	#
	push %rbp # callee saved
	#
	push %rbx # callee saved
	push %r11 # callee saved
	#
	push %r12 # callee saved
	push %r13 # callee saved
	#
	xor %rax, %rax
	#
	push %r14 # callee saved
	push %r15 # callee saved
	# 2 zeros to check later
	# whether something was called or jumped to
	#
	push %rax
	push %rax
	#
	#
	mov %rbp, %rsp
	#
	# Now stack is 16 - byte aligned
	#
	#
	# Validate inputs
	#
	test %rdi, %rdi # dst addr
	jz .error_state_invalid_params2
	test %rdx, %rdx # src addr
	jz .error_state_invalid_params2
	test %ecx, %ecx # in size
	jz .error_state_invalid_params2
	test %esi, %esi # out size
	jz .error_state_invalid_params2
	test %r8, %r8 # valid buffer ptr
	jz .error_state_invalid_params2
	#
	jmp .inputs_not_zero
	
.error_state_invalid_params2: #for shorter jumps
	jmp .error_state_invalid_params
	#
	#
.inputs_not_zero:
	# Make some room on stack, just for the case ...
	# this also also allows calls to functions
	# without adjusting rsp before calls
	# xchg %rcx, %rbx
	.if 0
	mov [%rbp - 8], %rax
	mov [%rbp - 16], %rax
	mov [%rbp - 24], %rax
	mov [%rbp - 32], %rax
	mov [%rbp - 32], %rax
	mov [%rbp - 40], %rax
	mov [%rbp - 48], %rax
	mov [%rbp - 56], %rax
	sub %rbp, 64
	.else
	xchg %rcx, %rbx
	xor %rcx, %rcx
	mov %cl, 8
	sub %rbp, %rcx
	#
.lala1:
	mov [%rbp], %rax
	sub %rbp, 8
	loopnz .lala1
	#
	xchg %rcx, %rbx
	.endif
	#
	# Limit ranges to 2^32 - 1
	#
	xor %rbx, %rbx
	.if INFLATE_ASM_STACK_64B_ALIGNED
	# make our 'private stack'(=rbp) be 64 - byte aligned
	mov %rax, %rbp
	.endif
	# dec %rbx # minus 1, i.e. all bits sets
	dec %ebx
	.if INFLATE_ASM_STACK_64B_ALIGNED
	and %ax, 63
	.endif
	shr %rbx, 32 # rbx should be now 2^32 - 1
	# mov %ebx, %ebx # zero high 32 bits
	#
	.if INFLATE_ASM_STACK_64B_ALIGNED
	movzx %rax, %ax
	.endif
	#
	and %rcx, %rbx # in count
	and %rsi, %rbx # out count
	#
	.if INFLATE_ASM_STACK_64B_ALIGNED
	test %ax, %ax
	jz .inputs_on_stack_aligned # only 0 bits
	sub %rbp, %rax # adjust
.inputs_on_stack_aligned:
	.endif
	#
	#
	# Validate ranges | paranoia ; - ) |
	# We store end pointers to make range check shorter
	# I.e. if addr + size >= end of address space, then it's an error
	# We also check for any overlaps with the stack space
	#
	.if INFLATE_ASM_NULLIFY_STACK
	mov %r14, %rcx # backup in size
	xor %rax, %rax
	xor %rcx, %rcx
	mov %r15, %rdi # backup dest ptr
	.if NUM_TO_ZERO_8 < 128
	mov %cl, NUM_TO_ZERO_8
	.else
	mov %cx, NUM_TO_ZERO_8
	.endif
	lea %rdi, [%rbp - 8]
	# mov %rdi, %rbp
	# sub %rdi, %rcx # rdi = ptr to low end of out stack
	# shr %cl, 3 # we clear 8 bytes at once
	#
	std # decrement rdi
	rep stosq
	#
	mov %rcx, %r14 # restore in size
	mov %rdi, %r15 # restore dest ptr
	#
	.else
	#
	xor %rax, %rax
	#
	.endif
	#
	#
.inputs_on_stack:
	#
	.if INFLATE_ASM_RANGE_CHECK
	.equ INFLATE_ASM_RANGE_ERROR, 1
	.endif
	#
	.if INFLATE_STACK_OVERLAP_CHECK
	.ifndef INFLATE_ASM_RANGE_ERROR
	.equ INFLATE_ASM_RANGE_ERROR, 1
	.endif
	.endif
	#
	.if INFLATE_STACK_OVERLAP_CHECK
	# mov %rbx, %rbp
	# sub %rbx, 256
	lea %rbx, [%rbp - 256]
	.endif
	#
	#
	mov %rax, %rdx # src
	add %rax, %rcx # unsigned add, i.e. carry flag for unsigned overflow
	#
	.if INFLATE_ASM_RANGE_CHECK
	jc .error_state_not_supported_range2
	.endif
	#
	.if INFLATE_STACK_OVERLAP_CHECK
	cmp %rdx, %rsp #
	ja .stack_below_src
	cmp %rax, %rbx # data overlaps with stack?
	ja .error_state_not_supported_range2 # overlap with the stack?
.stack_below_src:
	.endif
	#
	movq %xmm5, %rax # input end ptr
	#
	mov %rax, %rdi # dest
	add %rax, %rsi # unsigned add, i.e. carry flag for unsigned overflow
	#
	.if INFLATE_ASM_RANGE_CHECK
	jc .error_state_not_supported_range2
	.endif
	#
	.if INFLATE_STACK_OVERLAP_CHECK
	cmp %rdi, %rsp
	ja .stack_below_dest
	cmp %rax, %rbx # data overlaps with stack?
	ja .error_state_not_supported_range2 # overlap with the stack?
.stack_below_dest:
	.endif
	#
	movq %xmm6, %rax # output end ptr
	# store rsp to be able to
	# jump out of functions to error clauses
	mov [%rbp - RSP_BACKUP], %rsp
	#
	jmp .store_inputs
	#
	.if INFLATE_ASM_RANGE_ERROR
.error_state_not_supported_range2:
	jmp .error_state_not_supported_range # longer jump to real error
	.endif
	#
	#
.inflate_update_avail_error:
	mov %rsp, [%rbp - RSP_BACKUP]
	jmp .error_state
	#
	#
.inflate_update_avail_values:
	#
	movq %xmm4, %rax
	#
	movq %rax, %xmm5
	sub %rax, %r8
	jc .inflate_update_avail_error
	mov %r10, %rax # in bytes available
	#
	movq %rax, %xmm6
	sub %rax, %r9
	jc .inflate_update_avail_error
	mov %r11, %rax # out bytes available
	#
	movq %rax, %xmm4
	#
	ret
	#
	#
.store_inputs:
	#
	mov %ecx, %ecx
	mov %esi, %esi
	#
	mov %r13, %r8 # ptr to buffer
	#
	mov [%rbp - IN_SIZE], %ecx
	mov [%rbp - OUT_SIZE], %esi
	# mov [%rbp - IN_AVAIL], %ecx
	# mov [%rbp - OUT_AVAIL], %esi
	#
	mov [%rbp - IN_DATA], %rdx # ptr to in data
	mov [%rbp - OUT_DATA], %rdi # ptr to out data
	xor %r12, %r12 # clear bit buffer
	#
	mov [%rbp - SRC_START], %rdx # ptr to in data
	mov [%rbp - DST_START], %rdi # backup for dest
	#
	mov %r8, %rdx # src data ptr
	mov %r9, %rdi # dst data ptr
	#
	mov %r10, %rcx # in - avail
	mov %r11, %rsi # out - avail
	#
	#
	# set all variables to 0
	.if USE_CLEAR_REGS
	# mov %al, (CLR_RAX|CLR_RBX|CLR_RCX|CLR_RDX)
	call .clear_base_regs
	.else
	xor %rax, %rax
	xor %rbx, %rbx
	xor %rcx, %rcx
	xor %rdx, %rdx
	.endif
	#
	# No real need to clear xmm registers here.
	#
	.if 0
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
	.endif
	#
.algo_start:
	#
.loop_start_edx:
	#
	#
	.if 0
	xor %r10, %r10
	xor %r11, %r11
	#
	movq %rax, %xmm5 # src end ptr
	movq %rbx, %xmm6 # dest end ptr
	#
	sub %rax, %r8
	.if INFLATE_ASM_CHECK_ERRORS
	jc .error_at_loop_start
	.endif
	sub %rbx, %r9
	.if INFLATE_ASM_CHECK_ERRORS
	jc .error_at_loop_start
	.endif
	#
	mov %r10d, %eax # in_bytes left
	# mov [%rbp - IN_AVAIL], %eax # stack backup
	mov %r11d, %ebx # out_bytes avail
	# mov [%rbp - OUT_AVAIL], %ebx # stack backup
	.else
	call .inflate_update_avail_values
	.endif
	#
	# And we process the command
	#
.new_state:
	mov %ax, [%rbp - LAST_FLAG]
	bt %ax, 0 # test 0.th bit, if bit set, then carry flag also set
	jc .ok_state # then we just processed the last block
	#
	call .inflate_update_avail_values
	#
	xor %rbx, %rbx
	mov %al, 3
	# set next command after get_bits
	call .get_bits_func
	#
	mov [%rbp - LAST_FLAG], %al # don't need to filter out just the lowest bit
	xor %rbx, %rbx
	shr %al, 1
	xor %rcx, %rcx
	#
	cmp %al, ASM_INFLATE_STATE_RAW
	je .raw_mode
	cmp %al, ASM_INFLATE_STATE_FIXED
	je .fixed_mode
	cmp %al, ASM_INFLATE_STATE_DYN
	je .dyn_mode
	#
.inflate_error1:
	jmp .error_state
	#
	#
.raw_mode:
	#
	mov %rsi, %r8 # rsi = src ptr
	mov %cl, 4
	xor %r12, %r12 # reset bit buffer
	#
	sub %r10d, %ecx # in_avail minus 4
	.if INFLATE_ASM_CHECK_ERRORS
	jc .inflate_error1
	.endif
	cld
	lodsd # rsi also incremented here
	#
	mov %ecx, %eax
	#
	.if CHECK_RAW_SIZES
	# simple validity check, low 16 bits =?= high 16 bits negated
	not %eax
	movzx %ecx, %cx
	shr %eax, 16
	#
	cmp %cx, %ax # equal high / low 16 - bits ?
	jne .inflate_error1
	#
	.else
	movzx %ecx, %cx
	#
	.endif
	#
	mov %rdi, %r9 # rdi = dest ptr
	#
	# jecxz .raw_mode_jmp_new_state # can't jump too far ...
	#
	# no jump to main loop, can adjust in / out available counts here
	.if INFLATE_ASM_CHECK_ERRORS
	cmp %r10d, %ecx # enough input data ?
	jb .inflate_error1 # underflow?
	.endif
	#
	.if INFLATE_ASM_CHECK_ERRORS
	cmp %r11d, %ecx # enough output space ?
	jb .inflate_error1 # underflow?
	.endif
	#
.raw_mode_copy_bytes:
	# call .copy_bytes
	# cld # forward copy! already set
	rep movsb
.raw_mode_copy_done:
	mov %r8, %rsi # update src ptr
	mov %r9, %rdi # update dest ptr
.raw_mode_jmp_new_state:
	jmp .new_state
.raw_mode_end:
	#
	#
	#
.fixed_mode:
.fixed_mode_build_trees:
	call .clear_buffer
	#
	.if USE_CLEAR_REGS
	# mov %al, (CLR_RAX|CLR_RBX|CLR_RCX)
	call .clear_base_regs
	.else
	xor %rax, %rax
	xor %rbx, %rbx
	# mov %ebx, %eax
	xor %rcx, %rcx
	# mov %ecx, %eax
	.endif
	#
	cld
	lea %rdi, [%r13 + LENGTH_TREE_OFFSET]
	mov %ax, 285 # max symbol
	stosq
	# rdi = code counts
	mov %eax, ((152)|(112<<16)) # 2 shorts
	add %rdi, 16 # offset for code count 8, 9
	mov %bl, 24
	#
	stosd # rdi = + 20 after
	#
	mov [%rdi - 6], %bx # code count
	#
	add %rdi, 12 # rdi = symbols
	#
	#
.fixed_mode_compact_start:
	.if COMPACT_IMPL
	# xor %ax, %ax
	#
	# pairs of counts&offsets packed
	#
	# counters
	mov %ebx, (24|(144<<8)|(8<<16)|(112<<24))
	# base values, need to multiply by 2
	# mov %edx, (128|(140<<16)|(72<<24))
	mov %rax, (256|(280<<32)|(144<<48))
	#
	# cld
	#
.fixed_mode_compact_loop_start:
	# movzx %eax, %dl # to avoid xor ax, ax
	# mov %ax, %dx # to avoid xor ax, ax
	mov %cl, %bl
	# shl %ax, 1 # times 2
	#
.fixed_mode_compact_loop_inner:
	stosw # increments rdi by 2
	inc %ax
	loopnz .fixed_mode_compact_loop_inner
	shr %ebx, 8
	# shr %edx, 8
	# shr %rdx, 16
	shr %rax, 16
	test %bl, %bl
	jnz .fixed_mode_compact_loop_start
	#
	.endif
.fixed_mode_compact_end:
	#
	#
.fixed_mode_compact2_start:
	.if COMPACT_IMPL2
	# xor %eax, %eax
	# xor %rcx, %rcx
	mov %r15, ((6)|(256<<6)|(36<<16)|(2<<32)|(280<<38)|(28<<48)|(144<<54))
	mov %r14, 0x3000200010000 # offsets for the start
	mov %rsi, 0x4000400040004 # increment for every iteration
	#
.fixed_mode_compact2_loop_start:
	test %r15d, %r15d
	jz .fixed_mode_compact2_end
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
	
.fixed_mode_compact2_inner:
	# mov [%rdi], %rax
	stosq
	add %rax, %rsi # add 0x4...4...4...4 on every iteration
	# add %rdi, 8
	loopnz .fixed_mode_compact2_inner
	shr %r15, 16
	test %r15d, %r15d
	jnz .fixed_mode_compact2_loop_start
	.endif
.fixed_mode_compact2_end:
	#
	#
.fixed_mode_fixed_set_start:
	.if FIXED_LOOP_IMPL # else for COMPACT version
	#
	xor %rax, %rax
	# xor %rbx, %rbx
	mov %cl, 24
	mov %ax, 256
	# mov %bl, 2
	cld
	# loops
.fixed_set_1:
	stosw
	inc %ax
	loopnz .fixed_set_1
	#
	xor %ax, %ax
	mov %cl, 144
.fixed_set_2:
	stosw
	inc %ax
	loopnz .fixed_set_2
	#
	mov %ax, 280
	mov %cl, 8
.fixed_set_3:
	stosw
	inc %ax
	loopnz .fixed_set_3
	#
	mov %ax, 144
	mov %cl, 122
.fixed_set_4:
	stosw
	inc %ax
	loopnz .fixed_set_4
	#
	.endif
.fixed_mode_fixed_set_end:
	#
	# Now the distance tree
	#
	# xor %rax, %rax
	mov %eax, %eax
	xor %cx, %cx
	#
	mov %al, 29 # max symbol
	.if 1
	lea %rdi, [%r13 + LENGTH_TREE_OFFSET + TREE_SIZE]
	mov %cl, 32 # only 32 symbols
	.else
	mov %cx, 1024 # only 32 symbols
	lea %rdi, [%r13 + %rcx]
	shr %cx, 5
	.endif
	stosq # set max sym
	#
	mov [%rdi + 10], %cl # for counts at [5]
	#
	add %rdi, 32 # %rbx
	#
	# for (i = 0|i < 32|i_inc ) { symbols.us16[i] = i; }
	#
	.if 1
	xor %al, %al
.fixed_set_5:
	stosw # we save words
	inc %al
	loopnz .fixed_set_5
	#
	.else
	#
	shr %cl, 3
	mov %rax, 0x3000200010000
	mov %rbx, 0x1000100010001
.fixed_set_5b:
	stosq
	add %rax, %rbx
	loopnz .fixed_set_5b
	.endif
	#
	#
	jmp .inflate_aio_block_data
.fixed_mode_build_trees_end:
	#
.dyn_mode:
	nop
	#
	jmp .error_state_not_implemented
	#
	#
	#
# eax : #bits, %ebx: offset
	.equ GB_CHECK_0, 1
	#
.get_bits_func:
	movzx %rbx, %bx
	#
	bts %r12, 63 # mark buffer to use offset
	jmp ._get_bits_func_with_base
	#
.get_bits_func_no_base:
	btr %r12, 63 # mark buffer to use no offset
._get_bits_func_with_base:
	#
	movzx %rax, %ax
	#
	.if GB_CHECK_0
	# any bits requested ?
	test %ax, %ax
	jnz .get_bits_has_bits
	#
	btr %r12, 63
	jnc ._get_bits_no_bits
	mov %rax, %rbx # plus offset
._get_bits_no_bits:
	ret
	.endif
	#
.get_bits_has_bits:
	# We only have 57 bits in bit buffer
	# 1 bit for param, 6 bits for length, and the buffer
	cmp %ax, 57
	ja .get_bits_error
	# jbe .get_bits_backup_regs
.get_bits_backup_regs:
	# backup values
	#
	.if GET_BITS_BACKUP_ON_STACK
	mov [%rbp - RDX_BACKUP], %rdx
	mov [%rbp - RCX_BACKUP], %rcx
	.else
	movq %xmm2, %rdx
	movq %xmm1, %rcx
	#
	.endif
	#
	mov %rdx, %r12 # packed bitbuf and count
	#
	.if GET_BITS_BACKUP_ON_STACK
	mov [%rbp - RDI_BACKUP], %rdi
	mov [%rbp - RSI_BACKUP], %rsi
	.else
	movq %xmm3, %rsi
	movq %xmm4, %rdi
	#
	.endif
	#
	btr %rdx, 63 # remove offset bit
	# jmp .get_bits_backup_regs_end
	
	#
.get_bits_backup_regs_end:
	#
	#
	movzx %rcx, %dl # cl = dl, i.e. cl = bit count
	#
	shr %rdx, 6 # now rdx contains the real bits
	#
	and %cl, 0x3F # only 6 bits for buffer length
	#
	cmp %al, %cl # enough bits in the bit buffer?
	jbe .get_bits_enough
	#
	call .inflate_update_avail_values
	#
	# fill more bits!
	#
	mov %rdi, %rbx # backup rbx
	# bl = num of requested bits
	movzx %rbx, %al
	jmp .get_bits_get_more_loop
.get_bits_error:
	# we jump directly to error clause
	# to avoid any error checks in calling functions
	mov %rsp, [%rbp - RSP_BACKUP]
	# jmp .error_state_no_bits
	mov %al, 3
	jmp .restore_and_return
	#
.get_bits_get_more_loop:
	# test %r10d, %r10d # enough available
	# jz .get_bits_error
	#
	xor %rax, %rax
	#
	dec %r10d # avail - 1
	js .get_bits_error # underflow ?
	#
	mov %al, [%r8] # single byte
	# lodsb
	#
	shl %rax, %cl # shift
	#
	inc %r8 # next addr
	# inc %rsi
	add %cl, 8 # inc bit buf count
	#
	jc .get_bits_error # should never happen
	#
	or %rdx, %rax # prepend the bit buffer
	#
	cmp %bl, %cl # enough in the bit buffer ?
	ja .get_bits_get_more_loop # repeat if not
	#
	movzx %rax, %bl # al = requested num of bits
	# mov %r8, %rsi
	mov %rbx, %rdi # restore rbx
	#
	#
.get_bits_enough:
	#
	xor %rsi, %rsi
	mov %rdi, %rdx # rdi = bit buf
	#
	# cl = requested bits, al = bits in buffer
	xchg %al, %cl
	#
	sub %al, %cl # reduce bit count after
	#
	jc .get_bits_error # should never happen
	#
	# now al = num of rest bits
	#
	inc %si
	#
	shl %rsi, %cl # << in_bits
	shr %rdx, %cl # remove from buffer
	#
	dec %rsi # esi = ((1<<in_bits) - 1)
	#
	mov %cl, %al # rest bits
	and %rdi, %rsi # use bitmask to get the bits
	#
	mov %rax, %rdi
	#
	btr %r12, 63
	jnc ._get_bit_add_no_offset
	# has offset!
	add %rax, %rbx
._get_bit_add_no_offset:
	# pack buffer
	shl %rdx, 6
	or %rdx, %rcx
	mov %r12, %rdx
	#
.get_bits_stored:
	# mov [%rbp - IN_BITS], %rax
	#
	.if GET_BITS_BACKUP_ON_STACK
	movq %rcx, [%rbp - RCX_BACKUP]
	movq %rdx, [%rbp - RDX_BACKUP]
	movq %rdi, [%rbp - RDI_BACKUP]
	movq %rsi, [%rbp - RSI_BACKUP]
	.else
	#
	movq %rcx, %xmm1
	movq %rdx, %xmm2
	movq %rsi, %xmm3
	movq %rdi, %xmm4
	#
	.endif
	#
	ret
	#
.get_bits_end:
	#
	#
	#
.inflate_aio_block_data:
	#
	#
	lea %rdi, [%r13 + LENGTH_TREE_OFFSET]
	#
	movq %xmm0, %rdi
	#
.block_data_loop_start:
	# decode symbol
	#
	call .inflate_update_avail_values
	#
	movq %rdi, %xmm0
	# int sym = inflate_decode_symbol(d, len_tree);
	call .inflate_decode_symbol_func
	#
.block_data_loop_got_symbol:
	#
	cmp %ax, 256 # end of block?
	jne .block_data_loop_not_end
	# new iteration!
	jmp .loop_start_edx
	#
.block_data_loop_not_end:
	#
	# call .inflate_update_avail_values
	#
	test %r11, %r11
	jz .block_data_error_1 # no bytes available to write
	#
	cmp %ax, 256 # literal 0..255 ?
	ja .block_data_loop_not_literal
	#
	movq %rdi, %xmm6
	cmp %r9, %rdi
	jae .block_data_error_1
	mov [%r9], %al # store literal
	inc %r9 # dest ptr
	dec %r11 # write avail
	jmp .block_data_loop_start
	#
.block_data_loop_not_literal:
	movq %rdi, %xmm0 # rdi = length tree
	#
# TODO : check in get symbol to avoid this check
	#
	cmp %ax, [%rdi] # sym > len_tree_max_sym
	jg .block_data_error_1 # signed compare
	# cmp %ax, %bx # sym > len_tree_max_sym
	# jle .block_data_error_1 # signed compare
	#
	mov %dx, [%rdi + 640]
	# mov %dx, [%rdi + %rcx]
	# cmp %dx, 0 # distance tree empty ?
	test %dx, %dx
	js .block_data_error_1
	#
	# xchg %ax, %bx
	#
	# for easier indexing
	#
	xor %rcx, %rcx
	sub %ax, 257 # sym - 257
	#
	# Extra bits and base tables for length codes
	# static const unsigned char length_bits[29] = {
	# 0, 0, 0, 0, 0, 0, 0, 0, 
	# 1, 1, 1, 1, 2, 2, 2, 2, 
	# 3, 3, 3, 3, 4, 4, 4, 4, 
	# 5, 5, 5, 5, 0}             ;
	# static const unsigned short length_base[30] = {
	# 3, 4, 5, 6, 7, 8, 9, 10, 
	# 11, 13, 15, 17, 
	# 19, 23, 27, 31, 
	# 35, 43, 51, 59, 
	# 67, 83, 99, 115, 
	# 131, 163, 195, 227, 258, 0} ;
	#
	# ax = sym, %bx = length
	#
	# We try to compute values
# Instead of :
	# num_bits = length_bits[sym]
	# length = length_base[sym]
	# nop
	
.block_data_gen_values:
	#
	# xor %bx, %bx
	mov %rbx, %rcx
	mov %rdx, %rcx
	# xor %dx, %dx
	jmp .block_data_no_error_1
.block_data_error_1: # just a "jump gate"
	jmp .error_state # far jump ...
.block_data_no_error_1:
	#
	cmp %al, 28
	jb .bd_gv
	ja .bd_gv_end
	mov %bx, 258 # length offset
	xor %al, %al # num bits
	jmp .bd_gv_end
	#
.bd_gv:
	cmp %al, 8
	# jnc .block_data_sym_ge8
	jae .block_data_sym_ge8
	# add %bl, %al
	mov %bl, %al
	xor %al, %al # num bits
	jmp .bd_gv_end_1
.block_data_sym_ge8:
# example: al = 13
	# minus 8 = 5
	# a. rest after 'and' with 0x3 = 1
	# b. after shift by 2 = 1
	# shift a.rest by (1 + 1) = 1 << 2 = 4
	# shift b by (2 + 2) = 1 << 4 = 16
	# result = 16 + 4 + (3 offset) = 23
	sub %al, 8
	inc %bl # 0x1 for shifting
	mov %cl, %al
	and %al, 0x3 # only 2 bits of interest
	shr %cl, 2 # we are only interested in 4 - tuples
	mov %dl, %cl # for length bits later
	#
	inc %cl
	shl %al, %cl # shift by one - forth
	add %cl, 2 # e.g. 11, 13, 15, 17 minus 3, base is 8
	shl %bl, %cl # now we know 8, 16, 32, 64, 128
	add %bl, %al # result
	#
	mov %al, %dl
	#
	inc %al
.bd_gv_end_1:
	add %bl, 3
.bd_gv_end:
.block_data_gen_values_end:
	
.sym_check:
	# mov %r15d, %ebx # length
	#
	test %ax, %ax
	jz .sym_check_skip
.sym_check_not_zero:
# here: ax = sym, bx = length
	# Possibly get more bits from length code
	# length = inflate_getbits(d, sym, length);
	#
	call .get_bits_func
	#
	mov %ebx, %eax
.sym_check_skip:
	mov %r15d, %ebx # length
	movq %rdi, %xmm0
	add %rdi, 640 # rdi = distance tree
	# int dist = inflate_decode_symbol(d, distance_tree);
	call .inflate_decode_symbol_func
	#
.block_data_loop_got_dt_symbol:
	# dist is within range? (dist 30, 31 are invalid)
	# if (dist > 29 || dist > dt_max_sym)
	# {
	# result = INFLATE_ERROR_DATA;
	# break                      ;
	# }
	movq %rdi, %xmm0
	add %rdi, 640 # rdi = distance tree
	#
	cmp %ax, 29
	jbe .block_data_dist_valid
.block_data_error_3:
	jmp .block_data_error_1
.block_data_dist_valid:
	#
	# mov %bx, [%rdi] # bx = max_sym
	#
	cmp %ax, [%rdi] # sym > max_sym ?
	jg .block_data_error_3 # signed comparison!
	#
	# We try to compute the following tables
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
	
.block_data_dist_comp:
	# movzx %rax, %al
	# al = dist bits
	# bx = data offset
	lea %ebx, [%eax + 1]
	#
	cmp %al, 4
	jb .has_data_offset
	#
.dist_ge4:
	# dist - = 4
	sub %al, 4
	mov %bl, 1
	movzx %edx, %al
	# const unsigned int o = dist & 0x1;
	# dist >>= 1                 ;
	shr %al, 1
	# dist + +                   ;
	inc %al
	#
	# data_offset = dist + 1     ;
	lea %ecx, [%eax + 1]
	#
	# and %dl, 0x1 # single bit
	and %dl, %bl # single bit
	# inc %cl
	# data_offset = 0x1 << data_offset;
	shl %bx, %cl
	# data_offset |= (data_offset >> o);
	# bt %dx, 0 # need to perform OR with shifted values?
	# jnc .dist_no_shift
	mov %cl, %dl
	mov %dx, %bx
	# shorter even if maybe a bit slower
	shr %dx, %cl # not necessary shift if dl=cl=0
	or %bx, %dx # not necessary shift if dl=cl=0
	# lea %ebx, [%edx + %ebx]
.dist_no_shift:
	# data_offset + +            ;
	inc %ebx
	xor %rdx, %rdx
	xor %rcx, %rcx
	#
	# Possibly get more bits from distance code
	# data_offset = inflate_getbits(d, dist, data_offset);
	call .get_bits_func
	#
	mov %ebx, %eax
.block_data_dist_comp_end:
	#
.has_data_offset:
	# ebx = data offset
	# check space left
	#
	# int64_t addr_dist = (d - >destination - d - >destination_start);
	# Possible to go back?
	# if (data_offset > (int)addr_dist) {
	# return INFLATE_ERROR_DATA  ; ;
	# }
	mov %ecx, %r15d # data length
	#
	cmp %ebx, 0x7FFF # 15 bits offset
	ja .block_data_error_3
	#
	movzx %rbx, %bx
	#
	# enough space left to write?
	cmp %ecx, %r11d
	ja .block_data_error_3
	#
	mov %rax, %r9
	#
	mov %rdx, [%rbp - DST_START] # src start
	#
	mov %rsi, %r9
	#
	#
	sub %rax, %rdx # rax = bytes written
	jc .block_data_error_3 # underflow ?
	jz .block_data_error_3 # no bytes written ?
	#
	# data_offset > bytes_written ?
	cmp %rbx, %rax
	ja .block_data_error_3
	#
	sub %rsi, %rbx # src = dst - data_offset
	mov %rdi, %r9 # rdi = dst ptr
	#
	# Copy bytes
	cld
	rep movsb # copy rcx bytes
	mov %r9, %rdi # save dst ptr
	jmp .block_data_loop_start
# .block_data_error_2:
.inflate_aio_block_data_end:
	#
	#
	#
.clear_buffer:
	xor %rcx, %rcx
	xor %rax, %rax
	#
	mov %rdi, %r13 # buffer ptr
	mov %cl, BUFFER_LEN_8
	cld
	rep stosq
	#
	# Sets max symbols to - 1 for trees
	#
	lea %rdi, [%r13 + LENGTH_TREE_OFFSET] # rdi = l_tree ptr
	dec %ax # now it's minus 1
	mov %cx, (TREE_SIZE - 2) # - 2, because of stosw
	# mov [%rdi], %ax
	stosw
	add %rdi, %rcx
	stosw
	# mov [%rdi], %ax
	ret
	#
	.equ INFLATE_DECODE_SYMBOL_GET_BITS, 1
	#
.inflate_decode_symbol_func:
	# r14 = temp
	# rdi = tree start
	# eax = result
	# ebx = offset
	# edx = base
	#
	# int base = 0, offs = 0, len;
	#
# for : len = 1 ... 16 : inc len {
	#
	# start at 1
	lea %rsi, [%rdi + 8 + 2] # rsi = code counts ptr
	#
	.if INFLATE_DECODE_SYMBOL_GET_BITS
	mov %rax, %r12 # rax = bitbuffer|count
	movzx %rcx, %r12b
	movq %r10, %xmm5 # r10 = end ptr
	movq %xmm4, %r15 # temp store
	#
	shr %rax, 6 # %rax = bit buffer
	#
	sub %r10, %r8 # r10 = data avail
	jc .decode_symbol_error # overflow ?
	#
	test %cl, 0x38 # more than 7 bits ?
	jae .decode_symbol_error
	#
	# more than 8 bits in buffer?
	cmp %rax, 0x7F
	ja .decode_symbol_error
	#
	movq %xmm4, %r15 # temp store
	and %cl, 0x7 # cl = bit buffer counter
	cld
	xor %rbx, %rbx
	xor %rdx, %rdx
	mov %r15b, 1
	#
	.else
	#
	xor %rcx, %rcx
	xor %rax, %rax
	xor %rbx, %rbx
	xor %rdx, %rdx
	#
	# mov %cl, 15 # len
	inc %cl
	.endif
	# ebx = offset
	# eax = temp
	# edx = base
	# cx = loop counter
.decode_symbol_loop_start:
	# offs = (offs << 1) | inflate_getbits(d, 1, 0);
	shl %bx, 1
	#
	.if INFLATE_DECODE_SYMBOL_GET_BITS
	test %cl, %cl # bit buffer empty ?
	jnz .decode_symbol_has_bits
	# any bytes available to read?
	test %r10, %r10
	jz .decode_symbol_error
	#
	mov %al, [%r8]
	mov %cl, 8
	dec %r10
	inc %r8
.decode_symbol_has_bits:
	dec %cl
	shr %ax, 1
	adc %bx, 0 # add carry flag from shift
	#
	.else
	mov %al, 1
	#
	sub %rsp, 8
	call .get_bits_func_no_base
	add %rsp, 8
	#
	cld
	#
	shr %al, 1
	adc %bx, 0 # add carry flag from shift
	.endif
	#
	#
	lodsw # ax = code count and rsi + = 2
	#
	# const int c = t_code_counts.us16[len];
	# if (offs < c) {
	# break                      ;
	# }
	# base + = c                 ;
	# offs - = c                 ;
	#
	bt %bx, 15 # overflow?
	jc .decode_symbol_error
	# clc
	# sub %dx, %ax # offset minus code count
	# jc .decode_symbol_loop_offset_less
	cmp %bx, %ax # offset less than code count?
	jb .decode_symbol_loop_end
	#
	.if INFLATE_DECODE_SYMBOL_GET_BITS
	inc %r15b
	.else
	inc %cl
	.endif
	#
	sub %bx, %ax
	#
	add %dx, %ax # base plus code count
	jc .decode_symbol_error
	# next iteration!
	.if INFLATE_DECODE_SYMBOL_GET_BITS
	cmp %r15b, 16
	jb .decode_symbol_loop_start
	.else
	# loopnz .decode_symbol_loop_start
	cmp %cl, 16
	jb .decode_symbol_loop_start
	.endif
	# otherwise error (too many bits)
.decode_symbol_error:
	mov %rsp, [%rbp - RSP_BACKUP]
	jmp .error_state # overflow or bit buffer error!
.decode_symbol_loop_end:
	# }
	#
	.if INFLATE_DECODE_SYMBOL_GET_BITS
	# pack bits into bit buffer
	movzx %r12, %al # rest bits
	add %ebx, %edx # offset + = base
	shl %r12, 6
	movq %r15, %xmm4
	or %r12, %rcx # and the bit buffer count
	.else
	add %ebx, %edx # offset + = base
	.endif
	#
	
	# jc .decode_symbol_error # overflow?
	# / / assert(base + offs >= 0 && base + offs < 288);
	lea %rsi, [%rdi + %rbx * 2 + (32 + 8)]
	# base + = offs              ;
	# if (base < 0 || base > 287)
	# {
	# return INFLATE_ERROR
	# }
	# js .decode_symbol_error
	cmp %ebx, 287 # out_of_range ?
	ja .decode_symbol_error
	#
	# return t - >symbols.us16[base];
	# shl %dx, 1 # for shorts
	lodsw # ax = symbol
	# error for sym > 285 !
	cmp %ax, 285 # invalid symbol ?
	ja .decode_symbol_error
	#
	cmp %ax, [%rdi] # symbol greater than tree's max sym
	ja .decode_symbol_error
	#
	# pop %r14
	ret
.inflate_decode_symbol_func_end:
	#
	#
	#
.dyn_mode_build_trees:
	#
	call .clear_buffer
	#
	# Decode trees
	# 5 bits for HLIT (257 - 286) + offset 257
	# 5 bits for HDIST (1 - 32) + offset 1
	# 4 bits for HCLEN (4 - 19) + offset 4
	#
	call .inflate_update_avail_values
	#
	mov %rdx, %r12
	movzx %rcx, %r12b
	xor %rbx, %rbx
	xor %r14, %r14 # temp
	xor %r15, %r15 # temp
	test %rdx, %rdx
	jz .dyn_mode_build_bit_buffer_empty
	# invalid count ? too many bits in buffer?
	# should never happen and means corrupted bit buffer
	test %cl, 0x38 # 0x38 = xx11.1000
	jz .dyn_mode_build_bits_count_ok
.dyn_mode_build_bits_error:
	jmp .error_state
.dyn_mode_build_bits_count_ok:
	#
	shr %rdx, 6
	and %cl, 0x7
	#
	# too many bits in buffer?
	# should never happen and means corrupted bit buffer
	cmp %rdx, 0x7F # 0x7F = 7 bits set
	ja .dyn_mode_build_bits_error
	#
.dyn_mode_build_bit_buffer_empty:
	#
	# We need at least 26 bits
	# 14 bits for the header
	# at least 12 bits for the hclen
	mov %r14b, (26)
	#
	# This part is now internal "get bits function"
.dyn_mode_build_trees_bits_loop_start:
	cld
	mov %rsi, %r8
.dyn_mode_build_trees_bits_loop:
	xor %rax, %rax
	test %r10, %r10
	jz .dyn_mode_build_bits_error
	#
	lodsw
	#
	dec %r10
	shl %rax, %cl
	add %cl, 8
	or %rdx, %rax # prepend to bit buffer
	#
	# Using only a byte of r14!
	cmp %cl, %r14b
	jb .dyn_mode_build_trees_bits_loop
.dyn_mode_build_trees_head_loop_end:
	#
	mov %r8, %rsi
	mov %r12, %rdx # r12 = full buffer
	#
	btr %r14w, 9
	jae .dyn_mode_build_trees_after_hclen4
	#
	#
	mov %r12, %rdx # r12 = full buffer
	movzx %rax, %dx # ?|hclen|hdist|hlit
	#
	movzx %rbx, %dx # ?|hclen|hdist|hlit
	movzx %rdx, %dx # ?|hclen|hdist|hlit
	# xor %r14, %r14
	shr %bx, 10 # ?|hclen, 6 bits
	shr %dx, 5 # ?|hclen|hdist, 11 bits
	#
	and %ax, 0x1F # al = hlit
	and %bl, 0xF # bl = hclen
	and %dx, 0x1F # dl = hdist
	#
	# Checks before adding offsets
	# if (hlit > 286 || hdist > 30) {
	# return INFLATE_ERROR_DATA  ;
	# }
	cmp %al, 29 # hlit ?
	ja .dyn_mode_build_trees_error
	#
	cmp %dl, 29 # hdist ?
	ja .dyn_mode_build_trees_error
	#
	add %ax, 257 # hlit
	inc %dl # hdist
	add %bl, 4 # hclen
	#
	shl %eax, 16 # hlit shifted
	shl %edx, 8 # hdist shifted
	or %eax, %ebx # add hclen
	or %eax, %edx # add hdist
	
	mov [%rbp - HLIT_HDIST_HCLEN], %eax
	mov %r15, %rax
	#
	# xor %r15, %r15
	# ax = hlit, dx = hdist, bx = hclen
	# r12 = bit buffer, cl = bit buffer count
	#
	# rdi = lengths array ptr!
	lea %rdi, [%r13 + LENGTH_ARRAY_OFFSET + 16]
	#
	sub %cl, 14 # we consume 14 bits
	jc .dyn_mode_build_bits_error
	shr %r12, 14 # r12 = now 12 bits at least for hclen
	cmp %cl, 14
	jb .dyn_mode_build_bits_error
	#
	# Now we need to set lengths based on hclen
	# First 4 will be assigned anyway
	# static const unsigned char my_clcidx[19] = {
	# 16, 17, 18, 0, 
	# 8, 7, 9, 6, 
	# 10, 5, 11, 4, 
	# 12, 3, 13, 2, 
	# 14, 1, 15 }
	movzx %rax, %r12b
	movzx %rbx, %r12b
	movzx %rdx, %r12w
	and %al, 0x7 # 3 bits
	and %bl, (0x7<<3) # 0x38 # 3 bits
	and %dx, (0x7<<6) # 0x1C # 3 bits
	shr %bl, 3 # adjust for index 17
	shl %dx, 10 # adjust for index 18
	mov %ah, %bl # ax = for index 16, 17
	shr %r12, 9
	or %eax, %edx
	#
	stosd # set values for index 16, 17, 18
	#
	sub %cl, 12 # reduce bit count
	jc .dyn_mode_build_bits_error
	#
	xor %ebx, %ebx
	movzx %eax, %r12b
	sub %rdi, 20 # go to length start
	# lea %rdi, [%r13 + LENGTH_ARRAY_OFFSET]
	and %al, 0x7 # 3 bits
	xor %edx, %edx
	shr %r12, 3
	#
	stosb
	#
	# dec %rdi
	#
	# now we need to process (hclen - 4) rest 3 bit values
	cmp %r15b, 4
	je .dyn_mode_build_trees_hclen_done
	#
.dm_btl_1:
	#
	mov %dl, 3
	movzx %rax, %r15b # = hclen
	sub %al, 4
	jc .dyn_mode_build_bits_error # should never happen
	jz .dyn_mode_build_trees_hclen_done
	#
	mul %dl
	xor %rbx, %rbx
	movzx %r14, %al
	xor %rdx, %rdx
	# mark to jump to label afer
	bts %r14w, 9 # or %r14w, 0x200
	#
	jmp .dyn_mode_build_trees_bits_loop_start
	#
.dyn_mode_build_trees_after_hclen4:
	#
# Here : cl = bit count, %r12 = bit buffer
	movzx %rax, %r14b # al = requested bits
	# movzx %rdx, %dl
	mov %rbx, %r12 # rbx = bits in bit buffer
	xor %r14, %r14
	#
	xchg %al, %cl # cl = requested, al = all bits
	#
	shr %r12, %cl
	#
	sub %al, %cl
	jc .dyn_mode_build_bits_error
	#
	shl %r12, 6
	# cl = requested bits
	movzx %rcx, %r15b # = hclen
	xor %rdx, %rdx
	or %r12b, %al
	# pack 16 nibbles
	# mov %rsi, ((8)|(7<<4)|(9<<8)|(6<<12)|(10<<16)|(5<<20)|(11<<24)|(4<<28)|(12<<32)|(3<<36)|(13<<40)|(2<<44)|(14<<48)|(1<<52)|(15<<56))
	xor %rax, %rax
	mov %dl, 8
	sub %cl, 4
	lea %r14, [%r13 + LENGTH_ARRAY_OFFSET]
	#
	# for (i = 0                 ; i < hclen; + + i)
	# {
.dyn_mode_build_trees_length_loop:
	test %dl, %dl
	jz .dyn_mode_build_trees_error
	#
	# 3 bits code length (0 - 7)
	#
	# mov %dx, %si
	mov %al, %bl # get from buffer
	#
	# and %dx, 0xF # we need a nibble
	#
	and %al, 0x7 # 3 bits
	# shr %rsi, 4 # for next iteration
	#
	# add %rdx, %rdi
	lea %rdi, [%r14 + %rdx]
	shr %rbx, 3
	#
	# lengths[my_clcidx[i]] = (unsigned char)clen;
	#
	stosb
	#
	test %rbx, %rbx # any other bits set?
	jz .dyn_mode_build_trees_hclen_done
	#
	xor %dl, 0xF
	bt %dx, 3
	adc %dl, 0
	#
	loopnz .dyn_mode_build_trees_length_loop
	# }
	#
	jmp .dyn_mode_build_trees_hclen_done
.dm_btl_1_end:
	#
.dyn_mode_build_trees_error:
	jmp .error_state
	#
.dyn_mode_build_trees_hclen_done:
	#
	#
	# restore values
	#
	# Build length tree
	# xor %rcx, %rcx
	# rdi = l_tree ptr
	lea %rdi, [%r13 + LENGTH_TREE_OFFSET]
	# rsi = lengths ptr
	lea %rsi, [%r13 + LENGTH_ARRAY_OFFSET]
	xor %rbx, %rbx
	movq %xmm0, %rdi
	mov %cx, 19 # length size
	#
	call .build_tree
	#
	#
	# lea %rsi, [%r13 + LENGTH_TREE_OFFSET]
	movq %rsi, %xmm0
	#
	lodsw # load max sym
	#
	cmp %ax, 18
	ja .dyn_mode_build_trees_error
	#
	# Decode code lengths for the dynamic trees
	#
	mov %eax, [%rbp - HLIT_HDIST_HCLEN]
	xor %rcx, %rcx
	#
	# xor %rbx, %rbx
	mov %cl, %ah
	xor %rdx, %rdx
	shr %eax, 16 # eax = hlit
	add %cx, %ax # ecx = hlit + hdist
	#
	cmp %cx, (258 + (2 * 29))
	ja .dyn_mode_build_trees_error
	#
	pxor %xmm0, %xmm0 # num
	#
	# for (num = 0               ; num < hlit + hdist;)
	#
	movzx %r14, %cx # r14 = limit
	mov %rsi, LENGTH_TREE_OFFSET
	#
	# {
.loop_num_hlit_hdist:
	# int sym = inflate_decode_symbol(d, lt);
	lea %rdi, [%r13 + %rsi]
	#
	call .inflate_decode_symbol_func
	# ax = result
	# if (sym > lt_max_sym)
	# {
	# return INFLATE_ERROR_DATA  ;
	# }
	# xor %rbx, %rbx
	#
	movzxw %rbx, [%r13 + %rsi]
	#
	# should not be required, but check anyway
	cmp %bx, 18
	ja .dyn_mode_build_trees_error
	#
	cmp %ax, %bx
	ja .dyn_mode_build_trees_error
	#
	xor %bx, %bx
	#
	#
	# switch (sym)
	# {
# default:
	# Values 0 - 15 represent the actual code lengths
	# length = 1                 ;
	# break                      ;
	# }
	#
	inc %bl # length 1
	#
	movd %ecx, %xmm0 # cx = num
	#
.s2:
	cmp %al, 16
	jb .switch_sym_end # keep symbol, length = 1
	ja .switch_sym_not_16
# here : sym = 16
	# test, %cx, %cx
	# jz .dyn_mode_build_trees_error
	dec %cx
	js .dyn_mode_build_trees_error
	lea %rsi, [%r13 + %rcx + 64]
	# xchg %al, %bl
	#
	lodsb # sym = get value at length[num - 1]
	#
	mov %bl, 0x23 # for get_bits (2, 3)
	inc %cx
	jmp .switch_sym_end
	#
.switch_sym_not_16:
	.if 1
	mov %cl, %al
	mov %bx, 0x7B33 # (3, 3) for 17, (7, 11) for 18
	dec %cl # now 0 for 17, 1 for 18
	xor %al, %al
	shl %cl, 3 # to shift bytes
	shr %bx, %cl
	.else
	bt %ax, 0
	jnc .switch_sym_not_16_1
	mov %bl, 0x33
	jmp .switch_sym_not_16_2
.switch_sym_not_16_1:
	mov %bl, 0x7B
.switch_sym_not_16_2:
	.endif
.switch_sym_end:
	mov %r15w, %ax # store sym
	# compute length ?
	cmp %bl, 1
	je .dont_compute_length
	mov %al, %bl
	and %bl, 0xF
	shr %al, 4
	#
	call .get_bits_func
	#
	mov %bl, %al
	#
.s2_end:
	
.dont_compute_length:
	# if (length > hlit + hdist - num) {
	# return INFLATE_ERROR_DATA  ;
	# }
	#
	movq %rcx, %xmm0
	#
	movzx %rax, %r14w # max length available
	#
	sub %ax, %cx # calc rest length
	# jc .dyn_mode_build_trees_error # overflow ?
	#
	cmp %bx, %ax
	jg .dyn_mode_build_trees_error_2 # overflow ?
	#
	xor %ax, %ax
	# length at index num
	lea %rdi, [%r13 + %rcx + 64]
	#
	mov %al, %r15b # al = sym
	#
	cld
	#
	.if 0
	cmp %bx, 1
	jb .dyn_mode_build_trees_error_2
	ja .loop_num_hlit_hdist_many
	#
	stosb
	inc %cx
	jmp .loop_num_hlit_hdist_store
.loop_num_hlit_hdist_many:
	.endif
	#
	add %cx, %bx # cx = num + copy length
	#
	# cx = copy length, bx = new num after iterations
	xchg %cx, %bx
	#
	# while (length - - != 0) {
	# lengths[num + + ] = sym    ;
	# }
	rep stosb
	#
	mov %cx, %bx # cx = num
	#
.loop_num_hlit_hdist_store:
	#
	movd %xmm0, %ecx # store loop counter
	#
	cmp %cx, %r14w
	jb .loop_num_hlit_hdist
.loop_num_hlit_hdist_end:
	# }
	#
	#
	lea %rsi, [%r13 + (64 + 256)]
	#
	lodsb
	#
	# Is end_of_block present ?
	test %al, %al
	jz .dyn_mode_build_trees_error_2
	#
	mov %ecx, [%rbp - HLIT_HDIST_HCLEN]
	#
	lea %rdi, [%rsi + 64] # length tree
	#
	lea %rsi, [%r13 + 64] # length ptr
	#
	shr %ecx, 16 # cx = hlit
	#
	call .build_tree
	#
	# test %eax, %eax
	# jnz .dyn_mode_build_trees_error_2
	#
	xor %rax, %rax
	xor %rcx, %rcx
	# mov %rsi, %r13
	mov %eax, [%rbp - HLIT_HDIST_HCLEN]
	lea %rdi, [%r13 + DISTANCE_TREE_OFFSET]
	mov %cl, %ah # cl = ah = hdist
	shr %eax, 16 # ax = hlit
	# add %rsi, %rax # length + hlit
	lea %rsi, [%r13 + %rax + 64]
	#
	call .build_tree
	#
	# test %eax, %eax
	# jnz .dyn_mode_build_trees_error_2
	#
	jmp .inflate_aio_block_data
.dyn_mode_build_trees_error_2:
	jmp .error_state
.dyn_mode_build_trees_end:
	#
	#
# build_tree:
	# rdi = tree ptr
	# rsi = length array ptr
	# cx = num = length of length array
.build_tree:
	# if (num > 288) {
	# return INFLATE_ERROR_DATA  ;
	# }
	cmp %cx, 288
	jbe .build_tree_init_tree
	#
.build_tree_error:
	mov %rsp, [%rbp - RSP_BACKUP]
	jmp .error_state
	#
.build_tree_init_tree:
	# inflate_init_tree(t)
	movzx %rbx, %cx # num
	#
	mov %cx, 80
	mov %rdx, %rdi # tree ptr
	movzx %rcx, %cx
	xor %rax, %rax
	cld
	rep stosq # 8 * 80 = 640 bytes / zeros
	#
	dec %ax
	#
	mov %rdi, %rdx
	#
	mov %rcx, %rbx # restore num
	#
	stosw
	#
	mov %r15w, %ax # r15w = min sym = - 1
	# init tree end
	#
	# unsigned int ms = t - >max_sym;
	# unsigned short * pC = t - >code_counts.us16;
	add %rdi, 8 # rdi = code counts
	# xor %rbx, %rbx # bx = iteration variable
	# xor %rdx, %rdx # temp
	.if USE_CLEAR_REGS
	mov %al, (CLR_RAX| CLR_RBX|CLR_RDX)
	call .clear_regs
	.else
	xor %rax, %rax
	xor %rbx, %rbx
	xor %rdx, %rdx
	.endif
	#
.build_tree_loop_start:
	# Count number of codes having non - zero length
	# for (i = 0                 ; i < num; + + i)
	# {
	# unsigned int l = lengths[i];
	lodsb
	#
	test %al, %al
	jz .build_tree_loop_len_zero
	# if (l != 0)
	# {
	# if (l > 15)
	# {
	# return INFLATE_ERROR_DATA  ;
	# }
	# ms = i                     ;
	# pC[l] + +                  ;
	# }
	.if INFLATE_ASM_CHECK_ERRORS
	cmp %al, 15
	jge .build_tree_error
	.endif
	#
	shl %ax, 1 # we use shorts
	add %rax, %rdi # rdx = code counts[l]
	mov %r15w, %bx # max sym
	incw [%rax]
.build_tree_loop_len_zero:
	# }
	inc %rsi
	inc %bx
	xor %rax, %rax # reset
	#
	cmp %bx, %cx
	jb .build_tree_loop_start
	#
	sub %rsi, %rcx # rsi = length arr
	#
	mov %r14w, %cx # count backup
	#
	# t - >max_sym = ms          ;
	mov [%rdi - 8], %r15w
	#
	xor %rcx, %rcx
	#
	# rdi = code counts ptr
	mov %r15, %rdi
	#
	# clear offsets
	mov %cl, 4 # 4 * 8 = 32 bytes = 16 offsets
	mov %rdi, %r13 # r15 = offsets ptr
	# rax is already 0
	cld
	rep stosq
	# clear offsets end
	#
	mov %rdi, %r15
	#
	mov %cl, 32
	#
	# unsigned int num_codes = 0, available = 1;
	mov %r15, %r13 # r15 = offsets ptr
	movq %xmm3, %rdi # code counts
	movq %xmm4, %rsi # lengths backup
	#
	xor %rdx, %rdx # available
	mov %rsi, %rdi # rsi = code counts
	xor %rbx, %rbx # num codes
	inc %dl
.build_tree_offset_loop:
	# Compute offset table for distribution sort
	# for (i = 0                 ; i < 16; + + i)
	# {
	# unsigned int used = t_code_counts.us16[i];
	# mov %ax, [%rdi]
	lodsw
	# #if INFLATE_DO_CHECK_FOR_ERRORS
	# / * Check length contains no more codes than available * / 
	# if (used > available)
	# {
	# return INFLATE_ERROR_DATA  ;
	# }
	# #endif
	#
	# available = (available - used) << 1;
	sub %edx, %eax
	# .if INFLATE_ASM_CHECK_ERRORS
	jc .build_tree_error
	# .endif
	shl %edx, 1
	mov [%r15], %bx # offsets[i] = num_codes ;
	add %ebx, %eax # num_codes + = used ;
	add %rsi, 2 # code counts + 2
	add %r15, 2 # offsets + 2
	#
	loopnz .build_tree_offset_loop
	# }
	#
	# * Check all codes were used, 
	# or for the special case of only one code that it has length 1
	#
	# if ((num_codes > 1 && available > 0)
	# || (num_codes == 1 && t - >code_counts.us16[1] != 1))
	# {
	# return INFLATE_ERROR_DATA  ;
	# }
	#
	# rdi = code counts
	test %dx, %dx
	jz .build_tree_check_cc1
	cmp %bx, 1
	ja .build_tree_error
	#
.build_tree_check_cc1:
	cmp %bx, 1
	jne .build_tree_check_ok
	# mov %ax, [%rdi + 2]
	# cmp %ax, 1
	cmpw [%rdi + 2], 1
	jne .build_tree_error
	#
.build_tree_check_ok:
	#
	movzx %rcx, %r14w # cx = num
	#
	movq %rsi, %xmm4 # rsi = lengths
	mov %r15w, [%rdi - 8] # r15w = max sym
	mov %r14d, %ebx # r14 = num_codes
	#
	.if USE_CLEAR_REGS
	mov %al, (CLR_RAX|CLR_RBX|CLR_RDX)
	call .clear_regs
	.else
	xor %rbx, %rbx
	xor %rax, %rax
	xor %rdx, %rdx
	.endif
	#
	# rdi = code counts
	movq %rdi, %xmm3
	#
.build_tree_fill_symbols:
	#
	# Fill in symbols sorted by code
	# for (i = 0                 ; i < num; + + i)
	# {
	# register unsigned int l = lengths[i];
	lodsb
	#
	test %al, %al
	jz .build_tree_fill_symbols_len_is_zero
	# if (l != 0)
	# {
	shl %ax, 1 # accessing shorts
	add %rax, %r13 # add buffer
	# unsigned short j = offsets[l];
	# offsets[l] increment
	mov %bx, [%rax] # offset
	inc %bx
	mov [%rax], %bx
	shl %bx, 1 # accessing shorts
	# add (16 - 1) * 2, %rdi tree offset + 8
	lea %rax, [%rdi + %rbx + (32 - 2)]
	#
	# t_symbols.us16[j] = i      ;
	mov [%rax], %dx
	# }
.build_tree_fill_symbols_len_is_zero:
	inc %rsi # length ptr plus 1
	inc %dx # counter update
	xor %rbx, %rbx
	xor %rax, %rax
	cmp %dx, %cx
	jb .build_tree_fill_symbols
	# }
	#
.build_tree_fill_symbols_end:
	#
	# For the special case of only one code (which will be 0)
	# add a code 1 which results in a symbol that is too large
	#
	# if (num_codes == 1)
	# {
	cmp %r14w, 1
	jne .build_tree_num_codes_ne1
	#
	# t_symbols.us16[1] = t_max_sym + 1;
	inc %r15w
	inc %r14w
	mov [%rdi + 34], %r15w
	#
	# t_code_counts.us16[1] = 2  ;
	mov [%rdi + 2], %r14w
	# }
.build_tree_num_codes_ne1:    ;
	ret
.build_tree_end:
	#
	#
	.if USE_CLEAR_REGS
.clear_base_regs:
	mov %al, (CLR_RAX|CLR_RBX|CLR_RCX|CLR_RDX)
	jmp .clear_base_regs_impl
.clear_regs: # used to clear regs
	.ifdef CLR_RDI
	bt %al, CLR_RDI_BIT
	jnc .clear_regs_no_rdi
	xor %rdi, %rdi
.clear_regs_no_rdi:
	.endif
	#
	.ifdef CLR_RSI
.clear_regs_rsi:
	bt %ax, CLR_RSI_BIT
	jnc .clear_regs_no_rsi
	xor %rsi, %rsi
.clear_regs_no_rsi:
	.endif
	#
.clear_base_regs_impl:
	.ifdef CLR_RDX
	bt %ax, CLR_RDX_BIT
	jnc .clear_regs_no_rdx
	xor %rdx, %rdx
.clear_regs_no_rdx:
	.endif
	#
	.ifdef CLR_RCX
	bt %ax, CLR_RCX_BIT
	jnc .clear_regs_no_rcx
	xor %rcx, %rcx
.clear_regs_no_rcx:
	.endif
	#
	.ifdef CLR_RBX
	bt %ax, CLR_RBX_BIT
	jnc .clear_regs_no_rbx
	xor %rbx, %rbx
.clear_regs_no_rbx:
	.endif
	# bt %ax, 
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
	mov %al, 4
	jmp .restore_and_return
# .error_state_no_bits:
	# mov %al, 3
	# jmp .restore_and_return
.error_state_invalid_params:
	mov %al, 6
	jmp .restore_and_return
.error_state_internal_1:
	mov %al, 9
	jmp .restore_and_return
.error_state_internal_2:
	mov %al, 10
	jmp .restore_and_return
	#
	.if INFLATE_ASM_RANGE_CHECK
.error_state_not_supported_range:
	mov %al, 8
	jmp .restore_and_return
	.endif
	#
.error_state_not_implemented:
	mov %al, 5
	jmp .restore_and_return
	#
.getrip:
	mov %rax, [%rsp]
	ret
	#
.ok_state:
	xor %eax, %eax
.restore_and_return:
	movzx %eax, %al
	#
	# 2 dummies with 2 zeros
	pop %r11
	pop %rbx
	#
	#
	pop %r15
	pop %r14
	#
	pop %r13
	pop %r12
	#
	pop %r11
	pop %rbx
	#
	pop %rbp
	ret
	
_inflate_asm_x64_end:
	#
	.equ RAW_MODE_SIZE, (.raw_mode_end - .raw_mode)
	.equ GET_BITS_SIZE, (.get_bits_end - .get_bits_func)
	.equ BUILD_TREE_SIZE, (.build_tree_end - .build_tree)
	.equ BD_GV, (.block_data_gen_values_end - .block_data_gen_values)
	
inflate_store_xmm_registers:
	# movups instead of movaps, because memory can be unaligned
	movups [%rdi], %xmm0
	movups [%rdi + 16], %xmm1
	#
	movups [%rdi + 32], %xmm2
	movups [%rdi + 48], %xmm3
	#
	movups [%rdi + 64], %xmm4
	movups [%rdi + 80], %xmm5
	#
	movups [%rdi + 96], %xmm6
	movups [%rdi + 112], %xmm7
	ret
.inflate_store_xmm_registers_end:
	
inflate_restore_xmm_registers:
	movups %xmm0, [%rdi]
	movups %xmm1, [%rdi + 16]
	#
	movups %xmm2, [%rdi + 32]
	movups %xmm3, [%rdi + 48]
	#
	movups %xmm4, [%rdi + 64]
	movups %xmm5, [%rdi + 80]
	#
	movups %xmm6, [%rdi + 96]
	movups %xmm7, [%rdi + 112]
	ret
.inflate_restore_xmm_registers_end:
	
.mem_copy_asm:
	test %ecx, %ecx
	jz .mem_copy_asm_end
	#
	cmp %rdi, %rsi
	je .mem_copy_asm_end
	#
	lea %eax, [%esi + %edi]
	#
	test %rsi, %rsi
	jz .mem_copy_asm_error_end
	or %al, %cl
	#
	test %rdi, %rdi
	jz .mem_copy_asm_error_end
	#
	cmp %al, 64
	jb .mem_copy_asm_copy_bytes
	# shr %al, 1
	test %al, 0x3
	jnz .mem_copy_asm_copy_bytes
	#
	test %al, 0x4
	jnz .mem_copy_asm_copy_dwords
	#
	shr %ecx, 3 # 8 bytes at once
	rep movsq
	# jmp .mem_copy_asm_end
.mem_copy_asm_copy_dwords:
	shr %ecx, 2 # 4 bytes at once
	rep movsd
	# jmp .mem_copy_asm_end
.mem_copy_asm_copy_bytes:
	rep movsb
	# nop
	jmp .mem_copy_asm_end
.mem_copy_asm_error_end:
	xor %rax, %rax
.mem_copy_asm_end:
	
	.equ STORE_XMM_LEN, (.inflate_store_xmm_registers_end - inflate_store_xmm_registers)
	.equ RESTORE_XMM_LEN, (.inflate_restore_xmm_registers_end - inflate_restore_xmm_registers)
	
inflate_asm_x64_diffs:
	push %rbp # callee saved
	#
	mov %rbp, %rsp
	cld
	#
	mov %eax, FUNC_LEN
	stosd
	#
	mov %eax, GET_BITS_SIZE
	stosd
	#
	mov %eax, BUFFER_LEN
	stosd
	mov %eax, BUFFER_LEN_8
	stosd
	#
	mov %eax, BUILD_TREE_SIZE
	stosd
	#
	mov %eax, (.dyn_mode_build_trees_end - .dyn_mode_build_trees)
	stosd
	#
	mov %eax, BD_GV
	stosd
	#
	mov %eax, NUM_TO_ZERO
	stosd
	#
	mov %eax, NUM_TO_ZERO_8
	stosd
	#
	mov %eax, LAST_TO_ZERO
	stosd
	#
	mov %eax, (.mem_copy_asm_end - .mem_copy_asm)
	stosd
	#
	mov %eax, STORE_XMM_LEN
	stosd
	#
	mov %eax, RESTORE_XMM_LEN
	stosd
	#
	mov %eax, RAW_MODE_SIZE
	stosd
	#
	mov %eax, (.dm_btl_1_end - .dm_btl_1)
	stosd
	#
	.if 0
	mov %eax, (.dm_btl_2_end - .dm_btl_2)
	stosd
	#
	mov %eax, (.dm_btl_3_end - .dm_btl_3)
	stosd
	.endif
	#
	mov %eax, (.get_bits_error - .get_bits_func)
	stosd
	#
	mov %eax, (.inflate_aio_block_data_end - .inflate_aio_block_data)
	stosd
	#
	mov %eax, (.block_data_error_1 - .inflate_aio_block_data)
	stosd
	#
	.if 0
	mov %eax, (.block_data_error_2 - .inflate_aio_block_data)
	stosd
	#
	mov %eax, (.inflate_aio_block_data_end - .block_data_error_2)
	stosd
	.endif
	#
	mov %eax, (.inflate_aio_block_data_end - .block_data_error_3)
	stosd
	#
	mov %eax, (.block_data_dist_comp_end - .block_data_dist_comp)
	stosd
	#
	mov %eax, (.fixed_mode_build_trees_end - .fixed_mode_build_trees)
	stosd
	#
	mov %eax, (.fixed_mode_compact_end - .fixed_mode_compact_start)
	stosd
	#
	.if 0
	mov %eax, (.fixed_mode_compact2_end - .fixed_mode_compact2_start)
	stosd
	#
	mov %eax, (.fixed_mode_fixed_set_end - .fixed_mode_fixed_set_start)
	stosd
	.endif
	#
	mov %eax, (.inflate_decode_symbol_func_end - .inflate_decode_symbol_func)
	stosd
	#
	pop %rbp
	ret
