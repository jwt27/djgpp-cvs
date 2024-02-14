#include <stdint.h>

#define UBYTE uint8_t
#define UDWORD uint32_t
#define DWORD int32_t
#define UWORD uint16_t
#define WORD int16_t
#define VOID void

enum { ASM_OK, ASM_NORET, ASM_ABORT, PING_ABORT };

UDWORD dj64_thunk_call(int fn, UBYTE *sp, enum DispStat *r_stat, int *r_len);
