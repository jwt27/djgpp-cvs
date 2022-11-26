__plt_open: .quad 0
__plt_call: .quad 0
__plt_handle: .long 0
ext_nm: .asciz "DJ64"
lib_nm: .asciz "libdj64.so"

.macro asmcfunc nm,num
    .global \nm
    \nm:
    movl __plt_handle, %eax
    movl $0, %ebx        // reserve
    movl \num, %ecx
    movl %esp, %edx
    addl $4, %edx
    lcalll *__plt_call
.endm

.macro asmsym nm
\nm\()_ptr:
    .long \nm
.endm

#include "../plt.inc"
#include "../plt_asms.inc"

.global plt_init
plt_init:
    movl $0x168a, %eax
    movl $ext_nm, %esi
    int $0x2f
    jc 2f
    orb %al, %al
    jnz 2f
    movl %es, %eax
    movl %eax, __plt_open + 4
    movl %edi, __plt_open
    // now open lib
    movl $lib_nm, %esi
    lcalll *__plt_open
    jc 2f
    movl %eax, __plt_handle
    movl %es, %eax
    movl %eax, __plt_call + 4
    movl %edi, __plt_call
1:  ret
2:
    // TODO: print error
    movb $0x4c, %ah
    int $0x21
    jmp 1b
