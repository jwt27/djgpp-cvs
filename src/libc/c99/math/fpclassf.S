/*
 * File fpclassf.S.
 *
 * Copyright (C) 2003 Martin Str@"omberg <ams@ludd.luth.se>.
 *
 * This software may be used freely so long as this copyright notice is
 * left intact. There is no warranty on this software.
 *
 */

#include "fp-asm.h"
		
/*
 * Bits:	31 (sign), 30-23 (exponent), 22-0 (fraction)
 * Zero:	+/-		0		0
 * Subnormal	+/-		0		!=0
 * Normal	+/-		!=0, <0xff	any
 * Infinity:	+/-		0xff		0
 * NaN:		any		0xff		!=0
 */

	.globl	___fpclassifyf
___fpclassifyf:
	movl	4(%esp), %eax
	movl	%eax, %edx
	andl	$0x7f800000, %eax
	jz	zero_exponent

	cmpl	$0x7f800000, %eax
	je	all_ones_exponent

	movl	$FP_NORMAL, %eax
	ret

zero_exponent:
	movl    $FP_ZERO, %eax
	testl   $0x7fffff, %edx
	jz	zero

	movl	$FP_SUBNORMAL, %eax
zero:
	ret
	
all_ones_exponent:
	movl    $FP_INFINITE, %eax
	testl	$0x7fffff, %edx
	jz	infinity
	
	movl    $FP_NAN, %eax
infinity:
	ret
