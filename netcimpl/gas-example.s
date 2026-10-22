.section .data                           # .data section declaration

msg:
    .ascii    "Hello, World!\n" # Declare a label "msg" which has
                                # our string we want to print.

    len = . - msg               # "len" will calculate the current
                                # offset minus the "msg" offset.
                                # this should give us the size of
                                # "msg".

.section .text                           # .text section declaration

.globl  _start                 # we must export the entry point to the
                                # ELF linker or loader. Conventionally,
                                # they recognize _start as their entry
                                # point but this can be overridden with
                                # ld -e "label_name" when linking.
.type _start, @function

_start:
    ######################################
    # syscall - write(1, msg, len);
    ######################################
    mov    $4,%eax              # 4 = Syscall number for Write()

    mov    $1,%ebx              # File Descriptor to write to
                                # In this case: STDOUT is 1

    mov    $msg,%ecx            # String to write. A pointer to
                                # the variable 'msg'

    mov    $len,%edx            # The length of string to print
                                # which is 14 characters

    int    $0x80                # Poke the kernel and tell it to run the
                                # write() call we set up

    ######################################
    # syscall - exit(0);
    ######################################
    mov    $1,%al               # 1 = Syscall for Exit()
    mov    $0,%ebx              # The status code we want to provide.
    int    $0x80                # Poke kernel. This will end the 

start_end:
    ret
