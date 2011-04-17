/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
	.text
	.globl	___modfl
___modfl:
	pushl %ebp
	movl %esp,%ebp
	subl $4,%esp
	fldt 8(%ebp)
	movl 20(%ebp),%eax
	fnstcw -2(%ebp)
	movw -2(%ebp),%dx
	orb $0x0c,%dh
	movw %dx,-4(%ebp)
	fldcw -4(%ebp)
	fld %st(0)
	frndint
	fldcw -2(%ebp)
	fld %st(0)
	fstpt (%eax)
	fucom %st(1)
	fnstsw -2(%ebp)
	movw -2(%ebp),%dx
	andb $0x44,%dh
	xorb $0x40,%dh
	jnz 0f
	fxam
	fnstsw -2(%ebp)
	fstp %st(0)
	fstp %st(0)
	fldz
	movw -2(%ebp),%dx
	andb $0x02,%dh
	jz 1f
	fchs
	jmp 1f
0:
	fsubrp %st,%st(1)
1:
	leave
	ret
