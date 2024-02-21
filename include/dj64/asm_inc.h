#include <libc/asmobj.h>

#define __ASM(t, v) EXTERN ASM(t, v)
#define __ASM_PTR(t, v) EXTERN ASM_P(t, v)
#define __ASM_ARR(t, v) EXTERN t ASM_ARR(v)
#define __ASM_FUNC(v) EXTERN ASM_F(v)
#define __ASM_N(t, v) EXTERN t ASM_N(v)
#define SEMIC ;
#include "glob_asm.h"
#undef __ASM
#undef __ASM_PTR
#undef __ASM_ARR
#undef __ASM_FUNC
#undef __ASM_N
#undef SEMIC
