/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */

/* Written by Laurynas Biveinis                                */
/* This file contains some internal info related to symlinks   */
/* Note: symlink file format is still in internal include file */
/* because I don't think it's required for user apps           */
#ifndef __dj_include_libc_symlink_h_
#define __dj_include_libc_symlink_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

#include <sys/djtypes.h>

__DJ_size_t
#undef __DJ_size_t
#define __DJ_size_t

/* Semi-internal library function which reads symlink contents given  */
/* either a file name or its handle.  Used by readlink(), fstat() and */
/* user supplied (if any) file fstat handler.                         */

int __internal_readlink(const char * __path, int __fhandle, char * __buf, 
                        size_t  __max);

/* A prototype for internal library function for fully resolving symlink   */
/* chain. Standard function readlink() solves only one symlink level.      */
/* If path name passed appears to be not a symlink, it is copied to result */
/* string. Return code 1 means success, 0 - failure (to many links - errno */
/* is set too).                                                            */

int __solve_symlinks(const char * __symlink_path, char * __real_path);

/* Internal library function for fully resolving symlinks in `__symlink_path' */
/* except the last its component. Otherwise it is similar to __solve_symlinks */
int __solve_dir_symlinks(const char * __symlink_path, char * __real_path);

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_libc__h_ */
