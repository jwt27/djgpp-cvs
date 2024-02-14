#define __ASM(t, v) _E __ASMSYM(t) __##v
#define __ASM_ARR(t, v, l) _E __ASMARSYM(t, l) __##v
#define __ASM_FUNC(v) _E __ASMFSYM(void) __##v
#define SEMIC ;
#include <glob_asm.h>
#undef __ASM
#undef __ASM_ARR
#undef __ASM_FUNC
#undef SEMIC
