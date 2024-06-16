/*
 *  dj64 - 64bit djgpp-compatible tool-chain
 *  Copyright (C) 2021-2024  @stsp
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stubinfo.h"
#include "plt.h"

.bss

__plt_open: .quad 0
PLT_OPEN_CMD = 0
PLT_CLOSE_CMD = 1
__plt_call: .quad 0
__plt_ctrl: .quad 0
.global __plt_handle
__plt_handle: .long 0
AUX_CORE = 0
AUX_USER = 1

sel: .word 0
SHM_REQ_LEN = 0
SHM_RET_LEN = 4
SHM_HANDLE = 8
SHM_LINEAR = 0xc
SHM_NAME = 0x10
SHM_FLAGS = 0x16
SHM_HANDLE2 = 0x18
shm_block: .fill 0x1c, 1, 0
shm_block2: .fill 0x1c, 1, 0
handle: .long 0
maddr: .long 0
SHM_NOEXEC = 1
SHM_EXCL = 2
SHM_NEW_NS = 4
SHM_NS = 8
SHM_FLG_MASK = (SHM_NOEXEC | SHM_EXCL | SHM_NEW_NS | SHM_NS)
/* leave bits 4 and 5 reserved */
OPEN_FLG_SHIFT = 6

.data

ext_nm: .asciz "DJ64"
err_str: .asciz "dj64: load failure\r\n$"

.text

.macro call_plt
    movl __plt_handle, %eax
    movl $0, %esi  // reserved
    lcalll *__plt_call
.endm

.macro asmcfunc_n nm,num
    .global _\nm
    _\nm:
    pushl %ebp
    movl %esp, %ebp
    pushal
    movl %esp, %edx
    movl $\num, %ecx
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

.global plt_init
plt_init:
    pushl %es

    movl $0xa00, %eax
    movl $ext_nm, %esi
    int $0x31
    jc 2f
    orb %al, %al
    jnz 2f
    movl %es, %eax
    movl %eax, __plt_open + 4
    movl %edi, __plt_open

.macro mk_shm b,offs,size,name,err,flg,h2
    movl %fs:\size, %ecx
    orl %ecx, %ecx
    jz \err
    xorb %al, %al
    cmpb %fs:\name, %al
    je \err
    movl %ecx, \b + SHM_REQ_LEN
    movw %fs, \b + SHM_NAME + 4
    movl $\name, \b + SHM_NAME
    movzbw %fs:\flg, %ax
    andb $SHM_FLG_MASK, %al
    movw %ax, \b + SHM_FLAGS
    movl \h2, %eax
    movl %eax, \b + SHM_HANDLE2
    /* alloc mem */
    pushw %ds
    popw %es
    movl $\b, %edi
    movl $0xd00, %eax
    int $0x31
    jc \err
    /* alloc desc */
    movl $0, %eax
    movl $1, %ecx
    int $0x31
    movw %ax, sel
    /* set base */
    movw %ax, %bx
    movw \b + SHM_LINEAR + 2, %cx
    movw \b + SHM_LINEAR, %dx
    movl $7, %eax
    int $0x31
    /* set limit */
    movw sel, %bx
    movw %fs:\size, %dx
    decw %dx
    movw %fs:\size + 2, %cx
    movl $8, %eax
    int $0x31
    /* seek fd */
    movl %fs:STUBINFO_SELF_FD, %ebx
    movzwl %fs:\offs, %edx
    movzwl %fs:\offs + 2, %ecx
    movl $0x4200, %eax
    int $0x21
    /* read payload */
    pushl %ds
    movw sel, %ds
    movl %fs:STUBINFO_SELF_FD, %ebx
    movl %fs:\size, %ecx
    movl $0, %edx
    movl $0x3f00, %eax
    int $0x21
    popl %ds
    jc 2f
    /* unmap shm */
    movw \b + SHM_LINEAR + 2, %bx
    movw \b + SHM_LINEAR, %cx
    movl $0x801, %eax
    int $0x31
.endm

    mk_shm shm_block,STUBINFO_PAYLOAD_OFFS,STUBINFO_PAYLOAD_SIZE,STUBINFO_ARGV0,2f,STUBINFO_FLAGS,$0
    mk_shm shm_block2,STUBINFO_PAYLOAD2_OFFS,STUBINFO_PAYLOAD2_SIZE,STUBINFO_PAYLOAD2_NAME,5f,STUBINFO_FLAGS+1,shm_block+SHM_HANDLE

5:
    // now open lib
    movl $0, %eax
    movl $PLT_OPEN_CMD, %ebx
    movw shm_block + SHM_HANDLE, %di
    movw shm_block + SHM_HANDLE + 2, %si
    movzbl %fs:STUBINFO_FLAGS, %ecx
    shrl $OPEN_FLG_SHIFT, %ecx
    lcalll *__plt_open
    jc 2f
    movl %eax, __plt_handle
    movl %es, %eax
    movl %eax, __plt_call + 4
    movl %edi, __plt_call
    movl %eax, __plt_ctrl + 4
    movl %esi, __plt_ctrl

    movl %fs:STUBINFO_SELF_SIZE, %ecx
    orl %ecx, %ecx
    jz 2f
    addl $(4096-1), %ecx
    andl $~(4096-1), %ecx
    movl %ecx, %ebx
    shrl $16, %ebx
    andl $0xffff, %ecx
    /* alloc mem */
    movl $0x501, %eax
    int $0x31
    jc 2f
    movw %cx, maddr
    movw %bx, maddr + 2
    movw %di, handle
    movw %si, handle + 2
    /* set base */
    movw maddr, %dx
    movw maddr + 2, %cx
    movw sel, %bx
    movl $7, %eax
    int $0x31
    /* set limit */
    movw sel, %bx
    movw %fs:STUBINFO_SELF_SIZE, %dx
    decw %dx
    movw %fs:STUBINFO_SELF_SIZE + 2, %cx
    movl $8, %eax
    int $0x31
    /* seek fd */
    movl %fs:STUBINFO_SELF_FD, %ebx
    movzwl %fs:STUBINFO_SELF_OFFS, %edx
    movzwl %fs:STUBINFO_SELF_OFFS + 2, %ecx
    movl $0x4200, %eax
    int $0x21
    /* read payload */
    pushl %ds
    movw sel, %ds
    movl %fs:STUBINFO_SELF_FD, %ebx
    movl %fs:STUBINFO_SELF_SIZE, %ecx
    movl $0, %edx
    movl $0x3f00, %eax
    int $0x21
    popl %ds
    jc 2f
    /* close file */
    movl %fs:STUBINFO_SELF_FD, %ebx
    movl $0x3e00, %eax
    int $0x21
    /* free desc */
    movw sel, %bx
    movl $1, %eax
    int $0x31
    /* call manager */
    movl maddr, %ebx
    movl %fs:STUBINFO_SELF_SIZE, %ecx
    movl %fs:STUBINFO_MEM_BASE, %edx
    pushal
    movl %esp, %edx
    movl __plt_handle, %eax
    movl $AUX_CORE, %ebx
    movl $DL_SET_SYMTAB, %ecx
    movl %cs, %esi
    lcalll *__plt_ctrl
    popal
    /* free mem */
    movl $0x502, %eax
    movw handle, %di
    movw handle + 2, %si
    int $0x31
    jc 2f
    popl %es

1:
    ret
2:
    popl %es
    movl $err_str, %edx
    movl $0x900, %eax
    int $0x21
    movl $0x4c01, %eax
    int $0x21
    jmp 1b


.global plt_done
plt_done:
    // close lib
    movl __plt_handle, %eax
    movl $PLT_CLOSE_CMD, %ebx
    movw shm_block + SHM_HANDLE, %di
    movw shm_block + SHM_HANDLE + 2, %si
    lcalll *__plt_open
    /* free mem */
    movl $0xd01, %eax
    movw shm_block + SHM_HANDLE, %di
    movw shm_block + SHM_HANDLE + 2, %si
    int $0x31
    /* free mem */
    movl $0xd01, %eax
    movw shm_block2 + SHM_HANDLE, %di
    movw shm_block2 + SHM_HANDLE + 2, %si
    int $0x31
    ret

.bss

.global _dosobj_page
_dosobj_page: .space 4096

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
