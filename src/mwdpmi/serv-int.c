/* ---------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                 */
/* Distributed under the GPL, see COPYING for details.                    */
/* ---------------------------------------------------------------------- */

#include "code32.h"

/* This code is re-entrant, provided of course that the stacks in use are
   not overwritten.  */

void
server_goto (int frametype, __dpmi_regs *regs)
{
  if (frametype != 0)
    {
      word32 esp = regs->x.sp, sslin = regs->x.ss << 4;
      word32 back_esp, back_eip;

      /* Where do we return to?  */
      asm ("movl $server_goto_resume, %0" : "=g" (back_eip));
      asm ("movl %%esp, %0" : "=g" (back_esp));

      /* Push the "result" of a call to _go32.  "12" is three longs pushed
	 in the code below.  */
      *((word32 *)LINEAR_TO_PTR (sslin + (esp -= 4))) = back_esp - 12;
      *((word32 *)LINEAR_TO_PTR (sslin + (esp -= 4))) = back_eip;
      *((word32 *)LINEAR_TO_PTR (sslin + (esp -= 4))) = 0; /*cs:ip*/

      switch (frametype)
	{
	case 1: /* Push iret frame.  */
	  *((word16 *)LINEAR_TO_PTR (sslin + (esp -= 2))) =
	    regs->x.flags & ~((1 << FLAG_IF) | (1 << FLAG_TF));
	  /* Fall through.  */
	case 2: /* Push far ret frame.  */
	  *((word16 *)LINEAR_TO_PTR (sslin + (esp -= 2))) = code_seg;
	  *((word16 *)LINEAR_TO_PTR (sslin + (esp -= 2))) = (word16)&go32;
	  break;
	}
      regs->x.sp = esp;
    }

  asm volatile
    ("
        pushl	%%ebp
	pushfl
	movl	%0, %%ebp
        pushl	%%ebp                " /* save on stack for easy access */ "

	xorl	%%eax, %%eax
	movw	32(%%ebp), %%ax      " /* flags */ "
	pushl	%%eax
	movw	44(%%ebp), %%ax      " /* cs */ "
	pushl	%%eax
	movw	42(%%ebp), %%ax      " /* ip */ "
	pushl	%%eax
	movw	36(%%ebp), %%ax      " /* ds */ "
	pushl	%%eax
	movw	34(%%ebp), %%ax      " /* es */ "
	pushl	%%eax
	movw	38(%%ebp), %%ax      " /* fs */ "
	pushl	%%eax
	movw	40(%%ebp), %%ax      " /* gs */ "
	pushl	%%eax
	movw	48(%%ebp), %%ax      " /* ss */ "
	pushl	%%eax
	movw	46(%%ebp), %%ax      " /* sp */ "
	pushl	%%eax

	movl	28(%%ebp), %%eax
	movl	16(%%ebp), %%ebx
	movl	24(%%ebp), %%ecx
	movl	20(%%ebp), %%edx
	movl	4(%%ebp), %%esi
	movl	0(%%ebp), %%edi
	movl	8(%%ebp), %%ebp

	.byte 0x67,0x66,0xff,0x2e
	.long	_goreal_addr

server_goto_resume:
	pushl	%%ebp
	movl	10 * 4(%%esp), %%ebp

	movl	%%eax, 28(%%ebp)
	movl	%%ebx, 16(%%ebp)
	movl	%%ecx, 24(%%ebp)
	movl	%%edx, 20(%%ebp)
	movl	%%esi, 4(%%ebp)
	movl	%%edi, 0(%%ebp)
	popl	8(%%ebp)

	popl	%%eax
	movw	%%ax, 46(%%ebp)      " /* sp */ "
	popl	%%eax
	movw	%%ax, 48(%%ebp)      " /* ss */ "
	popl	%%eax
	movw	%%ax, 40(%%ebp)      " /* gs */ "
	popl	%%eax
	movw	%%ax, 38(%%ebp)      " /* fs */ "
	popl	%%eax
	movw	%%ax, 34(%%ebp)      " /* es */ "
	popl	%%eax
	movw	%%ax, 36(%%ebp)      " /* ds */ "
	addl	$8, %%esp            " /* eip, then cs */ "
	popl	%%eax
	movw	%%ax, 32(%%ebp)      " /* flags */ "

	popl	%%ebp                " /* regs pointer */ "
	popfl
	popl	%%ebp"
     : /* No output */
     : "m" (regs)
     : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi");
}
/* ---------------------------------------------------------------------- */
void
server_int (int no, __dpmi_regs *regs)
{
  /* Find out where we are going.  */
  regs->x.ip = *((word16 *) LINEAR_TO_PTR (no * 4));
  regs->x.cs = *((word16 *) LINEAR_TO_PTR (no * 4 + 2));
  regs->x.ss = code_seg;
  regs->x.sp = (word32)&server_stack;
  regs->x.flags = 0x3002;

  server_goto (1, regs);
}
/* ---------------------------------------------------------------------- */
void
server_jump (int cs, int ip, __dpmi_regs *regs)
{
  regs->x.cs = cs;
  regs->x.ip = ip;
  regs->x.ss = code_seg;
  regs->x.sp = (word32)&server_stack;
  while (1) server_goto (0, regs);
}
/* ---------------------------------------------------------------------- */
