/*
 * File fpclassl.S.
 *
 * Copyright (C) 2003 Martin Str@"omberg <ams@ludd.luth.se>.
 *
 * This software may be used freely so long as this copyright notice is
 * left intact. There is no warranty on this software.
 *
 */

#include "fp-asm.h"
	
/*
 * Bits:	79 (sign), 78-64 (exponent), 63 (integer), 62-0 (fraction)
 * Zero:	+/-		0		0		0
 * Subnormal	+/-		0		0		!=0
 * Normal	+/-		!=0, <0xff	1		any
 * Infinity:	+/-		0xff		1		0
 * NaN:		any		0xff		1		!=0
 * Unnormal(1)	any		0		1		any
 * Unnormal(2)	any		!=0		0		any
 */

/*
 *	Stack:
 *	12(%esp):	high 16 bits of the long double
 *	8(%esp):	middle 32 bits of the long double
 *	4(%esp):	low 32 bits of the long double
 *	0(%esp):	return address
 */
	.globl	___fpclassifyld
___fpclassifyld:
	movl	8(%esp), %edx
	movl	12(%esp), %eax
	testl	$0x80000000, %edx
	jz	no_one_bit

	andl	$0x7fffffff, %edx /* Remove integer bit. */
	andl	$0x7fff, %eax
	jz	unnormal
	
	cmpl	$0x7fff, %eax
	je	all_ones_exponent

	movl	$FP_NORMAL, %eax
	ret

no_one_bit:
	andl	$0x7fff, %eax
	jnz	unnormal

	orl	4(%esp), %edx
	movl	$FP_ZERO, %eax
	jz	zero

	movl    $FP_SUBNORMAL, %eax
zero:	
	ret
	
unnormal:	
	movl	$FP_UNNORMAL, %eax
	ret

all_ones_exponent:
	orl	4(%esp), %edx
	movl    $FP_INFINITE, %eax
	jz	infinity
	
	movl    $FP_NAN, %eax
infinity:
	ret
