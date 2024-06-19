#ifdef DJ64
#define RET_I lretl
#else
#define RET_I ret
#endif

.text
    .global _hello_asm
_hello_asm:
    movl $0x900, %eax
    movl $msg, %edx
    int $0x21
    RET_I

.data
msg: .asciz "Hello from ASM\n$"

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
