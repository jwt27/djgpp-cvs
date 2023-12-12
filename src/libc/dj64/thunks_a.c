#define IN_ASMOBJ 3
#include "asm_incs.h"
__attribute__((unused)) // TODO!
static void **asm_thunks[] = {
    #include "asym_incs.h"
};
