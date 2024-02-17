#include <dj64/thunks_a.h>

#define __S(x) #x
#define _S(x) __S(x)
#define __ASM(t, v) { _S(_##v), (void **)&__##v, 0 }
#define __ASM_FUNC(v) __ASM(t, v)
#define SEMIC ,
struct athunk _U(asm_thunks)[] = {
    #include "glob_asm.h"
};

const int _U(num_athunks) = _countof(_U(asm_thunks));

#ifndef NO_ASMC_THUNKS
struct athunk _U(asm_cthunks)[] = {
#define ASMCSYM(s, n) [n] = { _S(_##s), NULL, 0 },
#include "plt_asmc.h"
};

const int _U(num_cthunks) = _countof(_U(asm_cthunks));

uint32_t _U(asm_tab)[_countof(_U(asm_cthunks))];
#endif
