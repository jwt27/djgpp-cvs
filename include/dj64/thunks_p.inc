#include <string.h>
#include <dj64/thunks_p.h>

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
#define __CALL(n, s, l, f) dj64_asm_call(n, s, l, f)
#define __CSTK(l) dj64_clean_stk(l)

#define __CNV_PTR(t, d, f, l, t0) t d  = dj64_obj_init(f, l)
#define __CNV_CPTR(t, d, f, l, t0) t d  = dj64_obj_init(f, l)
#define __CNV_PTR_CCHAR(t, d, f, l, t0) t d = dj64_obj_init(f, strlen(f) + 1)
#define __CNV_CHAR_ARR(t, d, f, l, t0) t d = dj64_obj_init(f, l)
#define __CNV_PTR_PVOID(t, d, f, l, t0) t d = PTR_DATA(f)
#define __CNV_PTR_CPVOID(t, d, f, l, t0) t d = PTR_DATA(f)
#define __CNV_PTR_VOID(t, d, f, l, t0) t d = dj64_obj_init(f, l)
#define __CNV_PTR_CVOID(t, d, f, l, t0) t d = dj64_obj_init(f, l)
#define __CNV_SIMPLE(t, d, f, l, t0) t d = (f)

#define _CNV(c, t, at, l, n) c(at, _a##n, a##n, l, t)
#define _L_REF(nl, m) (a##nl * (m))
#define _L_IMM(n, l) (sizeof(a##n[0]) * (l))
#define _L_SZ(n) sizeof(*a##n)

#define U__CNV_PTR(f, d, l) dj64_obj_done(f, d, l)
#define U__CNV_CPTR(f, d, l) dj64_rm_dosobj(d)
#define U__CNV_PTR_CCHAR(f, d, l) dj64_rm_dosobj(d)
#define U__CNV_PTR_CHAR(f, d, l) dj64_obj_done(f, d, strlen(DATA_PTR(d)) + 1)
#define U__CNV_CHAR_ARR(f, d, l) dj64_obj_done(f, d, l)
#define U__CNV_PTR_PVOID(f, d, l)
#define U__CNV_PTR_CPVOID(f, d, l)
#define U__CNV_PTR_VOID(f, d, l) dj64_obj_done(f, d, l)
#define U__CNV_PTR_CVOID(f, d, l) dj64_rm_dosobj(d)
#define U__CNV_SIMPLE(f, d, l)

#define _UCNV(c, l, n) U##c(a##n, _a##n, l)

#include "thunk_asms.h"
