/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * Header for internal stat()/fstat()/lstat() assist functions.
 *
 */

#ifndef __XSTAT_H
#define __XSTAT_H

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dpmi.h>

/* Some errno.h headers do not define EFAULT.  Here the value is
   taken from POSIX IEEE 1003.1.  */
#include <errno.h>
#ifndef EFAULT
#define EFAULT  14
#endif

/* Under MS-DOS, file access permissions are shared by all, except for
   Write permission. */
#define READ_ACCESS     (S_IRUSR | S_IRGRP | S_IROTH)
#define WRITE_ACCESS    S_IWUSR
#define EXEC_ACCESS     (S_IXUSR | S_IXGRP | S_IXOTH)

/* Macro to convert a segment and an offset to a "far offset" suitable
   for _farxxx() functions of DJGPP.  */
#ifndef MK_FOFF
#define MK_FOFF(s,o) ((int)((((unsigned long)(s)) << 4) + (unsigned short)(o)))
#endif

#define MAX_TRUE_NAME   FILENAME_MAX

extern unsigned short   _osmajor, _osminor;
extern const    char  * _os_flavor;

extern time_t           _file_time_stamp(unsigned int);
extern ino_t            _invent_inode(const char *, unsigned, unsigned long);
extern unsigned short   _get_magic(const char *, int);
extern unsigned short   _get_dos_version(int);
extern char           * _truename(const char *, char *);
extern int              _is_remote_drive(int);
extern int              _is_executable(const char *, int, const char *);
extern short            _get_dev_info(int);
extern long             __filelength(int);
extern int              _is_remote_handle(int);
extern void             _djstat_describe_lossage(FILE *);
extern int              _getftime(int, unsigned int *);
extern blksize_t        _get_cached_blksize (const char *path);

#endif  /* __XSTAT_H */
