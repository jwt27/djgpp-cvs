#define IN_ASMOBJ 3
#include "asm_incs.h"
#include "thunks_a.h"

#define __S(x) #x
#define _S(x) __S(x)
#define __ASM(t, v) { _S(_##v), (void **)&__##v, 0 }
#define SEMIC ,
struct athunk asm_thunks[] = {
    #include "asym_incs.h"
};

const int num_athunks = _countof(asm_thunks);

struct athunk asm_cthunks[] = {
#define ASMCSYM(s, n) [n] = { _S(_##s), NULL, 0 },
#include "plt_asmc.h"
};

const int num_cthunks = _countof(asm_cthunks);

uint32_t asm_tab[_countof(asm_cthunks)];
