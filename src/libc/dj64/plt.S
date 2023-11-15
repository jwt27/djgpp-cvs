#include "stubinfo.h"
#include "plt.h"

.text

__plt_open: .quad 0
PLT_OPEN_CMD = 0
PLT_CLOSE_CMD = 1
__plt_call: .quad 0
__plt_ctrl: .quad 0
__plt_handle: .long 0
AUX_CORE = 0
AUX_USER = 1
ext_nm: .asciz "DJ64"

.macro call_plt
    movl __plt_handle, %eax
    lcalll *__plt_call
.endm

.macro asmcfunc nm,num
    .global \nm
    \nm:
    pushl %ebp
    movl %esp, %ebp
    pushal
    movl %esp, %edx
    movl \num, %ecx
    movl $AUX_CORE, %ebx
    call_plt
    popal
    popl %ebp
    ret
.endm

#include "plt.inc"

.global dj64_plt_call
dj64_plt_call:
    movl $AUX_USER, %ebx
    call_plt
    ret

.global dj64_plt_ctrl
dj64_plt_ctrl:
    movl $AUX_USER, %ebx
    movl __plt_handle, %eax
    lcalll *__plt_ctrl
    ret

sel: .word 0
SHM_REQ_LEN = 0
SHM_RET_LEN = 4
SHM_HANDLE = 8
SHM_LINEAR = 0xc
SHM_NAME = 0x10
shm_block: .fill 0x1b, 1, 0

.global plt_init
plt_init:
    movl $0xa00, %eax
    movl $ext_nm, %esi
    int $0x31
    jc 2f
    orb %al, %al
    jnz 2f
    movl %es, %eax
    movl %eax, __plt_open + 4
    movl %edi, __plt_open

    movl %fs:STUBINFO_PAYLOAD_SIZE, %ecx
    orl %ecx, %ecx
    jz 2f
    addl $(4096-1), %ecx
    andl $~(4096-1), %ecx
    movl %ecx, shm_block + SHM_REQ_LEN
    movw %fs, shm_block + SHM_NAME + 4
    movl $STUBINFO_ARGV0, shm_block + SHM_NAME
    /* alloc mem */
    pushw %ds
    popw %es
    movl $shm_block, %edi
    movl $0xd00, %eax
    int $0x31
    jc 2f
    /* alloc desc */
    movl $0, %eax
    movl $1, %ecx
    int $0x31
    /* set base */
    movl %eax, %ebx
    movw shm_block + SHM_LINEAR + 2, %cx
    movw shm_block + SHM_LINEAR, %dx
    movl $7, %eax
    int $0x31
    /* set limit */
    movw %fs:STUBINFO_PAYLOAD_SIZE, %dx
    decw %dx
    movw %fs:STUBINFO_PAYLOAD_SIZE+2, %cx
    movl $8, %eax
    int $0x31
    movw %bx, sel
    /* read payload */
    pushl %ds
    movw %bx, %ds
    movl %fs:STUBINFO_SELF_FD, %ebx
    movl %fs:STUBINFO_PAYLOAD_SIZE, %ecx
    movl $0, %edx
    movl $0x3f00, %eax
    int $0x21
    popl %ds
    jc 2f
    /* unmap shm */
    movw shm_block + SHM_LINEAR + 2, %bx
    movw shm_block + SHM_LINEAR, %cx
    movl $0x801, %eax
    int $0x31
    /* free desc */
    movw sel, %bx
    movl $1, %eax
    int $0x31

    // now open lib
    movl $0, %eax
    movl $PLT_OPEN_CMD, %ebx
    movw shm_block + SHM_HANDLE, %di
    movw shm_block + SHM_HANDLE + 2, %si
    movl %fs:STUBINFO_PAYLOAD_SIZE, %ecx
    lcalll *__plt_open
    jc 2f
    movl %eax, __plt_handle
    movl %es, %eax
    movl %eax, __plt_call + 4
    movl %edi, __plt_call
    movl %eax, __plt_ctrl + 4
    movl %esi, __plt_ctrl

    /* free mem */
    movl $0xd01, %eax
    movw shm_block + SHM_HANDLE, %di
    movw shm_block + SHM_HANDLE + 2, %si
    int $0x31
    jc 2f

1:
    ret
2:
    // TODO: print error
    movb $0x4c, %ah
    int $0x21
    jmp 1b

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
