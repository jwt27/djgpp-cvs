/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dpmi.h>
#include <crt0.h>
#include <go32.h>
#include <signal.h>
#include <setjmp.h>
#include <debug/dbgcom.h>
#include <sys/exceptn.h>

extern char __libdbg_ident_string[];
static char *id = __libdbg_ident_string;

/* ARGSUSED */
char **
__crt0_glob_function(char *foo)
{
  id = 0;
  return 0;
}

ExternalDebuggerInfo edi;
TSS a_tss;
static jmp_buf jumper;

static int my_ds,my_cs,app_cs;
static jmp_buf load_state;

static int nset, breakhandle[4];

static int _DPMIsetBreak(unsigned short sizetype, unsigned vaddr)
{
	int handle;

	asm volatile(						       "\n\
	    movw   %1,%%dx						\n\
	    movl   %2,%%ecx						\n\
	    movl   %%ecx,%%ebx						\n\
	    shrl   $16,%%ebx						\n\
	    movw   $0x0b00,%%ax						\n\
	    int    $0x31						\n\
	    jnc    3f							\n\
	    xorl   %%ebx,%%ebx						\n\
	    decl   %%ebx						\n\
	    jmp    1f							\n\
3:          movzwl %%bx,%%ebx						\n\
1:	    movl   %%ebx,%0						\n\
	    "
	    : "=g" (handle)			/* outputs */
	    : "g" (sizetype), "g"  (vaddr)	/* inputs */
	    : "ax", "bx", "cx", "dx"		/* regs used */
	);
	return handle;
}

static int _DPMIcancelBreak(int handle)
{
	unsigned state;

	asm volatile(						       "\n\
	    movl   %1,%%ebx						\n\
	    movw   $0x0b02,%%ax						\n\
	    int    $0x31						\n\
	    jnc    2f							\n\
	    xorl   %%eax,%%eax                                          \n\
2:	    andl   $1,%%eax						\n\
	    pushl  %%eax						\n\
	    movw   $0x0b01,%%ax						\n\
	    int    $0x31						\n\
	    popl   %0							\n\
	    "
	    : "=g" (state)			/* outputs */
	    : "g"  (handle)			/* inputs */
	    : "ax", "bx"			/* regs used */
	);
	return state;
}

/* Can't be static because called in asm below; -O3 inlines if static */
void _set_break_DPMI(void);
void _set_break_DPMI(void)
{
  int i;
  unsigned extract;
  unsigned short sizetype;
  unsigned long vbase;
  
  if(__dpmi_get_segment_base_address(__djgpp_app_DS, &vbase) == -1)
    return;
  extract = edi.dr[7] >> 16;
  nset = 0;

  for(i=0;i<4;i++)
    if( (edi.dr[7] >> (i*2))&3 ) {		/* enabled? */
      sizetype = (extract >> (i*4)) & 3;    /* extract the type */
      if(sizetype == 3) sizetype = 2;       /* convert for DPMI brain damage */
      sizetype = (sizetype << 8) + ((extract >> (i*4+2)) & 3) + 1; /* & size */
      breakhandle[i] = _DPMIsetBreak(sizetype, edi.dr[i]+vbase);
      if(breakhandle[i] == -1)
        printf("Error allocating DPMI breakpoint at address 0x%08lx\n",edi.dr[i]);
      else
        nset++;
    } else
      breakhandle[i] = -1;
  return;
}

/* Can't be static because called in asm below; -O3 inlines if static */
void _clear_break_DPMI(void);
void _clear_break_DPMI(void)
{
  int i,bt;

  if(!nset) {
    edi.dr[6] = 0;
    return;
  }

  bt = 0;
  for(i=3;i>=0;i--) {
    bt = bt << 1;                             /* Shift for next bit */
    if(breakhandle[i] != -1)
      bt |= _DPMIcancelBreak(breakhandle[i]);  /* Set low bit if active */
  }

  edi.dr[6] = bt;
}

static __dpmi_paddr old_i31,old_i21;

static void hook_dpmi(void)
{
  __dpmi_paddr new_int;
  extern void i21_hook(void),i31_hook(void);

  __dpmi_get_protected_mode_interrupt_vector(0x21, &old_i21);
  __dpmi_get_protected_mode_interrupt_vector(0x31, &old_i31);

  asm("mov %%cs,%0" : "=g" (new_int.selector) );
  new_int.offset32 = (unsigned long)i21_hook;
  __dpmi_set_protected_mode_interrupt_vector(0x21, &new_int);
  new_int.offset32 = (unsigned long)i31_hook;
  __dpmi_set_protected_mode_interrupt_vector(0x31, &new_int);
}

/* BUGS: We ignore the exception handlers for the child process, so signals
   do not work.  We also disable the hooking of HW interrupts that might
   cause the HW-interrupt-to-limit exceptions, since they can never be fixed.
   Byproduct:  You can't debug code which hooks int 9, since it's keyboard
   routine never gets called.  Eventually, we should save the exception and
   interrupt hooks and then chain to them on the next execution.  Someday. */

/* Watch set selector base, if it is __djgpp_app_DS then reset breakpoints */
asm(						       			"\n\
	.text								\n\
	.align  2,0x90							\n\
_i31_hook:								\n\
	cmpw	$0x0203,%ax						\n\
	je	Lc31a							\n\
	cmpw	$0x0007,%ax						\n\
	je	Lc31b							\n\
	cmpw	$0x0205,%ax						\n\
	je	Lc31d							\n\
Lc31c:	.byte	0x2e							\n\
	ljmp	_old_i31						\n\
Lc31a:	iret								\n\
Lc31b:	.byte	0x2e							\n\
	cmpw	___djgpp_app_DS,%bx					\n\
	jne	Lc31c							\n\
	pushf								\n\
	.byte	0x2e							\n\
	lcall	_old_i31						\n\
	call	___djgpp_save_interrupt_regs				\n\
	call	__clear_break_DPMI					\n\
	call	__set_break_DPMI					\n\
	movl	___djgpp_exception_state_ptr,%eax			\n\
	pushl	(%eax)							\n\
	pushl	%eax							\n\
	call	_longjmp						\n\
Lc31d:	cmpb	$9,%bl							\n\
	je	Lc31a							\n\
	cmpb	$0x75,%bl						\n\
	je	Lc31a							\n\
	jmp	Lc31c							\n\
	.align  2,0x90							\n\
_i21_hook:								\n\
	cmpb	$0x4c,%ah						\n\
	je	Lc21							\n\
Lc21j:	.byte	0x2e							\n\
	ljmp	_old_i21						\n\
Lc21:	push	%eax							\n\
	movl	8(%esp),%eax						\n\
	cs								\n\
	cmpw	_app_cs,%ax						\n\
	pop	%eax							\n\
	jg	Lc21j							\n\
	call	___djgpp_save_interrupt_regs				\n\
	movl	___djgpp_exception_state_ptr,%esi			\n\
	movl	$0x21,56(%esi)						\n\
	movl	$_load_state,%edi					\n\
	movl	$43,%ecx						\n\
	rep								\n\
	movsl								\n\
	pushl	$1							\n\
	pushl	$_jumper						\n\
	call	_longjmp						\n\
	"
	);

/*	movw	%cs:__go32_info_block+26, %fs				\n\
	.byte	0x64							\n\
	movw	$0x7021,0xb0f00						\n\ */

static void unhook_dpmi(void)
{
  __dpmi_set_protected_mode_interrupt_vector(0x31, &old_i31);
  __dpmi_set_protected_mode_interrupt_vector(0x21, &old_i21);
}

static void dbgsig(int sig)
{
  if(__djgpp_exception_state->__cs != my_cs || sig == SIGTRAP) {
    *load_state = *__djgpp_exception_state;	/* exception was in other process */
    longjmp(jumper, 1);
  }
}

void run_child(void)
{
  load_state->__cs = a_tss.tss_cs;
  load_state->__ss = a_tss.tss_ss;
  load_state->__ds = a_tss.tss_ds;
  load_state->__es = a_tss.tss_es;
  load_state->__fs = a_tss.tss_fs;
  load_state->__gs = a_tss.tss_gs;
  load_state->__eip = a_tss.tss_eip;
  load_state->__eflags = a_tss.tss_eflags;
  load_state->__eax = a_tss.tss_eax;
  load_state->__ebx = a_tss.tss_ebx;
  load_state->__ecx = a_tss.tss_ecx;
  load_state->__edx = a_tss.tss_edx;
  load_state->__esp = a_tss.tss_esp;
  load_state->__ebp = a_tss.tss_ebp;
  load_state->__esi = a_tss.tss_esi;
  load_state->__edi = a_tss.tss_edi;
  if(!setjmp(jumper)){
    /* jump to tss */
    _set_break_DPMI();
    hook_dpmi();
    longjmp(load_state, load_state->__eax);
    /* we never return here, execption routine will longjump */
  }
  /* exception routine:  save state, copy to tss, return */
  a_tss.tss_cs = load_state->__cs;
  a_tss.tss_ss = load_state->__ss;
  a_tss.tss_ds = load_state->__ds;
  a_tss.tss_es = load_state->__es;
  a_tss.tss_fs = load_state->__fs;
  a_tss.tss_gs = load_state->__gs;
  a_tss.tss_eip = load_state->__eip;
  a_tss.tss_esp = load_state->__esp;
  a_tss.tss_eflags = load_state->__eflags;
  a_tss.tss_eax = load_state->__eax;
  a_tss.tss_ebx = load_state->__ebx;
  a_tss.tss_ecx = load_state->__ecx;
  a_tss.tss_edx = load_state->__edx;
  a_tss.tss_esi = load_state->__esi;
  a_tss.tss_edi = load_state->__edi;
  a_tss.tss_ebp = load_state->__ebp;
  a_tss.tss_irqn = load_state->__signum;
  a_tss.tss_error = load_state->__sigmask;
  unhook_dpmi();
  _clear_break_DPMI();
}

static int invalid_addr(unsigned a, unsigned len)
{
  /* Here we assume expand up writable code.  We could check the rights to
     be sure, but that's a waste unless *_child routines fixed to know about
     different selectors. */

  unsigned limit;
  limit = __dpmi_get_segment_limit(__djgpp_app_DS);
  if(a >= 4096 && (a+len-1) <= limit)
    return 0;
/*  printf("Invalid access to child, address %#x length %#x  limit: %#x\n", a, len, limit);
  if (can_longjmp)
    longjmp(debugger_jmpbuf, 1); */
  return 1;
}

int read_child(unsigned child_addr, void *buf, unsigned len)
{
  if (invalid_addr(child_addr, len))
    return 1;
  movedata(__djgpp_app_DS, child_addr, my_ds, (int)buf, len);
  return 0;
}

int write_child(unsigned child_addr, void *buf, unsigned len)
{
  if (invalid_addr(child_addr, len))
    return 1;
  movedata(my_ds, (int)buf, __djgpp_app_DS, child_addr, len);
  return 0;
}

void edi_init(jmp_buf start_state)
{
  my_ds = 0;
  asm("mov %%ds,%0" : "=g" (my_ds) );
  my_cs = 0;
  asm("mov %%cs,%0" : "=g" (my_cs) );

  *load_state = *start_state;
  a_tss.tss_cs = load_state->__cs;
  a_tss.tss_ss = load_state->__ss;
  a_tss.tss_ds = load_state->__ds;
  a_tss.tss_es = load_state->__es;
  a_tss.tss_fs = load_state->__fs;
  a_tss.tss_gs = load_state->__gs;
  a_tss.tss_eip = load_state->__eip;
  a_tss.tss_esp = load_state->__esp;
  a_tss.tss_eflags = load_state->__eflags;

  __djgpp_app_DS = a_tss.tss_ds;
  app_cs = a_tss.tss_cs;
  edi.app_base = 0;
  signal(SIGTRAP, dbgsig);
  signal(SIGSEGV, dbgsig);
  signal(SIGFPE, dbgsig);
  signal(SIGINT, dbgsig);
}
