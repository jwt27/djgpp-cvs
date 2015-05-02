/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2004 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* exception handling support by Pierre Muller */

#if (GAS_MAJOR == 2) \
    && ((GAS_MINOR < 9) || ((GAS_MINOR == 9) && (GAS_MINORMINOR < 5)))
#define LJMP(there) "ljmp    " #there
#define LCALL(there) "lcall  " #there
#else
#define LJMP(there) "ljmp    *" #there
#define LCALL(there) "lcall   *" #there
#endif

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
#include <stubinfo.h>
#include <libc/farptrgs.h>
#include <sys/fsext.h>
#include <io.h>

#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 3))
# define __attribute_used __attribute__((__used__))
#elif __GNUC__ >= 2
# define __attribute_used __attribute__((__unused__))
#else
# define __attribute_used
#endif

extern char __libdbg_ident_string[];
#if 0
static char *id = __libdbg_ident_string;  /* Pacify compiler.  */
#endif

#define MEM_HANDLE_COUNT	256
#define DESCRIPTOR_COUNT	128
#define DOS_DESCRIPTOR_COUNT	128
#define DPMI_EXCEPTION_COUNT     20
#define DS_SIZE_COUNT           128

#define USE_FSEXT
#define CLOSE_UNREGISTERED_FILES
#define SAVE_FP

/* debug splitted into 3 parts */
/* #define DEBUG_ALL_DBGCOM  */

#ifdef DEBUG_ALL_DBGCOM
/* general debug infos */
#define DEBUG_DBGCOM
/* files open/close infos */
#define DEBUG_DBGCOM_FILES
/* exceptions infos */
#define DEBUG_EXCEPTIONS
#endif /* DEBUG_ALL_DBGCOM */

long mem_handles[MEM_HANDLE_COUNT];
unsigned short descriptors[DESCRIPTOR_COUNT];
unsigned short dos_descriptors[DOS_DESCRIPTOR_COUNT];

/* these all need to be static because
  ss can be different from ds in dbgsig !! */
static int excep_stack[1000];
static int errcode, cs, eflags, eip, ss, esp, ret_cs, ret_eip;
static int *cur_pos;
static int child_exception_level;

ExternalDebuggerInfo edi;
TSS a_tss;

static jmp_buf jumper;

static int my_ds;
static int my_cs;
static int app_cs;
static int __attribute_used app_exit_cs;
static int app_ds;
static unsigned int __attribute_used app_ds_size[DS_SIZE_COUNT];
static int  __attribute_used app_ds_index = 0;
static jmp_buf load_state;

static int nset, breakhandle[4];

static __dpmi_paddr our_handler[DPMI_EXCEPTION_COUNT], app_handler[DPMI_EXCEPTION_COUNT];

#ifdef DEBUG_EXCEPTIONS
typedef
  struct {
   short excp_cs;
   int   excp_eip;
   short excp_nb;
   } texcp_info;

static texcp_info excp_info[20];
static int excp_index;
static int excp_count;
static int redir_excp_count;
#endif

NPX npx;
NPX debugger_npx;

/* ------------------------------------------------------------------------- */
/* Store the contents of the NPX in the global variable `npx'.  */

#define FPU_PRESENT 0x04

void save_npx (void)
{
#ifdef SAVE_FP
  union {
    long double d_value;
    NPXREG r_value;
  } npx_reg_union;  /*  Fix -Wstrict-aliasing.  */
  int i;
  if ((__dpmi_get_coprocessor_status() & FPU_PRESENT) == 0)
    return;
  asm volatile
      ("movb	$0x0b, %%al					\n\
	outb	%%al, $0xa0					\n\
	inb	$0xa0, %%al					\n\
	testb	$0x20, %%al					\n\
	jz	1f						\n\
	xorb	%%al, %%al					\n\
	outb	%%al, $0xf0					\n\
	movb	$0x20, %%al					\n\
	outb	%%al, $0xa0					\n\
	outb	%%al, $0x20					\n\
1:								\n\
	fnsave	%0						\n\
	fwait"
       : "=m" (npx)
       : /* No input */
       : "%eax");
  npx.top = (npx.status & NPX_TOP_MASK) >> NPX_TOP_SHIFT;
  npx.in_mmx_mode = (npx.top == 0);
  for (i = 0; i < 8; i++)
  {
    /* tag is a array of 8 2 bits that contain info about FPU registers
       st(0) is register(top) and st(1) is register (top+1) ... */
    npx.st_valid[i] = ((npx.tag >> (((npx.top + i) & 7) << 1)) & 3) != 3;
    if (npx.st_valid[i])
    {
      npx_reg_union.r_value = npx.reg[i];
      npx.st[i] = npx_reg_union.d_value;
      /* On my Pentium II the two last bytes are set to 0xFF
         on MMX instructions, but on the Intel docs
         it was only specified that the exponent part
         has all bits set !
         Moreover this are only set if the specific mmx register is used */

      if (npx.reg[i].exponent != 0x7FFF)
        if ((npx.tag >> ((((npx.top + i) & 7) << 1)) & 3) == 2)
          npx.in_mmx_mode = 0;
    }
    else
    {
      npx.st[i] = 0;
      npx.in_mmx_mode = 0;
    }
  }

  if (npx.in_mmx_mode)
    for (i = 0; i < 8; i++)
    {
      npx_reg_union.r_value = npx.reg[i];
      npx.mmx[i] = npx_reg_union.d_value;
    }

  /* Restore debugger's FPU state.  */
  asm volatile ("frstor %0" : :"m" (debugger_npx));
#endif
}
/* ------------------------------------------------------------------------- */
/* Reload the contents of the NPX from the global variable `npx'.  */

void load_npx (void)
{
  if ((__dpmi_get_coprocessor_status() & FPU_PRESENT) == 0)
    return;
  /* Save debugger's FPU state.  */
  asm volatile ("fnsave %0" : :"m" (debugger_npx));
#if 0
  /* This code is disabled because npx.mmx[] and npx.st[] are supposed
     to be read-only, they exist to make it easier for a debugger to
     display the FP registers either as long doubles or as 64-bit MMX
     registers.  If the debugger wants to *change* the values, it
     should always change in npx.reg[].  Otherwise, we will need a
     whole slew of flags to know which one of the different views
     should be used to restore child's FPU state, or else the debugger
     will be forced to handle the extra burden of copying the same
     value into each one of the three views of the same registers.  */
  if (npx.in_mmx_mode)
  {
    int i;
    /* change reg to mmx */
    for (i = 0; i < 8; i++)
      if (npx.mmx[i]!= * (long double *) &(npx.reg[i]))
       memcpy(&(npx.reg[i]), &(npx.mmx[i]), 10);
  }
  else
  {
    int i;
    /* change reg to st */
    for (i = 0; i < 8; i++)
       if ((npx.st_valid[i]) && (npx.st[i]!= * (long double *) &(npx.reg[i])))
         memcpy(&(npx.reg[i]), &(npx.st[i]), 10);
  }
#endif
  asm volatile ("frstor %0" : "=m" (npx));
}

static int _DPMIcancelBreak(int handle)
{
  int rv, state;

  rv = __dpmi_get_state_of_debug_watchpoint(handle, &state);
  if(rv == -1) {
    printf("DPMI get watchpoint state failed for handle 0x%x\n",handle);
    state = 0;
  }
  rv = __dpmi_clear_debug_watchpoint(handle);
  if(rv == -1)
    printf("DPMI release watchpoint failed for handle 0x%x\n", handle);

  /* printf("CancelBreak han=0x%x returns state=0x%x\n", handle, state); */
  return (state & 1);
}

/* Can't be static because called in asm below; -O3 inlines if static */
void _set_break_DPMI(void);
void _set_break_DPMI(void)
{
  int i, rv;
  unsigned extract;
  unsigned char brtype;
  unsigned long vbase;
  __dpmi_meminfo bpinfo;
  
  if(__dpmi_get_segment_base_address(app_ds, &vbase) == -1)
    return;
  extract = edi.dr[7] >> 16;
  nset = 0;
  edi.app_base = vbase;

  for (i = 0; i < 4; i++)
    if ((edi.dr[7] >> (i * 2)) & 3)
    {							/* enabled? */
      brtype = (extract >> (i * 4)) & 3;		/* extract the type */
      if(brtype == 3) brtype = 2;			/* convert for DPMI brain damage */
      bpinfo.size = ((extract >> (i * 4 + 2)) & 3) + 1;	/* size */
      bpinfo.address = edi.dr[i] + vbase;
      rv = __dpmi_set_debug_watchpoint(&bpinfo, brtype);
      if (rv != -1)
      {
        breakhandle[i] = bpinfo.handle;
        /* printf("SetBreak typ=%d siz=%d at 0x%x returns han=%d\n", brtype, (int)bpinfo.size, (unsigned)bpinfo.address, breakhandle[i]); */
        if (breakhandle[i] == (bpinfo.address >> 16))	/* Win 2K bug */
          breakhandle[i] = nset;
        nset++;
      }
      else
      {
        printf("Error allocating DPMI breakpoint type %d of size %d at address 0x%08lx\n", brtype, (int)bpinfo.size, edi.dr[i]);
        breakhandle[i] = -1;
      }
    }
    else
      breakhandle[i] = -1;
  return;
}

/* Can't be static because called in asm below; -O3 inlines if static */
void _clear_break_DPMI(void);
void _clear_break_DPMI(void)
{
  int i, bt;

  if (!nset)
  {
    edi.dr[6] = 0;
    return;
  }

  bt = 0;
  for (i = 3; i > -1; i--)
  {
    bt = bt << 1;                              /* Shift for next bit */
    if (breakhandle[i] != -1)
      bt |= _DPMIcancelBreak(breakhandle[i]);  /* Set low bit if active */
  }

  edi.dr[6] = bt;
}

static __dpmi_paddr old_i31, old_i21, user_i31, user_i21;
static int user_int_set = 0;
static __dpmi_paddr my_i9, user_i9, my_i8, user_i8;
static void (*oldNOFP)(int);
static void dbgsig(int);

static void hook_dpmi(void)
{
  int i;
  __dpmi_paddr new_int;
  extern void i21_hook(void), i31_hook(void), __dbgcom_kbd_hdlr(void);

  __dpmi_get_protected_mode_interrupt_vector(0x21, &old_i21);
  __dpmi_get_protected_mode_interrupt_vector(0x31, &old_i31);
  /* Save our current interrupt vectors for the keyboard and the timer */
  __dpmi_get_protected_mode_interrupt_vector(0x09, &my_i9);
  __dpmi_get_protected_mode_interrupt_vector(0x08, &my_i8);

  for (i = 0; i < DPMI_EXCEPTION_COUNT; i++)
    __dpmi_get_processor_exception_handler_vector(i,&our_handler[i]);

  asm volatile("mov %%cs, %0" : "=g" (new_int.selector));
  new_int.offset32 = (unsigned long)i21_hook;
  __dpmi_set_protected_mode_interrupt_vector(0x21, &new_int);
  new_int.offset32 = (unsigned long)i31_hook;
  __dpmi_set_protected_mode_interrupt_vector(0x31, &new_int);
  /* avoid to set the ds limit to 0x0FFF twice */
  new_int.offset32 = (unsigned long)__dbgcom_kbd_hdlr;
  __dpmi_set_protected_mode_interrupt_vector(0x09, &new_int);

  /* If we have called already unhook_dpmi, the user interrupt
     vectors for the keyboard and the timer are valid. */
  if (user_int_set)
  {
    if ((user_i9.offset32 != new_int.offset32) || (user_i9.selector != new_int.selector))
      __dpmi_set_protected_mode_interrupt_vector(0x09, &user_i9);
    __dpmi_set_protected_mode_interrupt_vector(0x08, &user_i8);
    __dpmi_set_protected_mode_interrupt_vector(0x21, &user_i21);
    __dpmi_set_protected_mode_interrupt_vector(0x31, &user_i31);
  }
  /*    DONT DO THIS (PM)
    for (i = 0; i < DPMI_EXCEPTION_COUNT; i++)
    {
      if (app_handler[i].offset32 && app_handler[i].selector)
        __dpmi_set_processor_exception_handler_vector(i, &app_handler[i]);
    } */
  load_npx();
  oldNOFP = signal(SIGNOFP, dbgsig); /* if we run under FP emulation */
  /* Crtl-C renders app code unreadable */
  __djgpp_app_DS = app_ds;
}

/* The instructions in __dj_forced_test[] MUST MATCH the expansion of:

   EXCEPTION_ENTRY($13)
   EXCEPTION_ENTRY($14)
   EXCEPTION_ENTRY($15)
   EXCEPTION_ENTRY($16)
   EXCEPTION_ENTRY($17)

   and the first 4 lines that follow the "exception_handler:" label
   near the beginning of src/libc/go32/exceptn.S.  Don't change
   them unless you know what you are doing!

   You ask why don't we start from exception 17 (0x11) instead of
   13 (0x0D), and save some of the bytes in the buffer below?  Well,
   the code that compares the bytes below with the child's code runs
   from change_exception_handler, which is only called if the DPMI
   function 0x203 (Set Processor Exception Handler) succeeds.  However,
   exception 17 is Alignment Check, and any DPMI environment would be
   crazy to support it, 'cause a typical PC code will trigger it all
   the time.  So most DPMI servers don't support it, and our code will
   never be called if we tie it to exception 17.  In contrast, exception
   13 is GPF, and *any* DPMI server will support that!  */
unsigned char __dj_forced_test[] = {
  0x6a,0x0d,			/* pushl $0x0d */
  0xeb,0x10,			/* jmp relative +0x10 */
  0x6a,0x0e,			/* pushl $0x0e */
  0xeb,0x0c,			/* jmp relative +0x0c */
  0x6a,0x0f,			/* pushl $0x0f */
  0xeb,0x08,			/* jmp relative +0x08 */
  0x6a,0x10,			/* pushl $0x10 */
  0xeb,0x04,			/* jmp relative +0x04 */
  0x6a,0x11,			/* pushl $0x11 */
  0xeb,0x0,			/* jmp relative 0 */
  0x53,				/* pushl %ebx     */
  0x1e,				/* push %ds       */
  0x2e,0x80,0x3d		/* (beginning of) %cs:cmpb $0,forced */
}; /* four next bytes contain the address of the `forced' variable */

static int __attribute_used forced_test_size = sizeof (__dj_forced_test);

static int forced_address_known = 0;
static unsigned int forced_address = 0;

/* Set an exception handler */
/* stores it into app_handler if selector is app_cs  */

asm("\n\
        .text                                                           \n\
        .balign  16,,7                                                  \n\
_change_exception_handler:                                              \n\
        pushl %eax							\n\
        push  %es							\n\
        push  %ds							\n\
        .byte 0x2e							\n\
        movw  _my_ds,%ax						\n\
        movw  %ax,%ds							\n\
        movw  %ax,%es							\n\
        movzbl %bl,%eax                                                 \n\
        imull $8,%eax							\n\
        addl  $_app_handler,%eax  /* only retain handlers */            \n\
        cmpw  _app_cs,%cx /* for the main app     */			\n\
        jne   _not_in_current_app					\n\
	cmpb  $20,%bl							\n\
	jae   _transmit_unchanged_values				\n\
        movl  %ecx,4(%eax)                                              \n\
        movl  %edx,(%eax)                                               \n\
        cmpb  $0x0d,%bl	                                                \n\
        jne   _no_forced_check                                          \n\
        cmpl  $0,_forced_address_known                                  \n\
        jne   _no_forced_check                                          \n\
        pushl %esi                                                      \n\
        pushl %edi                                                      \n\
        pushl %ecx                                                      \n\
        pushfl                                                          \n\
        cld                                                             \n\
        movw  %cx,%es                                                   \n\
        movl  %edx,%edi                                                 \n\
        movl  $___dj_forced_test,%esi                                   \n\
        movl  _forced_test_size,%ecx                                    \n\
        repe                                                            \n\
        cmpsb                                                           \n\
        jne   _forced_not_found                                         \n\
        movl  $1,_forced_address_known                                  \n\
        movl  %es:(%edi),%edi                                           \n\
        movl  %edi,_forced_address                                      \n\
_forced_not_found:                                                      \n\
        popfl                                                           \n\
        popl  %ecx                                                      \n\
        popl  %edi                                                      \n\
        popl  %esi                                                      \n\
_no_forced_check:                                                       \n\
_not_in_current_app:                                                    \n\
        subl  $_app_handler,%eax /* allways restore our handler */      \n\
        addl  $_our_handler,%eax                                        \n\
        movl  4(%eax),%ecx                                              \n\
        movl (%eax),%edx                                                \n\
_transmit_unchanged_values:						\n\
        pop   %ds                                                       \n\
        pop   %es                                                       \n\
        popl  %eax                                                      \n\
        ret                                                             \n"
);

/* Get an exception handler */
/* Problem : we must react like the dpmi server */
/* for example win95 refuses call 0x210 so we should also refuse it */
/* otherwise we can get into troubles */
asm("\n\
        .text                                                           \n\
        .balign  16,,7                                                  \n\
_get_exception_handler:                                                 \n\
        pushl   %eax                                                    \n\
	pushf								\n\
	.byte	0x2e							\n\
	" LCALL(_old_i31) "						\n\
        popl   %eax                                                     \n\
	jc	Lc31_set_flags_and_iret					\n\
        pushl   %eax                                                    \n\
        push    %es                                                     \n\
        push    %ds                                                     \n\
        .byte   0x2e                                                    \n\
        movw    _my_ds,%ax                                              \n\
        movw    %ax,%ds                                                 \n\
        movw    %ax,%es                                                 \n\
        movzbl %bl,%eax                                                 \n\
        imull  $8,%eax                                                  \n\
        addl  $_app_handler,%eax   /* only retain handlers */           \n\
        cmpw  $0,4(%eax)                                                \n\
        je    _app_exception_not_set /* for the main app */             \n\
        cmpl  $0,(%eax)                                                 \n\
        je    _app_exception_not_set                                    \n\
        movl  (%eax),%edx                                               \n\
        movw  4(%eax),%cx                                               \n\
        pop    %ds                                                      \n\
        pop    %es                                                      \n\
        popl   %eax                                                     \n\
	clc								\n\
        jmp   Lc31_set_flags_and_iret				        \n\
_app_exception_not_set:                                                 \n\
        pop   %ds                                                       \n\
        pop   %es                                                       \n\
        popl  %eax                                                      \n\
        .byte 0x2e                                                      \n\
        " LJMP(_old_i31) "                                              \n\
        ret                                                             \n"
); 

/* Change a handle in the list: EAX is the old handle, EDX is the new */
/* for changing a value, we need our ds, because cs has no write access */
asm(								       "\n\
	.text								\n\
	.balign  16,,7							\n\
_change_handle:								\n\
	pushl	%ecx							\n\
	xorl	%ecx,%ecx						\n\
CL1:									\n\
	.byte	0x2e 							\n\
	cmpl	%eax,_mem_handles(,%ecx,4)				\n\
	jne	CL2							\n\
	push	%ds							\n\
	.byte	0x2e							\n\
	movw	_my_ds,%ax						\n\
	movw	%ax,%ds							\n\
	movl	%edx,_mem_handles(,%ecx,4)				\n\
	pop	%ds							\n\
	popl	%ecx							\n\
	ret								\n\
CL2:									\n\
	incl	%ecx							\n\
	cmpl	$256,%ecx	/* MEM_HANDLE_COUNT */			\n\
	jl	CL1							\n\
	popl	%ecx							\n\
	ret								\n"
);

/* Change a descriptor in the list: AX is the old, DX is the new */
/* for changing a value, we need our ds, because cs has no write access */
asm(						       			"\n\
	.text								\n\
	.balign  16,,7							\n\
_change_descriptor:							\n\
	pushl	%ecx							\n\
	pushl	%eax							\n\
	xorl	%ecx,%ecx						\n\
CL3:									\n\
	.byte	0x2e 							\n\
	cmpw	%ax,_descriptors(,%ecx,2)				\n\
	jne	CL4							\n\
	push	%ds							\n\
	.byte	0x2e							\n\
	movw	_my_ds,%ax						\n\
	movw	%ax,%ds							\n\
	movw	%dx,_descriptors(,%ecx,2)				\n\
	pop	%ds							\n\
	popl	%eax							\n\
	popl	%ecx							\n\
	ret								\n\
CL4:									\n\
	incl	%ecx							\n\
	cmpl	$128,%ecx	/* DESCRIPTOR_COUNT */			\n\
	jl	CL3							\n\
	popl	%eax							\n\
	popl	%ecx							\n\
	ret								\n"
);

/* Add descriptors to the list: AX is the first, CX is the count */
asm(									"\n\
	.text								\n\
	.balign  16,,7							\n\
_add_descriptors:							\n\
	pushl	%edx							\n\
	pushl	%ecx							\n\
	pushl	%ebx							\n\
	pushl	%eax							\n\
	movw	$0x0003,%ax						\n\
	pushf								\n\
	.byte	0x2e							\n\
	" LCALL(_old_i31) "						\n\
	movw	%ax,%bx							\n\
	popl	%eax							\n\
	pushl	%eax							\n\
	movw	%ax,%dx							\n\
	xorw	%ax,%ax							\n\
CL5:									\n\
	call	_change_descriptor					\n\
	addw	%bx,%dx							\n\
	loop	CL5							\n\
	popl	%eax							\n\
	popl	%ebx							\n\
	popl	%ecx							\n\
	popl	%edx							\n\
	ret								\n"
);

/* Change a dos_descriptor in the list: AX is the old, DX is the new */
/* for changing a value, we need our ds, because cs has no write access */
asm(						       			"\n\
	.text								\n\
	.balign  16,,7							\n\
_change_dos_descriptor:							\n\
	pushl	%eax							\n\
	pushl	%ecx							\n\
	xorl	%ecx,%ecx						\n\
CL6:									\n\
	.byte	0x2e 							\n\
	cmpw	%ax,_dos_descriptors(,%ecx,2)				\n\
	jne	CL7							\n\
	push	%ds							\n\
	.byte	0x2e							\n\
	movw	_my_ds,%ax						\n\
	movw	%ax,%ds							\n\
	movw	%dx,_dos_descriptors(,%ecx,2)				\n\
	pop	%ds							\n\
	popl	%ecx							\n\
	popl	%eax							\n\
	ret								\n\
CL7:									\n\
	incl	%ecx							\n\
	cmpl	$128,%ecx	/* DOS_DESCRIPTOR_COUNT */		\n\
	jl	CL6							\n\
	popl	%ecx							\n\
	popl	%eax							\n\
	ret								\n"
);

/* BUGS: We ignore the exception handlers for the child process, so signals
   do not work.  We also disable the hooking of the numeric coprocessor
   HW interrupt. */

/* Watch set selector base, if it is __djgpp_app_DS then reset breakpoints */

/* Watch the following DPMI-functions: (added by RH)  (some by PM)

  0x0000   : __dpmi_allocate_ldt_descriptors
  0x0001   : __dpmi_free_ldt_descriptor
  0x0007   : __dpmi_set_selector_base_address
             hooked because the hardware breakpoints need to be reset
             if we change the base address of __djgpp_app_DS
  0x0008   : __dpmi_set_selector_limit
             hooked for djgpp_hw_exception tracing
  0x000A   : __dpmi_create_alias_descriptor
  
  0x0100   : __dpmi_allocate_dos_memory
  0x0101   : __dpmi_free_dos_memory
  
  0x0201   : __dpmi_set_real_mode_interrupt
	     (There are problems with Ctrl-Break in debugger. Therefore 
             let's not allow to redefine it)

  0x0202   : __dpmi_get_processor_exception_handler_vector
  0x0203   : __dpmi_set_processor_exception_handler_vector
  0x0205   : __dpmi_set_protected_mode_interrupt
             (int 0x09 and 0x75 rejected)
             Ctrl-C will thus create a SIGINT in the top level
             program but cont will directly return to
             the code before interruption no matter at which level)
             (passing to next would be possible but then we would get a
              interruption in all levels !!)
  0x0210   : __dpmi_get_extended_exception_handler_vector_pm
  0x0212   : __dpmi_set_extended_exception_handler_vector_pm
  
  0x0501   : __dpmi_allocate_memory
  0x0501   : __dpmi_free_memory
  0x0503   : __dpmi_resize_memory

*/ 
asm(									"\n\
	.text								\n\
	.balign  16,,7							\n\
	.globl  _dbgcom_hook_i31					\n\
_dbgcom_hook_i31:							\n\
_i31_hook:								\n\
	cmpw	$0x0000,%ax						\n\
	je	Lc31_alloc_descriptors					\n\
	cmpw	$0x0001,%ax						\n\
	je	Lc31_free_descriptor					\n\
	cmpw	$0x0007,%ax						\n\
	je	Lc31_set_selector_base_address				\n\
	cmpw	$0x0008,%ax						\n\
	je	Lc31_set_selector_limit                                 \n\
        cmpw	$0x000A,%ax						\n\
	je	Lc31_create_alias_descriptor				\n\
	cmpw	$0x0100,%ax						\n\
	je	Lc31_allocate_dos_memory				\n\
	cmpw	$0x0101,%ax						\n\
	je	Lc31_free_dos_memory					\n\
	cmpw	$0x0201,%ax					        \n\
	je	Lc31_set_real_mode_interrupt				\n\
        cmpw    $0x0202,%ax                                             \n\
        je      _get_exception_handler                                  \n\
        cmpw    $0x0203,%ax                                             \n\
        je      Lc31_set_exception_handler                              \n\
	cmpw	$0x0205,%ax						\n\
	je	Lc31_set_protected_mode_interrupt			\n\
        cmpw    $0x0210,%ax                                             \n\
        je      _get_exception_handler                                  \n\
        cmpw    $0x0212,%ax                                             \n\
        je      Lc31_set_exception_handler                              \n\
	cmpw	$0x0501,%ax						\n\
	je	Lc31_alloc_mem						\n\
	cmpw	$0x0502,%ax						\n\
	je	Lc31_free_mem						\n\
	cmpw	$0x0503,%ax						\n\
	je	Lc31_resize_mem						\n\
L_jmp_to_old_i31:                                                       \n\
        .byte	0x2e							\n\
	" LJMP(_old_i31) "						\n\
Lc31_set_flags_and_iret:                                                \n\
        pushl	%eax				                        \n\
	pushf								\n\
	popl	%eax		/* store the right flags for iret */	\n\
	movl	%eax,12(%esp)						\n\
	popl	%eax							\n\
Lc31_iret:                                                              \n\
        iret								\n\
Lc31_set_selector_limit:                                                \n\
	.byte	0x2e							\n\
	cmpw	_app_ds,%bx					        \n\
	jne	L_jmp_to_old_i31					\n\
        pushl   %ds                                                     \n\
        pushl   %eax                                                    \n\
        .byte   0x2e                                                    \n\
        movw    _my_ds,%ax                                              \n\
        movw    %ax,%ds                                                 \n\
        movl    _app_ds_index,%eax                                      \n\
        movw    %cx,_app_ds_size+2(,%eax,4)                             \n\
        movw    %dx,_app_ds_size(,%eax,4)                               \n\
        cmpl    $127,_app_ds_index                                      \n\
        jne     Lc31_index_ok                                           \n\
        pushl   %ebx                                                    \n\
        movl    _app_ds_size-4(,%eax,4),%ebx                            \n\
        movl    %ebx,_app_ds_size                                       \n\
        movl    _app_ds_size(,%eax,4),%ebx                              \n\
        movl    %ebx,_app_ds_size+4                                     \n\
        movl    $1,_app_ds_index                                        \n\
        popl    %ebx                                                    \n\
Lc31_index_ok:                                                          \n\
        incl    _app_ds_index                                           \n\
        popl    %eax                                                    \n\
        popl    %ds                                                     \n\
        jmp     L_jmp_to_old_i31                                        \n\
Lc31_set_selector_base_address:                                         \n\
	.byte	0x2e							\n\
	cmpw	_app_ds,%bx					        \n\
	jne	L_jmp_to_old_i31					\n\
	pushf								\n\
	.byte	0x2e							\n\
	" LCALL(_old_i31) "						\n\
	call	___djgpp_save_interrupt_regs				\n\
	call	__clear_break_DPMI					\n\
	call	__set_break_DPMI					\n\
	movl	___djgpp_exception_state_ptr,%eax			\n\
	pushl	(%eax)							\n\
	pushl	%eax							\n\
	call	_longjmp						\n\
Lc31_set_real_mode_interrupt:						\n\
        cmpb   $0x1B, %bl  						\n\
        je     Lc31_iret   						\n\
        jmp    L_jmp_to_old_i31                                         \n\
Lc31_set_protected_mode_interrupt:                                      \n\
	cmpb	$0x75,%bl						\n\
	je	Lc31_iret						\n\
	/*cmpb	$0x09,%bl*/                                            	\n\
	/*je	Lc31_iret*/						\n\
	jmp	L_jmp_to_old_i31					\n\
Lc31_alloc_mem:								\n\
	pushf								\n\
	.byte	0x2e							\n\
	" LCALL(_old_i31) "						\n\
	jc	Lc31_set_flags_and_iret					\n\
	pushf								\n\
	pushl	%edx							\n\
	pushw	%si							\n\
	pushw	%di							\n\
	popl	%edx							\n\
	xorl	%eax,%eax						\n\
	call	_change_handle						\n\
	popl	%edx							\n\
	popf								\n\
	clc								\n\
	jmp	Lc31_set_flags_and_iret					\n\
Lc31_free_mem:								\n\
	pushw	%si							\n\
	pushw	%di							\n\
	pushf								\n\
	.byte	0x2e							\n\
	" LCALL(_old_i31) "						\n\
	jc	Lc31_resize_mem_error					\n\
	popl	%eax							\n\
	push	%edx							\n\
	xorl	%edx,%edx						\n\
	call	_change_handle						\n\
	pop	%edx							\n\
	xorl	%eax,%eax						\n\
	jmp	Lc31_set_flags_and_iret					\n\
Lc31_resize_mem:							\n\
	pushw	%si							\n\
	pushw	%di							\n\
	pushf								\n\
	.byte	0x2e							\n\
	" LCALL(_old_i31) "						\n\
	jnc	Lc31_resize_mem_ok					\n\
Lc31_resize_mem_error:							\n\
	addl	$4,%esp							\n\
	stc								\n\
	jmp	Lc31_set_flags_and_iret					\n\
Lc31_resize_mem_ok:							\n\
	popl	%eax							\n\
	pushw	%si							\n\
	pushw	%di							\n\
	popl	%edx							\n\
	call	_change_handle						\n\
	xorl	%eax,%eax						\n\
	jmp	Lc31_set_flags_and_iret					\n\
Lc31_alloc_descriptors:							\n\
	pushl	%ecx							\n\
	pushf								\n\
	.byte	0x2e							\n\
	" LCALL(_old_i31) "						\n\
	popl	%ecx							\n\
	jc	Lc31_set_flags_and_iret					\n\
	call	_add_descriptors					\n\
	clc								\n\
	jmp	Lc31_set_flags_and_iret					\n\
Lc31_free_descriptor:							\n\
	pushl	%ebx							\n\
	pushf								\n\
	.byte	0x2e							\n\
	" LCALL(_old_i31) "						\n\
	popl	%eax							\n\
	jc	Lc31_set_flags_and_iret					\n\
	push	%edx							\n\
	xorw	%dx,%dx							\n\
	call	_change_descriptor					\n\
	pop	%edx							\n\
	clc								\n\
	jmp	Lc31_set_flags_and_iret					\n\
Lc31_create_alias_descriptor:						\n\
	pushf								\n\
	.byte	0x2e							\n\
	" LCALL(_old_i31) "						\n\
	jc	Lc31_set_flags_and_iret					\n\
	pushl	%eax							\n\
	push	%edx							\n\
	movw	%ax,%dx							\n\
	xorw	%ax,%ax							\n\
	call	_change_descriptor					\n\
	pop	%edx							\n\
	popl	%eax							\n\
	clc								\n\
	jmp	Lc31_set_flags_and_iret					\n\
Lc31_allocate_dos_memory:						\n\
	pushf								\n\
	.byte	0x2e							\n\
	" LCALL(_old_i31) "						\n\
	jc	Lc31_set_flags_and_iret					\n\
	pushl	%eax							\n\
	xorl	%eax,%eax						\n\
	call	_change_dos_descriptor					\n\
	popl	%eax							\n\
	clc								\n\
	jmp	Lc31_set_flags_and_iret					\n\
Lc31_free_dos_memory:							\n\
	pushl	%edx							\n\
	pushf								\n\
	.byte	0x2e							\n\
	" LCALL(_old_i31) "						\n\
	popl	%eax							\n\
	jc	Lc31_set_flags_and_iret					\n\
	xorw	%dx,%dx							\n\
	call	_change_dos_descriptor					\n\
	clc								\n\
	jmp	Lc31_set_flags_and_iret					\n\
Lc31_set_exception_handler:                                             \n\
        pushl  %eax                                                     \n\
        pushl  %ebx                                                     \n\
        pushl  %ecx                                                     \n\
        pushl  %edx                                                     \n\
        pushf                                                           \n\
        .byte  0x2e                                                     \n\
        " LCALL(_old_i31) "                                             \n\
        popl   %edx                                                     \n\
        popl   %ecx                                                     \n\
        popl   %ebx                                                     \n\
        popl   %eax                                                     \n\
        jc   Lc31_set_flags_and_iret                                    \n\
        call   _change_exception_handler                                \n\
        pushf                                                           \n\
        .byte  0x2e                                                     \n\
        " LCALL(_old_i31) "                                             \n\
        jmp Lc31_set_flags_and_iret                                     \n\
	.balign  16,,7							\n\
        .globl  _dbgcom_hook_i21                                        \n\
_dbgcom_hook_i21:                                                       \n\
_i21_hook:								\n\
	cmpb	$0x4c,%ah						\n\
	je	Lc21							\n\
Lc21_jmp_to_old:                                                        \n\
        .byte	0x2e							\n\
	" LJMP(_old_i21) "						\n\
Lc21:	push	%eax							\n\
	movl	8(%esp),%eax						\n\
	cs								\n\
	cmpw	_app_exit_cs,%ax					\n\
	je	Lc21_exit                                               \n\
	cs								\n\
	cmpw	_app_cs,%ax					        \n\
	je	Lc21_exit                                               \n\
	pop	%eax							\n\
        jmp     Lc21_jmp_to_old                                         \n\
Lc21_exit:                                                              \n\
	pop	%eax							\n\
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

/* complete code to return from an exception */
asm ( ".text								\n\
       .balign 16,,7							\n\
       .globl    _dbgcom_exception_return_to_debuggee			\n\
_dbgcom_exception_return_to_debuggee:       /* remove errorcode from stack */\n\
       /* we must also switch stack back !! */				\n\
       /* relative to ebp */						\n\
       /* 0 previous ebp */						\n\
       /* 4 exception number */						\n\
       /* 8 return eip */						\n\
       /* 12 return cs */						\n\
       /* 16 return eflags */						\n\
       /* 20 return esp  */						\n\
       /* 24 return ss  */						\n\
       /* -4 stored ds */						\n\
       /* -8 stored eax */						\n\
       /* -12 stored esi */						\n\
       pushl  %ebp							\n\
       movl   %esp,%ebp							\n\
       pushl  %ds							\n\
       pushl  %eax							\n\
       pushl  %esi							\n\
       movl   %cs:___djgpp_our_DS,%eax					\n\
       movw   %ax,%ds							\n\
       addl   $32,_cur_pos						\n\
       decl    _child_exception_level					\n\
       movl   24(%ebp),%eax						\n\
       movw   %ax,%ds							\n\
       movl   20(%ebp),%esi						\n\
       /* ds:esi points now to app stack */				\n\
       subl  $28,%esi							\n\
       movl  %esi,20(%ebp)						\n\
       /* eflags on app stack */					\n\
       movl  16(%ebp),%eax						\n\
       movl  %eax,%ds:24(%esi)						\n\
       /* cs on app stack */						\n\
       movl  12(%ebp),%eax						\n\
       movl  %eax,%ds:20(%esi)						\n\
       /* eip on app stack */						\n\
       movl  8(%ebp),%eax						\n\
       movl  %eax,%ds:16(%esi)						\n\
       /* esi on app stack */						\n\
       movl  -12(%ebp),%eax						\n\
       movl  %eax,%ds:12(%esi)						\n\
       /* eax on app stack */						\n\
       movl  -8(%ebp),%eax						\n\
       movl  %eax,%ds:8(%esi)						\n\
       /* ds on app_stack */						\n\
       movl  -4(%ebp),%eax						\n\
       movl  %eax,%ds:4(%esi)						\n\
       /* ebp on app_stack */						\n\
       movl  (%ebp),%eax						\n\
       movl  %eax,%ds:(%esi)						\n\
       /* switch stack */						\n\
       movl  24(%ebp),%eax						\n\
       movw  %ax,%ss							\n\
       movl  %esi,%esp							\n\
       /* now on app stack */						\n\
       popl  %ebp							\n\
       popl  %eax							\n\
       movw  %ax,%ds							\n\
       popl  %eax							\n\
       popl  %esi							\n\
       iret								\n\
    ");

static jmp_buf here;

/* simple code to return from an exception */
/* don't forget to increment cur_pos       */
asm ( ".text								\n\
       .balign 16,,7							\n\
       .globl    _dbgcom_exception_return_to_here			\n\
_dbgcom_exception_return_to_here:       /* remove errorcode from stack */\n\
        movl    %cs:___djgpp_our_DS,%eax                                \n\
        movw    %ax,%ds                                                 \n\
        movw    %ax,%es                                                 \n\
        addl    $32,_cur_pos                                            \n\
        decl    _child_exception_level                                  \n\
	pushl	$1							\n\
	pushl	$_here						        \n\
	call	_longjmp						\n\
        ");
        
/*	movw	%cs:__go32_info_block+26, %fs				\n\
	.byte	0x64							\n\
	movw	$0x7021,0xb0f00						\n\ */

/* do not set limit of ds selector two times */
asm (".text								\n\
        .global ___dbgcom_kbd_hdlr					\n\
___dbgcom_kbd_hdlr:							\n\
        " LJMP(%cs:___djgpp_old_kbd) "");



static void unhook_dpmi(void)
{
  int i;
  /* Crtl-C renders debugger code unreadable */
  __djgpp_app_DS = __djgpp_our_DS;
  signal(SIGNOFP, oldNOFP);	/* in case we run under FP emulation */
  save_npx();
  /* save app i31 and i21 if changed */
  __dpmi_get_protected_mode_interrupt_vector(0x31, &user_i31);
  __dpmi_get_protected_mode_interrupt_vector(0x21, &user_i21);
  /* restore i31 and i21 */
  __dpmi_set_protected_mode_interrupt_vector(0x31, &old_i31);
  __dpmi_set_protected_mode_interrupt_vector(0x21, &old_i21);

  /* Save the interrupt vectors for the keyboard and the the
     time, because the debuggee may have changed it. */
  __dpmi_get_protected_mode_interrupt_vector(0x09, &user_i9);
  __dpmi_get_protected_mode_interrupt_vector(0x08, &user_i8);

  /* And remember it for hook_dpmi */
  user_int_set = 1;

  /* Now restore our interrupt vectors */
  __dpmi_set_protected_mode_interrupt_vector(0x09, &my_i9);
  __dpmi_set_protected_mode_interrupt_vector(0x08, &my_i8);
  for (i = 0; i < DPMI_EXCEPTION_COUNT; i++)
  {
    if (i != 2)
      __dpmi_set_processor_exception_handler_vector(i, &our_handler[i]);
  }

  asm volatile("sti");   /* This improve stability under Win9X after SIGINT */
                         /* Why? (AP) */
}

#define RETURN_TO_HERE 0
#define RETURN_TO_DEBUGGEE 1

static void call_app_exception(int signum, char return_to_debuggee)
{
  extern void dbgcom_exception_return_to_here(void);
  extern void dbgcom_exception_return_to_debuggee(void);
#ifdef DEBUG_EXCEPTIONS
  redir_excp_count++;
#endif
  eip = load_state->__eip;
  cs  = load_state->__cs;
  esp = load_state->__esp;
  ss  = load_state->__ss;
  eflags = load_state->__eflags;
  /* reset the debug trace bit */
  /* we don't want to step inside the exception_table code */
  load_state->__eflags &= 0xFFFFFEFFU;
  errcode = load_state->__sigmask;
  load_state->__eip=app_handler[signum].offset32;
  load_state->__cs=app_handler[signum].selector;
  /* use our own exception stack */
  child_exception_level++;
  cur_pos -= 8;
  if (cur_pos < &excep_stack[0])
  {
   /* We have a problem here, but this should never happen.  */
   fprintf (stderr,
            "Level of nesting in debugger exceptions too high: %d\n",
            child_exception_level);
   exit(-1);
  }
  load_state->__ss = my_ds;
  load_state->__esp= (int) cur_pos;
  /* where to return */
  ret_cs = my_cs;
  if (return_to_debuggee)
    ret_eip = (int) &dbgcom_exception_return_to_debuggee;
  else
    ret_eip = (int) &dbgcom_exception_return_to_here;
  cur_pos[0] = ret_eip;
  cur_pos[1] = ret_cs;
  cur_pos[2] = errcode;
  cur_pos[3] = eip;
  cur_pos[4] = cs;
  cur_pos[5] = eflags;
  cur_pos[6] = esp;
  cur_pos[7] = ss;
  longjmp(load_state, load_state->__eax);
}

static void dbgsig(int sig)
{
  unsigned int ds_size;
  int signum =  __djgpp_exception_state->__signum;
  asm volatile
      ("movl _app_ds,%%eax					\n\
        lsl  %%eax,%%eax					\n\
        movl %%eax,%0"
        : "=g" (ds_size) );

  /* correct ds limit here */
  if ((ds_size==0x0FFF) && (signum==0x0C || signum==0x0D))
  {
    /* If forced_address is known then
       signum contains the fake exception value (PM) */
    if (forced_address_known)
      movedata(app_cs, forced_address, my_ds, (int) &signum, 4);
    else
      signum = 0x1B;  /* else we default to SIGINT */

    if (app_ds_index > 1)
      __dpmi_set_segment_limit(app_ds, app_ds_size[app_ds_index - 2]);  /* set the limit correctly */
    /* let app restore the ds selector */
    if (!setjmp(here))
    {
      *load_state = *__djgpp_exception_state;     /* exception was in other process */
      load_state->__eip = here->__eip;
      load_state->__esp = here->__esp;
      load_state->__cs = here->__cs;
      load_state->__ss = here->__ss;
      /* do use ds exception */
      load_state->__signum = 0x0C;
      /* longjmp returns eax value */
      load_state->__eax = 1;
      call_app_exception(__djgpp_exception_state->__signum, RETURN_TO_HERE);
    }
    __djgpp_exception_state->__signum=signum;
  }

#ifdef DEBUG_EXCEPTIONS
  excp_info[excp_index].excp_eip = __djgpp_exception_state->__eip;
  excp_info[excp_index].excp_cs = __djgpp_exception_state->__cs;
  excp_info[excp_index].excp_nb = signum;
  excp_index++;
  excp_count++;
  if (excp_index == 20)
    excp_index = 0;
#endif
  if (__djgpp_exception_state->__cs == app_cs)
     /* || sig == SIGTRAP) */
  {
    *load_state = *__djgpp_exception_state;	/* exception was in other process */
    longjmp(jumper, 1);
  }
  else
  {
    extern int invalid_sel_addr(short sel, unsigned a, unsigned len, char for_write);

    if ((signum<DPMI_EXCEPTION_COUNT) &&
        (app_handler[signum].offset32) &&
        (app_handler[signum].selector) &&
        !invalid_sel_addr(app_handler[signum].selector,
        app_handler[signum].offset32,1,0) &&
        ((app_handler[signum].offset32 !=
        our_handler[signum].offset32) ||
        (app_handler[signum].selector !=
        our_handler[signum].selector)))
    {
      *load_state = *__djgpp_exception_state;
      /* This exception was in other process, so the debuggee should
         handle it.  */
      call_app_exception(signum, RETURN_TO_DEBUGGEE);
    }
#if 0
    else
    {
      *load_state = *__djgpp_exception_state;
      longjmp(jumper, 1);
    }
#endif
  }
}

void run_child(void)
{
  /* we should call the exception handlers if an exception is on */
  /* Question : how distinguish exception zero ? */
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
  if (!setjmp(jumper))
  {
    extern int invalid_sel_addr(short sel, unsigned a, unsigned len, char for_write);
    /* jump to tss */
    _set_break_DPMI();
    hook_dpmi();
    if (a_tss.tss_trap == 0xFFFF)
    {
      /* We were asked by the debugger to deliver exception to
         the child when it is resumed.  */
      if (a_tss.tss_irqn >= DPMI_EXCEPTION_COUNT && forced_address_known)
      {
        unsigned app_ds_size = __dpmi_get_segment_limit (app_ds);
        if (app_ds_size > 0x0FFF)
        {
          /* This is a fake exception (SIGINT, SIGALRM, etc.).
             We need to poke the `forced' variable in the child
             with the fake exception number.  */
          _farpokel (app_ds, forced_address, a_tss.tss_irqn);

          /* We also need to save the child's DS limit in the
             child's ds_limit variable, because the child's fake
             exception handling code will try to restore the DS
             limit from the value of ds_limit.  ds_limit is
             defined in exceptn.S at offset -4 relative to the
             forced variable (PM).  */
          _farpokel (app_ds, forced_address - 4, app_ds_size);
        }
        a_tss.tss_irqn = 0x0d; /* simulate a GPF in the child */
      }
      if ((a_tss.tss_irqn < DPMI_EXCEPTION_COUNT)
          && (app_handler[a_tss.tss_irqn].offset32)
          && (app_handler[a_tss.tss_irqn].selector)
          && !invalid_sel_addr(app_handler[a_tss.tss_irqn].selector, app_handler[a_tss.tss_irqn].offset32, 1, 0))
      {
        call_app_exception(a_tss.tss_irqn, RETURN_TO_DEBUGGEE);
      }
      else
      {
        a_tss.tss_irqn = 0;
        longjmp(load_state, load_state->__eax);
      }
    }
    else
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
  a_tss.tss_trap = 0;
  unhook_dpmi();
  _clear_break_DPMI();
}

static int invalid_addr(unsigned a, unsigned len)
{
  /* Here we assume expand up writable code.  We could check the rights to
     be sure, but that's a waste unless *_child routines fixed to know about
     different selectors. */

  unsigned limit;
  limit = __dpmi_get_segment_limit(app_ds);
  if (4096 <= a                 /* First page is used for NULL pointer detection. */
      && a <= limit             /* To guard against limit < len. */
      && a - 1 <= limit - len)  /* To guard against limit <= a + len - 1. */
    return 0;

#if 0
  printf("Invalid access to child, address %#x length %#x  limit: %#x\n", a, len, limit);
  if (can_longjmp)
    longjmp(debugger_jmpbuf, 1);
#endif

  return 1;
}

int read_child(unsigned child_addr, void *buf, unsigned len)
{
  if (invalid_addr(child_addr, len))
    return 1;
  movedata(app_ds, child_addr, my_ds, (int)buf, len);
  return 0;
}

int write_child(unsigned child_addr, void *buf, unsigned len)
{
  if (invalid_addr(child_addr, len))
    return 1;
  movedata(my_ds, (int)buf, app_ds, child_addr, len);
  return 0;
}

int invalid_sel_addr(short sel, unsigned a, unsigned len, char for_write)
{
  /* Here we assume expand up writable code.  We could check the rights to
     be sure, but that's a waste unless *_child routines fixed to know about
     different selectors. */

  unsigned limit;
  char read_allowed = 0;
  char write_allowed = 0;
  
  asm volatile
    ("										\n\
      movw  %2,%%ax								\n\
      verr  %%ax								\n\
      jnz   .Ldoes_not_has_read_right						\n\
      movb  $1,%0								\n\
.Ldoes_not_has_read_right:							\n\
      verw  %%ax								\n\
      jnz   .Ldoes_not_has_write_right						\n\
      movb  $1,%1								\n\
.Ldoes_not_has_write_right: "
     : "=qm" (read_allowed), "=qm" (write_allowed)
     : "g" (sel)
     );

  if (for_write)
  {
    if (!write_allowed)
      return 1;
  }
  else
    if (!read_allowed)
      return 1;

  limit = __dpmi_get_segment_limit(sel);
  /* some selectors don't have zero page protection
     like the protected interrupt stack */
  if (/*a >= 4096 && */ (a + len - 1) <= limit)
    return 0;
#if 0
  printf("Invalid access to child, address %#x length %#x  limit: %#x\n", a, len, limit);
  if (can_longjmp)
    longjmp(debugger_jmpbuf, 1);
#endif

  return 1;
}

int read_sel_addr(unsigned child_addr, void *buf, unsigned len, unsigned sel)
{
  /* first clear memory */
  memset(buf, 0, len);
  if (invalid_sel_addr(sel, child_addr, len, 0))
    return 1;
  movedata(sel, child_addr, my_ds, (int)buf, len);
  return 0;
}

int write_sel_addr(unsigned sel, unsigned child_addr, void *buf, unsigned len)
{
  if (invalid_sel_addr(sel, child_addr, len, 1))
    return 1;
  movedata(my_ds, (int)buf, sel, child_addr, len);
  return 0;
}

static _GO32_StubInfo si;

static void (*oldTRAP)(int);
static void (*oldSEGV)(int);
static void (*oldFPE)(int);
static void (*oldINT)(int);
static void (*oldQUIT)(int);
static void (*oldILL)(int);

int cmd_selector;	/* set by v2loadimage */

void edi_init(jmp_buf start_state)
{
  int i;
  my_ds = 0;
  asm volatile ("mov %%ds,%0" : "=g" (my_ds));
  my_cs = 0;
  asm volatile ("mov %%cs,%0" : "=g" (my_cs));

  for (i = 0; i < DPMI_EXCEPTION_COUNT; i++)
  {
    app_handler[i].offset32 = 0;
    app_handler[i].selector = 0;
  }
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
  a_tss.tss_trap = 0;

  app_ds = a_tss.tss_ds;
  app_cs = a_tss.tss_cs;
  if (__dpmi_get_segment_base_address(app_ds, &edi.app_base) == -1)
    abort ();
  /* Save debugger's FPU state.  */
  asm volatile ("fnsave %0" : :"m" (debugger_npx));
  /* Fill the debuggee's FPU state with the default values, taken from
     the equivalent of FNINIT performed by FNSAVE above.  */
  memset(&npx, 0, sizeof(npx));
  save_npx();
  /* Save all the changed signal handlers */
  oldTRAP = signal(SIGTRAP, dbgsig);
  oldSEGV = signal(SIGSEGV, dbgsig);
  oldFPE = signal(SIGFPE, dbgsig);
  oldINT = signal(SIGINT, dbgsig);
  oldQUIT = signal(SIGQUIT, dbgsig);
  oldILL = signal(SIGILL, dbgsig);
  movedata(a_tss.tss_fs, 0, my_ds, (unsigned)&si, sizeof(si));
  memset(mem_handles, 0, sizeof(mem_handles));
  mem_handles[0] = si.memory_handle;
  memset(descriptors, 0, sizeof(descriptors));
  descriptors[0] = si.cs_selector;
  descriptors[1] = si.ds_selector;
  descriptors[2] = app_ds;
  descriptors[3] = app_cs; 
  app_exit_cs=si.cs_selector;
  memset(dos_descriptors, 0, sizeof(dos_descriptors));
  dos_descriptors[0] = _farpeekw(si.psp_selector, 0x2C);
  dos_descriptors[1] = si.psp_selector;
  if (cmd_selector)
    dos_descriptors[2] = cmd_selector;
  /* set initial value of cur_pos */
  cur_pos = &excep_stack[1000 - 40];
  /* pattern fill exception stack for debugging */
  memset(&excep_stack, 0xAB, sizeof(excep_stack));
  child_exception_level = 0;
}

static void close_handles(void); /* Forward declaration */

void cleanup_client(void)
{
  int i;

  /* restore __djgpp_app_DS for Ctrl-C !! */
  __djgpp_app_DS = __djgpp_our_DS;
#ifdef DEBUG_EXCEPTIONS
  fprintf(stderr,"excp_count = %d\n", excp_count);
  fprintf(stderr,"redir_excp_count = %d\n", redir_excp_count);
  fprintf(stderr,"excp_index = %d\n", excp_index);
  fprintf(stderr,"app_cs %04x\tapp_ds %04x\n", app_cs, app_ds);
  fprintf(stderr,"my_cs %04x\tmy_ds %04x\n", my_cs, my_ds);
  for (i = 0; i < excp_count; i++)
  {
    fprintf(stderr, " excep %04x:%08x\tsignal %08x\n",
                    excp_info[i].excp_cs,
                    excp_info[i].excp_eip,
                    excp_info[i].excp_nb);
    excp_info[i].excp_eip = 0;
    excp_info[i].excp_cs = 0;
    excp_info[i].excp_nb = 0;
  }
  for (i = 0; i < DS_SIZE_COUNT; i++)
  {
    if (app_ds_size[i])
    {
      fprintf(stderr, " ds size %08x\n", app_ds_size[i]);
      app_ds_size[i] = 0;
    }
  }
  excp_count = 0;
  redir_excp_count = 0;
  excp_index = 0;
  for (i = 0; i < DPMI_EXCEPTION_COUNT; i++)
  {
    fprintf(stderr, " app %d handler = %04x:%08lx\n",
                    i, app_handler[i].selector, app_handler[i].offset32);
  }
  for (i = 0; i < DESCRIPTOR_COUNT; i++)
  {
    if (descriptors[i])
      fprintf(stderr,"used descriptor: %08x\n", descriptors[i]);
  }
#endif
  for (i = 0; i < DPMI_EXCEPTION_COUNT; i++)
  {
    app_handler[i].offset32 = 0;
    app_handler[i].selector = 0;
  }
  /* Invalidate the info about the `forced' variable.  */
  forced_address_known = 0;
  forced_address = 0;
  forced_test_size = sizeof (__dj_forced_test); /* pacify the compiler */
  /* Set the flag, that the user interrupt vectors are no longer valid */
  user_int_set = 0;

  memset(&npx, 0, sizeof(npx));
  /* Close all handles, which may be left open */
  close_handles();
  for (i = 0; i < DOS_DESCRIPTOR_COUNT; i++)
  {
    if (dos_descriptors[i])
    {
#ifdef DEBUG_DBGCOM
      fprintf(stderr,"free dos memory: %08x\n", dos_descriptors[i]);
#endif
      __dpmi_free_dos_memory(dos_descriptors[i]);
    }
  }
  for (i = 0; i < MEM_HANDLE_COUNT; i++)
  {
    if (mem_handles[i])
    {
#ifdef DEBUG_DBGCOM
      fprintf(stderr,"free mem : %08lx\n", mem_handles[i]);
#endif
      __dpmi_free_memory(mem_handles[i]);
    }
  }
  for (i = 0; i < DESCRIPTOR_COUNT; i++)
  {
    if (descriptors[i])
    {
#ifdef DEBUG_DBGCOM
      fprintf(stderr,"free descriptor: %08x\n", descriptors[i]);
#endif
      __dpmi_free_ldt_descriptor(descriptors[i]);
    }
  }
  /* Restore all changed signal handlers */
  signal(SIGTRAP, oldTRAP);
  signal(SIGSEGV, oldSEGV);
  signal(SIGFPE, oldFPE);
  signal(SIGINT, oldINT);
  signal(SIGQUIT, oldQUIT);
  signal(SIGILL, oldILL);
}

#ifndef USE_FSEXT

static void close_handles(void)
{
}

#else /* USE_FSEXT */
/*
   Now the FSEXT function for watching files being opened. This is needed,
   because the debuggee can open files which are not closed and if you
   do this multiple times, the limit of max opened files is reached.

   The watching is done by the FSEXT function by hooking the _open(),
   _creat() and _close() calls from the libc functions. The only things
   which are added is recording files which are opened (and closed) by
   the debugger. When cleanup_client() is called, this list is compared
   with actual open files and every file, which was not seen by dbg_fsext()
   is closed.

   This technique does not work correctly when the debugger uses the lowest
   routines for opening/creating/closing files which are
   _dos_open(), _dos_creat(), _dos_creatnew() and _dos_close().
*/

static unsigned char handles[256];
static int in_dbg_fsext = 0;

static void close_handles(void)
{
  __dpmi_regs r;
  int psp_la;
  int jft_ofs;
  int jft_count;
  int handle;

  /* Get our PSP address.  */
  r.x.ax = 0x6200;
  __dpmi_int (0x21, &r);
  psp_la = ((int)r.x.bx) << 4;

  /* Get the offset of the JFT table by (seg << 4) + offset */
  jft_ofs = (_farpeekw(_dos_ds, psp_la + 0x36) << 4) +
            _farpeekw(_dos_ds, psp_la + 0x34);

  /* Number of used entries in the JFT table */
  jft_count = _farpeekw(_dos_ds, psp_la + 0x32);

  /* Disable the fsext function */
  in_dbg_fsext++;

  for (handle = 0; handle < jft_count; handle++)
  {
    if (_farpeekb(_dos_ds, jft_ofs++) != 0xFF  /* it is an opened handle */
        && handles[handle] == 0xFF)            /* but not recorded by the fsext function */
    { /* it was opened by the debuggee */
#ifdef CLOSE_UNREGISTERED_FILES
#ifdef DEBUG_DBGCOM_FILES
      fprintf(stderr, "closing %d\n", handle);
#endif
      _close(handle);
#else  /* not CLOSE_UNREGISTERED_FILES */
#ifdef DEBUG_DBGCOM_FILES
      fprintf(stderr, "unknown open file %d\n", handle);
#endif
#endif /* CLOSE_UNREGISTERED_FILES */
    }
  }

  /* Enable the fsext function */
  in_dbg_fsext--;
}


static int dbg_fsext(__FSEXT_Fnumber _function_number, int *_rv, va_list _args)
{
  int attrib, oflag, retval = 0, handle;
  const char *filename;
  /* We are called from this function */
  if (in_dbg_fsext) return 0;
  switch (_function_number)
  {
    default:
      return 0;
    case __FSEXT_creat:
      filename = va_arg(_args, const char *);
      attrib = va_arg(_args, int);
      in_dbg_fsext++;
      retval = _creat(filename, attrib);
#ifdef DEBUG_DBGCOM_FILES
      fprintf(stderr, "_creat(%s) => %d\n", filename,retval);
#endif
      in_dbg_fsext--;
      if (retval != -1)
      {
        handles[retval] = retval;
        __FSEXT_set_function(retval, dbg_fsext);
      }
      break;
    case __FSEXT_open:
      filename = va_arg(_args, const char *);
      oflag = va_arg(_args, int);
      in_dbg_fsext++;
      retval = _open(filename, oflag);
#ifdef DEBUG_DBGCOM_FILES
      fprintf(stderr, "_open(%s) => %d\n", filename, retval);
#endif
      in_dbg_fsext--;
      if (retval != -1)
      {
        handles[retval] = retval;
        __FSEXT_set_function(retval, dbg_fsext);
      }
      break;
    case __FSEXT_close:
      handle = va_arg(_args, int);
      in_dbg_fsext++;
#ifdef DEBUG_DBGCOM_FILES
      fprintf(stderr, "_close(%d)\n", handle);
#endif
      retval = _close(handle);
      in_dbg_fsext--;
      if (retval == 0)
      {
        handles[handle] = 0xFF;
        __FSEXT_set_function(handle, NULL);
      }
      break;
  }
  *_rv = retval;
  return 1;
}

/* With attribute constructor to be called automaticaly before main */

static void __attribute__((__constructor__))
_init_dbg_fsext(void)
{
  __dpmi_regs r;
  int psp_la;
  int jft_ofs;
  int jft_count;

  /* Get our PSP address.  */
  r.x.ax = 0x6200;
  __dpmi_int (0x21, &r);
  psp_la = ( (int)r.x.bx ) << 4;

  /* Get the offset of the JFT table by (seg << 4) + offset */
  jft_ofs = (_farpeekw(_dos_ds, psp_la + 0x36) << 4) +
            _farpeekw(_dos_ds, psp_la + 0x34);

  /* Number of used entries in the JFT table */
  jft_count = _farpeekw(_dos_ds, psp_la + 0x32);

  /* Add the handler for opening/creating files */
  __FSEXT_add_open_handler(dbg_fsext);

  /* Initialize all the handles to 0xFF */
  memset(handles, 0xFF, sizeof(handles));

  /* Get a copy of all already opened handles */
  movedata(_dos_ds, jft_ofs, _my_ds(), (int)handles, jft_count);

  /* enable the fsext function */
  in_dbg_fsext = 0;
}
#endif /* def USE_FSEXT */
