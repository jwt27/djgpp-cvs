/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#undef assert

#if defined(NDEBUG)
#define assert(test) (void)0
#else
#define assert(test) ((test)?0:__dj_assert(#test,__FILE__,__LINE__))
#endif

#ifndef __dj_include_assert_h_
#define __dj_include_assert_h_

#ifdef __cplusplus
extern "C" {
#endif

void	__dj_assert(const char *,const char *,int);

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_assert_h_ */

#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */
