/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#undef __P
#if defined(__STDC__) || defined(__cplusplus)
#define __P(x) x
#else
#define __P(x)
#endif

#if defined(__cplusplus)
#define __BEGIN_DECLS	extern "C" {
#define __END_DECLS	}
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif
