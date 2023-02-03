/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_sys_exceptn_h__
#define __dj_include_sys_exceptn_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

#include "libc/asmobj.h"
#include <setjmp.h>
#include <dpmi.h>

#ifdef __dj_include_setjmp_h_
EXTERN ASM_P(jmp_buf, __djgpp_exception_state_ptr);	/* Must include setjmp.h first */
#define __djgpp_exception_state (*__djgpp_exception_state_ptr)
#endif

EXTERN unsigned short ASM(__djgpp_our_DS);
EXTERN unsigned short ASM(__djgpp_app_DS);	/* Data selector invalidated by HW ints */
EXTERN unsigned short ASM(__djgpp_ds_alias);	/* Data selector always valid */
EXTERN unsigned short ASM(__djgpp_dos_sel);	/* Linear mem selector copy in locked mem */
EXTERN short ASM(__excep_ds_alias);
EXTERN void ASM_F(__djgpp_cbrk_hdlr);
EXTERN int ASM_F(__djgpp_exception_table);
EXTERN int ASM_F(__djgpp_kbd_hdlr);
EXTERN int ASM_F(__djgpp_kbd_hdlr_pc98);
EXTERN unsigned ASM(_stklen);
EXTERN unsigned ASM(__djgpp_stack_limit);

/* These are all defined in exceptn.S and only used here */
EXTERN __dpmi_paddr ASM(__djgpp_old_kbd);
EXTERN __dpmi_paddr ASM(__djgpp_old_timer);

/* Hardware Interrupt Flags:

   1 = Disable INTR and QUIT keys (Ctrl-C and Ctrl-\);
   2 = Count Ctrl-Break (don't kill);
   4 = IRET from our timer interrupt handler, don't chain */
EXTERN unsigned short ASM(__djgpp_hwint_flags);
EXTERN unsigned ASM(__djgpp_cbrk_count);	/* Count of CTRL-BREAK hits */
extern int __djgpp_exception_inprog;	/* Nested exception count */
EXTERN int ASM(__djgpp_hw_lock_start);
EXTERN int ASM(__djgpp_hw_lock_end);
EXTERN int ASM(__djgpp_iret);
EXTERN int ASM(__djgpp_i24);
EXTERN unsigned ASM(__djgpp_stack_top);
EXTERN int ASM(__djgpp_npx_hdlr);
EXTERN unsigned short ASM(__djgpp_sigint_key);  /* key that raises SIGINT */
EXTERN unsigned short ASM(__djgpp_sigquit_key); /* key that raises SIGQUIT */
EXTERN unsigned short ASM(__djgpp_sigint_mask); /* kb mask for SIGINT key */
EXTERN unsigned short ASM(__djgpp_sigquit_mask);/* kb mask for SIGQUIT key */
EXTERN unsigned ASM(exception_stack);
#define djgpp_exception_stack exception_stack

void __djgpp_exception_toggle(void);
int  __djgpp_set_ctrl_c(int __enable);	/* On by default */
int  __djgpp_set_sigint_key(int new_key);  /* Set key which raises SIGINT */
int  __djgpp_set_sigquit_key(int new_key); /* Set key which raises SIGQUIT */

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_sys_exceptn_h__ */
