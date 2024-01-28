#define IN_ASMOBJ 3
#include "asm_incs.h"
#include "thunks_a.h"

#define __S(x) #x
#define _S(x) __S(x)
#define __ASM(t, v) { _S(v), (void **)&__##v, 0 }
#define SEMIC ,
struct athunk asm_thunks[] = {
    #include "asym_incs.h"
};

const int num_athunks = _countof(asm_thunks);
