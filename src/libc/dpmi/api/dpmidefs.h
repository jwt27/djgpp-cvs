/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>

#define DPMI(x)		movw $(x),%ax; int $0x31; jnc L_noerror; movw %ax,___dpmi_error; movl $-1,%eax; jmp L_leave; L_noerror:
#define DPMIce(x)	movw $(x),%ax; int $0x31; movw %ax,___dpmi_error; jc L_error

	.comm	___dpmi_error,2

	.text
