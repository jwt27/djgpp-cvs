#include <stdint.h>

struct athunk {
    const char *name;
    void **ptr;
    unsigned flags;
};

#ifdef _IN_DJ64
#define _U(x) x
#else
#define _U(x) x##_user
#endif

extern struct athunk _U(asm_thunks)[];
#define _countof(array) (sizeof(array) / sizeof(array[0]))
extern const int _U(num_athunks);
extern struct athunk _U(asm_cthunks)[];
extern const int _U(num_cthunks);
extern uint32_t _U(asm_tab)[];
