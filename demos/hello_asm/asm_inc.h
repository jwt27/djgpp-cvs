/* proxy header for djgpp, not needed for dj64 */
#define __ASM(x, y) extern x y
#define __ASM_FUNC(x) void x(void)
#define __ASM_ARR(t, x, l) extern t x[l]
#define SEMIC ;
#include "glob_asm.h"
#undef __ASM
#undef __ASM_FUNC
#undef __ASM_ARR
#undef SEMIC
