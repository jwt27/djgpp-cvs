/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_stdarg_h_
#define __dj_include_stdarg_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#ifdef __dj_include_varargs_h_
#error stdarg.h and varargs.h are mutually exclusive
#endif

#include <sys/djtypes.h>

#ifndef _VA_LIST
__DJ_va_list
#define _VA_LIST
#endif

/* New va_list builtins from GCC 2.96 or later */
#if ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 96)) || (__GNUC__ >= 3)

#define va_arg                  __builtin_va_arg
#define va_end                  __builtin_va_end
#define va_start(ap, last_arg)  __builtin_stdarg_start((ap), (last_arg))

#else /* #if ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 96)) || (__GNUC__ >= 3) */
  
#define __dj_va_rounded_size(T)  \
  (((sizeof (T) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#define va_arg(ap, T) \
    (ap = (va_list) ((char *) (ap) + __dj_va_rounded_size (T)),	\
     *((T *) (void *) ((char *) (ap) - __dj_va_rounded_size (T))))

#define va_end(ap)

#define va_start(ap, last_arg) \
 (ap = ((va_list) __builtin_next_arg (last_arg)))

#endif /* #if ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 96)) || (__GNUC__ >= 3) */
  
#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_stdarg_h_ */
