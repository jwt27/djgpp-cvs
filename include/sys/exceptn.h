/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_sys_exceptn_h__
#define __dj_include_sys_exceptn_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

#ifdef __dj_include_setjmp_h_
extern jmp_buf *__djgpp_exception_state_ptr;	/* Must include setjmp.h first */
#define __djgpp_exception_state (*__djgpp_exception_state_ptr)
#endif

extern unsigned short __djgpp_our_DS;
extern unsigned short __djgpp_app_DS;	/* Data selector invalidated by HW ints */
extern unsigned short __djgpp_ds_alias;	/* Data selector always valid */
extern unsigned short __djgpp_dos_sel;	/* Linear mem selector copy in locked mem */
extern unsigned short __djgpp_hwint_flags; /* 1 = Disable Ctrl-C; 2 = Count Ctrl-Break (don't kill) */
extern unsigned __djgpp_cbrk_count;	/* Count of CTRL-BREAK hits */
extern int __djgpp_exception_inprog;	/* Nested exception count */

void __djgpp_exception_toggle(void);
int  __djgpp_set_ctrl_c(int enable);	/* On by default */

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_sys_exceptn_h__ */
