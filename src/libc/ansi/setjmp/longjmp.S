/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* This is file LONGJMP.S */
/*
** Copyright (C) 1993 DJ Delorie, 334 North Rd, Deerfield NH 03037-1110
**
** This file is distributed under the terms listed in the document
** "copying.dj", available from DJ Delorie at the address above.
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
**	jmp_buf:
**	 eax ebx ecx edx esi edi ebp esp eip fl cs ds es fs gs ss
**	 0   4   8   12  16  20  24  28  32  36 40 42 44 46 48 50
*/

	.globl	_longjmp /* jmp_buf, int */
_longjmp:
	movl	4(%esp),%edi	/* get jmp_buf */
	movl	8(%esp),%eax	/* store retval in j->eax */
	movl	%eax,0(%edi)

	movw	46(%edi),%fs
	movw	48(%edi),%gs
	movl	4(%edi),%ebx
	movl	8(%edi),%ecx
	movl	12(%edi),%edx
	movl	24(%edi),%ebp

	/* Now for some uglyness.  The jmp_buf structure may be ABOVE the
	   point on the new SS:ESP we are moving to.  We don't allow overlap,
	   but do force that it always be valid.  We will use ES:ESI for
	   our new stack before swapping to it.  */

	movw	50(%edi),%es
	movl	28(%edi),%esi
	subl	$28,%esi	/* We need 7 working longwords on stack */

	movl	60(%edi),%eax
	es
	movl	%eax,(%esi)	/* Exception pointer */

	movzwl	42(%edi),%eax
	es
	movl	%eax,4(%esi)	/* DS */

	movl	20(%edi),%eax
	es
	movl	%eax,8(%esi)	/* EDI */

	movl	16(%edi),%eax
	es
	movl	%eax,12(%esi)	/* ESI */

	movl	32(%edi),%eax
	es
	movl	%eax,16(%esi)	/* EIP - start of IRET frame */

	movl	40(%edi),%eax
	es
	movl	%eax,20(%esi)	/* CS */

	movl	36(%edi),%eax
	es
	movl	%eax,24(%esi)	/* EFLAGS */

	movl	0(%edi),%eax
	movw	44(%edi),%es

	movw	50(%edi),%ss
	movl	%esi,%esp

	popl	___djgpp_exception_state_ptr
	popl	%ds
	popl	%edi
	popl	%esi

	iret			/* actually jump to new cs:eip loading flags */
