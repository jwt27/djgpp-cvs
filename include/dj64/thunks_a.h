#include <stdint.h>

struct athunk {
    const char *name;
    void **ptr;
    unsigned flags;
};

extern struct athunk asm_thunks[];
#define _countof(array) (sizeof(array) / sizeof(array[0]))
extern const int num_athunks;
extern struct athunk asm_cthunks[];
extern const int num_cthunks;
extern uint32_t asm_tab[];
