/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_stddef_h_
#define __dj_include_stddef_h_

#ifdef __cplusplus
namespace std {
  extern "C" {
#endif

#include <sys/djtypes.h>
  
#define NULL 0
#ifdef __cplusplus
#define offsetof(s_type, mbr) ((std::size_t) &((s_type *)0)->mbr)
#else
#define offsetof(s_type, mbr) ((size_t) &((s_type *)0)->mbr)
#endif

#ifndef _PTRDIFF_T
typedef int ptrdiff_t;
#define _PTRDIFF_T
#endif

#ifndef _SIZE_T
__DJ_size_t
#define _SIZE_T
#endif

#ifndef _WCHAR_T
__DJ_wchar_t
#define _WCHAR_T
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
  }
}
#endif

#endif /* !__dj_include_stddef_h_ */


#if defined(__cplusplus)
#ifndef __dj_via_cplusplus_header_

using std::ptrdiff_t;
using std::size_t;

#endif /* !__dj_via_cplusplus_header_ */
#endif /* __cplusplus */
