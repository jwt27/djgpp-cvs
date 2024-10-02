AUX_CORE = 0
AUX_USER = 1

.macro call_plt
    movl __plt_handle, %eax
    movl $0, %esi  # reserved
    lcalll *__plt_call
.endm

.text

.global dj64_plt_call
dj64_plt_call:
    movl $AUX_USER, %ebx
    call_plt
    ret
