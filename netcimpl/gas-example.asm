	bits 64
	global _start
	section .text
	
_start:
	mov rax, 0x02000004
	mov rdi, 1         
	mov rsi, message   
	mov rdx, 13        
	int 0x80 
	
	mov eax, 1
	mov ebx, 0
	int 0x80
	
	section .data
message: db "Hello, World", 10
