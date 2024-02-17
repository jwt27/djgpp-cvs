#include <libc/asmobj.h>
#define __ASM(t, v) EXTERN t ASM(v)
#define __ASM_ARR(t, v, l) EXTERN t ASM_ARR(v)
#define __ASM_FUNC(v) EXTERN ASM_F(v)
#define SEMIC ;
#include <glob_asm.h>
#undef __ASM
#undef __ASM_ARR
#undef __ASM_FUNC
#undef SEMIC
