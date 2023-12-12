/* Copyright (C) 2017 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_string_h_
#define __dj_include_string_h_

#include <stddef.h>

int	strcoll(const char *_s1, const char *_s2);
char *	strerror(int _errcode);
size_t	strxfrm(char *  _s1, const char *  _s2, size_t _n);

//int	strerror_r(int _errnum, char *_strerrbuf, size_t _buflen);

int	memicmp(const void *_s1, const void *_s2, size_t _n);
char *	strdup(const char *_s);
char *	strndup(const char *_s, size_t _n);
size_t	strlcat(char *_dest, const char *_src, size_t _size);
size_t	strlcpy(char *_dest, const char *_src, size_t _size);
char *	strlwr(char *_s);
int	strcasecmp(const char *_s1, const char *_s2);
int	stricmp(const char *_s1, const char *_s2);
int	strncasecmp(const char *_s1, const char *_s2, size_t _n);
int	strnicmp(const char *_s1, const char *_s2, size_t _n);
char *	strupr(char *_s);

#define __USE_GNU
#include_next <string.h>
#undef __USE_GNU
#ifdef __USE_POSIX_IMPLICITLY
#undef __USE_POSIX_IMPLICITLY
#undef _DEFAULT_SOURCE
#undef _POSIX_SOURCE
#endif

#endif /* !__dj_include_string_h_ */
