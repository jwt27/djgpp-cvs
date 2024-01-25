#ifndef ASMOBJ_H
#define ASMOBJ_H

#include <libc/djthunks.h>

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

extern int *____djgpp_base_address;
#define _DP(l) djaddr2ptr(*____djgpp_base_address + (l))
#define ASM_P(t, x) unsigned *__##x;/*
*/DEFINE(x, ((t *)_DP(*__##x)))/*
*/

#define ASM_AP(t, x) unsigned *__##x;/*
*/static inline void _##x(int idx) { /*TODO: asm call to __x[idx]*/ }/*
*/DEFINE(x, ((t *)__##x))/*
*/

#define _PD(l) ((char *)(l) - (unsigned long)(__djgpp_mem_base + *____djgpp_base_address))
#define ASM_F(x) *__##x;/*
*/DEFINE(x, (void (*)(void))_PD(__##x))/*
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
#define ASM_F(x) *__##x
#define ASM_N(x) extern *__##x
#define ASM_P(t, x) unsigned *__##x
#define ASM_AP(t, x) unsigned *__##x
#endif

#define EXTERN

#endif

#endif
