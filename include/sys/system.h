/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_sys_system_h__
#define __dj_include_sys_system_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

#define __system_redirect	      0x0001 /* redirect internally */
#define __system_call_cmdproc	      0x0002 /* always call COMMAND/$SHELL */
#define __system_use_shell	      0x0004 /* use $SHELL if set */
#define __system_allow_multiple_cmds  0x0008 /* allow `cmd1; cmd2; ...' */
#define __system_allow_long_cmds      0x0010 /* handle commands > 126 chars  */
#define __system_handle_null_commands 0x1000 /* ignore cmds with no effect */
#define __system_ignore_chdir	      0x2000 /* make `cd' be a null command */
#define __system_emulate_chdir	      0x4000 /* handle `cd' internally */

extern int __system_flags;

extern int _shell_command (const char *_prog, const char *_cmdline);

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* __dj_include_sys_system_h__ */
