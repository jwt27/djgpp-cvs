#ifndef ASMOBJ_H
#define ASMOBJ_H

#define _h #
#define _f(x) x
#define DEFINE(x, y) _f(_h)define x y

#ifndef IN_ASMOBJ

#define ASM(x) *__##x;/*
*/DEFINE(x, (*__##x))/*
*/

#define EXTERN extern

#else

#define ASM(x) _##x

#define EXTERN

#endif

#endif
