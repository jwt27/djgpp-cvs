#ifdef DJ64
#define RET_I lretl
#else
#define RET_I ret
#endif

.text
    .global _hello_asm
_hello_asm:
    call _copy_msg
    movl $0x900, %eax
    movl $_msg, %edx
    int $0x21
    RET_I

.bss
    .global _msg
_msg: .space 128

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
