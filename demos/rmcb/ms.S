#include <libc/asmdefs.h>
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
