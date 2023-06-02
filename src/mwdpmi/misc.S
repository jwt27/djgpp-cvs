/* ------------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                         */
/* Distributed under the GPL, see COPYING for details.                       */
/* ------------------------------------------------------------------------- */

/* Save the registers as extra arguments and call create_entry.  */
.align 2
.globl _client_create_entry
_client_create_entry:
	pushal
	call	_client_create
	popal
	nop
	testb	$1, 32(%esp)
	je	client_create_success
	.byte 0x67,0x66,0xff,0x2e
	.long	_goreal_addr
client_create_success:
	movl	0x08(%esp), %gs
	movl	0x0c(%esp), %fs
	movl	0x10(%esp), %es
	movl	0x14(%esp), %ds
	pushl	0x04 + 0(%esp)	/* SS */
	pushl	0x00 + 4(%esp)	/* ESP */
	pushl	0x20 + 8(%esp)	/* EFLAGS */
	pushl	0x1c + 12(%esp)	/* CS */
	pushl	0x18 + 16(%esp)	/* EIP */
	pushfl
	andb	$0xbf, 1(%esp) /* Clear NT */
	popfl
	iretl
/* ------------------------------------------------------------------------- */
.align 2
.globl _save_state
_save_state:
	lret
/* ------------------------------------------------------------------------- */
.align 2
.globl _raw_protect_to_real
_raw_protect_to_real:
	pushfl				/* EFLAGS */
	andl	$FLAGS_USER, (%esp)
	orl	$0x3002, (%esp)
	pushl	%esi			/* CS */
	pushl	%edi			/* EIP */
	pushl	%eax			/* DS */
	pushl	%ecx			/* ES */
	pushl	$0			/* FS */
	pushl	$0			/* GS */
	pushl	%edx			/* SS */
	pushl	%ebx			/* ESP */
	.byte 0x67,0x66,0xff,0x2e
	.long	_goreal_addr
/* ------------------------------------------------------------------------- */
.globl _callback_entry
_callback_entry:
	pushal
	jmp	_callback_handler
/* ------------------------------------------------------------------------- */

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
