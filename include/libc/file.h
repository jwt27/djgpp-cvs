/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_libc_file_h__
#define __dj_include_libc_file_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

#define _IOREAD   000010
#define _IOWRT    000020
#define _IOMYBUF  000040
#define _IOEOF    000100
#define _IOERR    000200
#define _IOSTRG   000400
#define _IORW     001000
#define _IOAPPEND 002000
#define _IORMONCL 004000  /* remove on close, for temp files */
/* if _flag & _IORMONCL, ._name_to_remove needs freeing */

int	_flsbuf(int, FILE*);
int	_filbuf(FILE *);
void	_fwalk(void (*)(FILE *));

#define __getc(p) (--(p)->_cnt>=0 ? \
  (int)(*(unsigned char*)(p)->_ptr++) : \
  _filbuf(p))

#define __putc(x,p) (--(p)->_cnt>=0? \
  ((int)((unsigned char)((*(p)->_ptr++=(unsigned)(x))))): \
  _flsbuf((unsigned)(x),p))

#undef  fileno
#define fileno(f)	(f->_file)
#undef  feof
#define feof(f)		(((f)->_flag&_IOEOF)!=0)
#undef  ferror
#define ferror(f)	(((f)->_flag&_IOERR)!=0)

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* __dj_include_libc_file_h__ */
