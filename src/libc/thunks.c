#include "thunk_incs.h"

#define PACKED __attribute__((packed))
enum DispStat { DISP_OK, DISP_NORET };
enum { ASM_CALL_OK, ASM_CALL_ABORT };
enum { FDPP_RET_ABORT = -1, FDPP_RET_OK, FDPP_RET_NORET };
enum { ASM_OK, ASM_NORET, ASM_ABORT, PING_ABORT };
#define ___assert(x)
static int recur_cnt;
static __dpmi_regs s_regs;
#define UBYTE uint8_t
#define UDWORD uint32_t
#define DWORD int32_t
#define UWORD uint16_t
#define WORD int16_t
#define VOID void

#define _ARG(n, t, ap) (*(t *)(ap + n))
#define _ARG_PTR(n, t, ap) ((t *)(ap + n))
#define _ARG_R(t) t
#define _ARG_RPTR(t) t *
#define _RET(r) r
#define _RET_PTR(r) PTR_DATA(r)

static UDWORD FdppThunkCall(int fn, UBYTE *sp, enum DispStat *r_stat,
        int *r_len)
{
    UDWORD ret;
    UBYTE rsz = 0;
    enum DispStat stat;

#define _SP sp
#define _DISP_CMN(f, c) { \
    c; \
}
#define _DISPATCH(r, rv, rc, f, ...) _DISP_CMN(f, { \
    rv _r; \
    stat = DISP_OK; \
    _r = f(__VA_ARGS__); \
    ret = rc(_r); \
    if (stat == DISP_OK) \
        rsz = (r); \
    else \
        ___assert(ret != ASM_NORET); \
})
#define _DISPATCH_v(f, ...) _DISP_CMN(f, { \
    stat = DISP_OK; \
    ret = ASM_OK; \
    f(__VA_ARGS__); \
})

    switch (fn) {
        #include "thunk_calls.h"

        default:
//            fdprintf("unknown fn %i\n", fn);
//            _fail();
            return 0;
    }

    *r_stat = stat;
    *r_len = rsz;
    return ret;
}

static int _FdppCall(__dpmi_regs *regs)
{
    int len;
    UDWORD res;
    enum DispStat stat;

//    assert(fdpp);

    s_regs = *regs;
    res = FdppThunkCall(regs->d.ecx, (UBYTE *)DATA_PTR(regs->d.edx), &stat, &len);
    *regs = s_regs;
    if (stat == DISP_NORET)
        return (res == ASM_NORET ? FDPP_RET_NORET : FDPP_RET_ABORT);
    switch (len) {
    case 0:
        break;
    case 1:
        regs->h.al = res;
        break;
    case 2:
        regs->x.ax = res;
        break;
    case 4:
        regs->d.eax = res;
        break;
    default:
//        _fail();
        break;
    }
    return FDPP_RET_OK;
}

int FdppCall(__dpmi_regs *regs); // XXX
int FdppCall(__dpmi_regs *regs)
{
    int ret;
    recur_cnt++;
    ret = _FdppCall(regs);
    recur_cnt--;
    return ret;
}


#define _TFLG_NONE 0
#define _TFLG_FAR 1
#define _TFLG_NORET 2
#define _TFLG_INIT 4

static uint32_t do_asm_call(int num, uint8_t *sp, uint8_t len, int flags)
{
    return 0;
}

static uint8_t *clean_stk(size_t len)
{
//    uint8_t *ret = (uint8_t *)so2lin(s_regs.ss, LO_WORD(s_regs.esp));
//    s_regs.esp += len;
//    return ret;
    return NULL;
}

typedef void (*_cbk_VOID)(void);

static UDWORD alloc_cbk_VOID(_cbk_VOID cbk)
{
    return 0; // TODO
}

#define __ARG(t) t
#define __ARG_PTR(t) t *
#define __ARG_PTR_FAR(t)
#define __ARG_A(t) t
#define __ARG_PTR_A(t) t *
#define __ARG_PTR_FAR_A(t)
#define __ARG_CBK(t) _cbk_##t
#define __ARG_CBK_A(t) UDWORD
#define __RET(t, v) v
#define __RET_PTR(t, v) DATA_PTR(v)
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
