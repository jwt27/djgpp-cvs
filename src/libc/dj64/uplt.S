#include <libc/asmdefs.h>
.include "plt_defs.inc"

.bss

__plt_call: .quad 0
__plt_handle: .long 0

.text

.global _start
_start:
    movl 4(%esp), %eax
    movl %eax, __plt_call
    movl 8(%esp), %eax
    movl %eax, __plt_call + 4
    movl 12(%esp), %eax
    movl %eax,  __plt_handle
    ret
