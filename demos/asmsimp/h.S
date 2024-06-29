#include <libc/asmdefs.h>
#ifndef DJ64
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
