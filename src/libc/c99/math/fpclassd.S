/*
 * File fpclassd.S.
 *
 * Copyright (C) 2003 Martin Str@"omberg <ams@ludd.luth.se>.
 *
 * This software may be used freely so long as this copyright notice is
 * left intact. There is no warranty on this software.
 *
 */

#include "fp-asm.h"
	
/*
 * Bits:	63 (sign), 62-52 (exponent), 51-0 (fraction)
 * Zero:	+/-		0		0
 * Subnormal	+/-		0		!=0
 * Normal	+/-		!=0, <0xff	any
 * Infinity:	+/-		0xff		0
 * NaN:		any		0xff		!=0
 */

/*
 *	Stack:
 *	8(%esp):	high 32 bits of the double
 *	4(%esp):	low 32 bits of the double
 *	0(%esp):	return address
 */
	.globl	___fpclassifyd
___fpclassifyd:
	movl	8(%esp), %eax
	movl	%eax, %edx
	andl	$0x7ff00000, %eax
	jz	zero_exponent

	cmpl	$0x7ff00000, %eax
	je	all_ones_exponent

	movl	$FP_NORMAL, %eax
	ret

zero_exponent:
	andl	$0xfffff, %edx
	orl	4(%esp), %edx
	movl    $FP_ZERO, %eax
	jz	zero

	movl	$FP_SUBNORMAL, %eax
zero:
	ret

all_ones_exponent:
	andl	$0xfffff, %edx
	orl	4(%esp), %edx
	movl    $FP_INFINITE, %eax
	jz	infinity
	
	movl    $FP_NAN, %eax
infinity:
	ret
