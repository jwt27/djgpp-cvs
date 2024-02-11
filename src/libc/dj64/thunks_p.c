#include <string.h>
#include "dosobj.h"
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
#define __ARG_ARR(t) t
#define __ARG_PTR_FAR(t)
#define __ARG_A(t) t
#define __ARG_PTR_A(t) UDWORD
#define __ARG_ARR_A(t) UDWORD
#define __ARG_PTR_FAR_A(t)
#define __RET(t, v) v
#define __RET_PTR(t, v) djaddr2ptr(v)
#define __RET_PTR_FAR(t, v) FP_FROM_D(t, v)
#define __CALL(n, s, l, f) do_asm_call(n, s, l, f)
#define __CSTK(l) clean_stk(l)

static uint32_t obj_init(const void *data, uint16_t len)
{
    uint32_t ret = mk_dosobj(len);
    pr_dosobj(ret, data, len);
    return ret;
}

#define __CNV_PTR(t, d, f, l, t0) t d  = obj_init(f, l)
#define __CNV_CPTR(t, d, f, l, t0) t d  = obj_init(f, l)
#define __CNV_PTR_CCHAR(t, d, f, l, t0) t d = obj_init(f, strlen(f) + 1)
#define __CNV_CHAR_ARR(t, d, f, l, t0) t d = obj_init(f, l)
#define __CNV_PTR_VOID(t, d, f, l, t0) t d = (0)
#define __CNV_PTR_CVOID(t, d, f, l, t0) t d = (0)
#define __CNV_SIMPLE(t, d, f, l, t0) t d = (f)

#define _CNV(c, t, at, l, n) c(at, _a##n, a##n, l, t)
#define _L_REF(nl) a##nl
#define _L_IMM(n, l) (sizeof(*_L_REF(n)) * (l))
#define _L_SZ(n) sizeof(*_L_REF(n))

static void obj_done(void *data, uint32_t fa, uint16_t len)
{
    cp_dosobj(data, fa, len);
    rm_dosobj(fa);
}

#define U__CNV_PTR_FAR(f, d, l) // unused
#define U__CNV_PTR(f, d, l) obj_done(f, d, l)
#define U__CNV_CPTR(f, d, l)
#define U__CNV_PTR_CCHAR(f, d, l)
#define U__CNV_PTR_CHAR(f, d, l)
#define U__CNV_PTR_ARR(f, d, l)
#define U__CNV_CHAR_ARR(f, d, l)
#define U__CNV_PTR_VOID(f, d, l)
#define U__CNV_PTR_CVOID(f, d, l)
#define U__CNV_SIMPLE(f, d, l)

#define _UCNV(c, l, n) U##c(a##n, _a##n, l)

#include "thunk_asms.h"
