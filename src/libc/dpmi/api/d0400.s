/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#define USE_ESI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_get_version)
	ENTER

	DPMI(0x0400)

	movl	ARG1, %esi
	movb	%ah, (%esi)
	movb	%al, 1(%esi)
	movw	%bx, 2(%esi)
	movb	%cl, 4(%esi)
	movb	%dh, 5(%esi)
	movb	%dl, 6(%esi)

	xorl	%eax,%eax

	LEAVE
