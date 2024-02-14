#include <dj64/thunks_a.h>

#ifndef NO_GLOB_TMPL
#define _E
#include "glob_tmpl.h"
#undef _E
#endif

#define __S(x) #x
#define _S(x) __S(x)
#define __ASM(t, v) { _S(_##v), (void **)&__##v, 0 }
#define __ASM_FUNC(v) __ASM(t, v)
#define SEMIC ,
struct athunk asm_thunks[] = {
    #include "glob_asm.h"
};

const int num_athunks = _countof(asm_thunks);

#ifndef NO_ASMC_THUNKS
struct athunk asm_cthunks[] = {
#define ASMCSYM(s, n) [n] = { _S(_##s), NULL, 0 },
#include "plt_asmc.h"
};

const int num_cthunks = _countof(asm_cthunks);

uint32_t asm_tab[_countof(asm_cthunks)];
#endif
