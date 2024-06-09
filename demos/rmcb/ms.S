#include "asm.h"
.include "asm.inc"

.text
.global _my_mouse_handler
_my_mouse_handler:
    pusha
    handler_prolog SIGSTK_LEN
    call _do_mouse
    restore_stack
    popa
    iretl

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
