#ifndef __ASM_H
#define __ASM_H

.macro ldlmr m,r
movl \m\()__plt,\r
movl %fs:(\r),\r
.endm

.macro stlcm c,m
pushl %ebx
movl \m\()__plt,%ebx
movl \c,%fs:(%ebx)
popl %ebx
.endm

#endif
