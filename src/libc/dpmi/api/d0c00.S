/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_install_resident_service_provider_callback)
	ENTER

	movl	ARG1, %edi
	DPMI(0x0c00)
	xorl	%eax,%eax

	LEAVE
