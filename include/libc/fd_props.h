/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_libc_fdprops_h__
#define __dj_include_libc_fdprops_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

/* Delete file when the last descriptor referencing it is closed.  */
#define FILE_DESC_TEMPORARY 0x01

typedef struct fd_properties fd_properties;

struct fd_properties
{
  unsigned char ref_count;
  char *filename;
  unsigned long flags;
  fd_properties *prev;
  fd_properties *next;
};

extern fd_properties ** __fd_properties;

int __set_fd_properties(int _fd, const char * _file, int _oflags);
void __dup_fd_properties(int _from, int _to);
int __clear_fd_properties(int _fd);

static __inline__ int __has_fd_properties(int _fd)
{
  return __fd_properties && __fd_properties[_fd];
}

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifdef __cplusplus
}
#endif

#endif /* __dj_include_libc_fdprops_h__  */
