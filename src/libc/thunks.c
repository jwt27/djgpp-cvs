#include "dpmi.h"
#include "string.h"
#include "dos.h"
#include "libc/internal.h"
#include "signal.h"
#include "stdio.h"
#include "unistd.h"
#include "io.h"
#include "ctype.h"

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
