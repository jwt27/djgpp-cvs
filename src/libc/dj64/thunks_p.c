#include "thunk_incs.h"
#include "thunks_p.h"

#define PACKED __attribute__((packed))

#define UBYTE uint8_t
#define UDWORD uint32_t
#define DWORD int32_t
#define UWORD uint16_t
#define WORD int16_t
#define VOID void

#define __ARG(t) t
#define __ARG_PTR(t) t *
#define __ARG_PTR_FAR(t)
#define __ARG_A(t) t
#define __ARG_PTR_A(t) t *
#define __ARG_PTR_FAR_A(t)
#define __ARG_CBK(t) _cbk_##t
#define __ARG_CBK_A(t) UDWORD
#define __RET(t, v) v
#define __RET_PTR(t, v) djaddr2ptr(v)
#define __RET_PTR_FAR(t, v) FP_FROM_D(t, v)
#define __CALL(n, s, l, f) do_asm_call(n, s, l, f)
#define __CSTK(l) clean_stk(l)

#define __CNV_PTR_FAR(t, d, f, l, t0) // unused
#define __CNV_PTR(t, d, f, l, t0) t d  = (f)
#define __CNV_PTR_CCHAR(t, d, f, l, t0) t d = (f)
#define __CNV_PTR_CHAR(t, d, f, l, t0) t d = (f)
#define __CNV_PTR_ARR(t, d, f, l, t0) t d = (f)
#define __CNV_PTR_VOID(t, d, f, l, t0) t d = (f)
#define __CNV_CBK(t, d, f, l, t0) UDWORD d = alloc##t0(f)
#define __CNV_SIMPLE(t, d, f, l, t0) t d = (f)

#define _CNV(c, t, at, l, n) c(at, _a##n, a##n, l, t)
#define _L_REF(nl) a##nl
#define _L_IMM(n, l) (sizeof(*_L_REF(n)) * (l))

#include "thunk_asms.h"
