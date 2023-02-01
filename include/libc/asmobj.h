#ifndef ASMOBJ_H
#define ASMOBJ_H

#define _h #
#define _f(x) x
#define DEFINE(x, y) _f(_h)define x y

#ifndef IN_ASMOBJ

#define ASM(x) *__##x;/*
*/DEFINE(x, (*__##x))/*
*/

#define ASM_N(x) *__##x;/*
*/DEFINE(x, (*__##x))/*
*/

extern char *__djgpp_mem_base;
extern int *____djgpp_base_address;
#define _DP(l) (__djgpp_mem_base + *____djgpp_base_address + (l))
#define ASM_P(t, x) unsigned *__##x;/*
*/DEFINE(x, ((t *)_DP(*__##x)))/*
*/

#define EXTERN extern

#else

#if IN_ASMOBJ == 1
#define ASM_N(x) _##x
#endif

#if IN_ASMOBJ == 2
#define ASM_N(x) *__##x
#endif

#if IN_ASMOBJ == 3
#define ASM(x) *__##x
#define ASM_N(x) extern *__##x
#define ASM_P(t, x) unsigned *__##x
#endif

#define EXTERN

#endif

#endif
