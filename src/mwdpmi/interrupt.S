/* ------------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                         */
/* Distributed under the GPL, see COPYING for details.                       */
/* ------------------------------------------------------------------------- */
/* FIXME: all hardware interrupts currently taken as software ditto.  */
	
#include "defines.h"
#define NOT_C_CODE
#include "dpmi.h"

.globl _interrupt_entries
_interrupt_entries:

/* 00 thru 0f */
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
/* 10 thru 1f */
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
	.word 0xe866, softint_or_except - (. + 2)
/* 20 thru 2f */
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
/* 30 thru 3f */
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
/* 40 thru 4f */
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
/* 50 thru 5f */
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
/* 60 thru 6f */
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
/* 70 thru 7f */
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
/* 80 thru 8f */
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
/* 90 thru 9f */
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
/* a0 thru af */
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
/* b0 thru bf */
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
/* c0 thru cf */
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
/* d0 thru df */
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
/* e0 thru ef */
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
/* f0 thru ff */
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
	.word 0xe866, softint - (. + 2)
/* ------------------------------------------------------------------------- */
.globl _reflect
_reflect:

/* 00 thru 0f */
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
/* 10 thru 1f */
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
/* 20 thru 2f */
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
/* 30 thru 3f */
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
/* 40 thru 4f */
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
/* 50 thru 5f */
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
/* 60 thru 6f */
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
/* 70 thru 7f */
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
/* 80 thru 8f */
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
/* 90 thru 9f */
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
/* a0 thru af */
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
/* b0 thru bf */
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
/* c0 thru cf */
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
/* d0 thru df */
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
/* e0 thru ef */
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
/* f0 thru ff */
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
	.word 0xe866, reflect_to_real - (. + 2)
/* ------------------------------------------------------------------------- */
/* These entry points MUST be located within the first 64K of the code
   segment, or else the 16-bit addressing mode used in the table will
   cause, ahem, peculiar results.  */

/* If we go here it means that a software interrupt occurred in the client.
   Exceptions, etc., don't go here.  */
.align 2
softint:
	subl	$2, %esp
	xchgl	%ebp, (%esp)
	shrl	$16, %ebp
	subl	$_interrupt_entries + 4, %ebp
	addl	%ebp, %ebp	/* Item size now 8 */
	cs
	addl	_current_software_interrupt_table, %ebp
	cs
	pushl	(%ebp)		/* EIP */
	cs
	movl	4(%ebp), %ebp	/* get CS */
	xchgl	%ebp, 4(%esp)
	lret
/* ------------------------------------------------------------------------- */
/* This relies on IOPL being 3.  Can be fixed to work with any non-zero
   value.  */

.align 2
softint_or_except:
	jmp	softint
	pushw	%ax
	movb	$4 + 8 + 1(%esp), %al
	andb	$(3 << (FLAG_IOPL - 8)), %al
	cmpb	$(3 << (FLAG_IOPL - 8)), %al
	movw	(%esp), %ax
	jne	except_with_error_code

	
except_with_error_code:
/* ------------------------------------------------------------------------- */
.align 2
reflect_to_real:
	cli
	subl	$2, %esp
	xchgl	%ebp, (%esp)
	shrl	$16, %ebp
	subl	$_reflect + 4, %ebp
	/* Now %ebp == 4 * no */

	push	%ds
	push	%es
	push	%fs
	push	%gs

	push	%ss
	pop	%ds

	movl	-SERVER_LINEAR(%ebp), %ebp
	pushl	%ebp
	/* Now %ebp == cs:ip of real mode interrupt entry.  */

	/* Set up real mode stack.  */
	movl	_current_client_low, %ebp
	addl	$REAL_MODE_STACK_SIZE - 4, %ebp
	movl	%esp, (%ebp)				/* esp */
	addl	$4, (%ebp)
	movl	$reflect_to_real_back_again, -4(%ebp)	/* eip */
	movl	$0, -8(%ebp)				/* fake cs:ip */
	movw	$0x3002, -10(%ebp)			/* flags */
	pushw	_code_seg
	movl	$_go32, -14(%ebp)			/* ip - note order */
	popw	-12(%ebp)				/* cs */

	/* Set up parameters for prot->real switch */
	pushl	$0x3002				/* EFLAGS */
	pushl	4(%esp)				/* CS in high word */
	shrl	$16, (%esp)
	pushl	8(%esp)				/* IP in low word */
	andl	$0xffff, (%esp)
	pushl	$0				/* DS */
	pushl	$0				/* ES */
	pushl	$0				/* FS */
	pushl	$0				/* GS */
	pushl	_current_client_handle		/* SS */
	pushl	$REAL_MODE_STACK_SIZE - 18	/* ESP */
	movl	14 * 4(%esp), %ebp
	.byte 0x67,0x66,0xff,0x2e
	.long	_goreal_addr

reflect_to_real_back_again:
	addl	$8 * 4, %esp

	xchgl	%eax, (%esp)
	andl	$FLAGS_USER, %eax
	andl	$~FLAGS_USER, 32(%esp)
	orl	%eax, 32(%esp)
	pop	%eax

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	addl	$4, %esp		/* Drop saved ebp */

	pushfl
	andb	$0xbf, 1(%esp)		/* Clear NT */
	popfl
	iretl
/* ------------------------------------------------------------------------- */
/* Standard handler for Int 0x21.  */

.align 2
.global _prot_21_entry
_prot_21_entry:
	cmpb	$DOS_EXIT, %ah
	jne	_reflect + (4 * 0x21)
	cli
	pushl	%eax
	mov	%ss, %ax
	mov	%ax, %ds
	mov	%ax, %es
	call	_client_terminate
/* Never returns.  */
/* ------------------------------------------------------------------------- */
/* Standard handler for Int 0x2f.  */

.align 2
.global _prot_2f_entry
_prot_2f_entry:
	cmpw	$DPMI_GET_CPU_MODE, %ax
	je	cpumode
	cmpw	$DPMI_GET_API_ENTRY, %ax
	jne	_reflect + (4 * 0x2f)

/* We don't support any extensions right now.  */
	iretl

/* We're in protected mode.  */
cpumode:
	xorw	%ax, %ax
	iretl
/* ------------------------------------------------------------------------- */
/* Standard handler for Int 0x31.  */

.align 2
.global _prot_31_entry
_prot_31_entry:
	/* Push SS/ESP/EFLAGS/CS/EIP done by gate assuming called from
	   lower privilege.  */
	push	%ds
	push	%es
	push	%fs
	push	%gs
	pushal

	xorl	%eax, %eax		/* Zero high word of selectors.  */
	movw	%ax, 32 + 2(%esp)
	movw	%ax, 36 + 2(%esp)
	movw	%ax, 40 + 2(%esp)
	movw	%ax, 44 + 2(%esp)
	movw	%ax, 52 + 2(%esp)
	movw	%ax, 64 + 2(%esp)

	mov	%ss, %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs
	cli
	cld
	call	_prot_31
	popal
	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	iretl
/* ------------------------------------------------------------------------- */
