/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
	.file	"overflow.S"
	.data
	.balign	8
	.comm	exception_stack, 8000
	.balign	8
overflow_state_buf:		/* jmp_buf */
	.long	0, 0, 0, 0	/* eax, ebx, ecx, edx */
	.long	0, 0, 0, 0	/* esi, edi, ebp, esp */
	.globl	___djgpp_stack_overflow_eip
___djgpp_stack_overflow_eip:
	.long	0, 0		/* eip, eflags */
	.word	0, 0, 0, 0, 0, 0/* cs, ds, es, fs, gs, ss */
	.long	0, 0, 0		/* sigmask, signum, exception_ptr */
	.text
errstring:
	.ascii	"Out of stack. \0"
	.balign	16,,7
	.globl	___djgpp_stack_overflow_exit
___djgpp_stack_overflow_exit:
	movl	%eax, overflow_state_buf
	movl	$overflow_state_buf, %eax

	/* change stack as soon as possible */
	movl	%esp, 28(%eax)
	movl	$exception_stack + 8000, %esp

	/* eax already stored */
	movl	%ebx, 4(%eax)
	movl	%ecx, 8(%eax)
	movl	%edx, 12(%eax)
	movl	%esi, 16(%eax)
	movl	%edi, 20(%eax)
	movl	%ebp, 24(%eax)
	/* esp already stored */
	/* eip stored before jumping here */
	pushfl
	popl	36(%eax)
	movw	%cs, 40(%eax)
	movw	%ds, 42(%eax)
	movw	%es, 44(%eax)
	movw	%fs, 46(%eax)
	movw	%gs, 48(%eax)
	movw	%ss, 50(%eax)

	/* fake exception number like in __djgpp_traceback_exit,
	   0x7e == 0x7a + 1 + SIGSEGV - SIGABRT */
	movl	$0x7e, 56(%eax)

	movl	$overflow_state_buf, ___djgpp_exception_state_ptr

	/* print error message */
	pushl	$14
	pushl	$errstring
	pushl	$2
	call	__write

	/* 291 == SIGSEGV */
	pushl	$291

	/* this does not return */
	call	___djgpp_traceback_exit
