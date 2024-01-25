#include <assert.h>
#include <djdev64/dj64init.h>
#include <libc/djthunks.h>
#include "plt.h"
#include "thunk_incs.h"

#define PACKED __attribute__((packed))
enum { ASM_CALL_OK, ASM_CALL_ABORT };
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

static int _dj64_call(int libid, int fn, __dpmi_regs *regs, uint8_t *sp,
    dj64dispatch_t *disp)
{
    int len;
    UDWORD res;
    enum DispStat stat;

//    assert(fdpp);

    s_regs = *regs;
    res = (libid ? disp : FdppThunkCall)(regs->d.ecx, sp, &stat, &len);
    *regs = s_regs;
    if (stat == DISP_NORET)
        return (res == ASM_NORET ? DJ64_RET_NORET : DJ64_RET_ABORT);
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
    return DJ64_RET_OK;
}

struct udisp {
    dj64dispatch_t *disp;
    dj64symtab_t *st;
};
#define MAX_HANDLES 10
static struct udisp udisps[MAX_HANDLES];
static const struct dj64_api *dj64api;

void djloudprintf(const char *format, ...)
{
    va_list vl;

    va_start(vl, format);
    dj64api->print(DJ64_PRINT_LOG, format, vl);
    va_end(vl);
    va_start(vl, format);
    dj64api->print(DJ64_PRINT_TERMINAL, format, vl);
    va_end(vl);
}

uint8_t *djaddr2ptr(uint32_t addr)
{
    return dj64api->addr2ptr(addr);
}

static int dj64_call(int handle, int libid, int fn, uint8_t *sp)
{
    int ret;
    struct udisp *u;
    __dpmi_regs *regs = (__dpmi_regs *)sp;
    sp += sizeof(*regs);
    assert(handle < MAX_HANDLES);
    u = &udisps[handle];
    recur_cnt++;
    ret = _dj64_call(libid, fn, regs, sp, u->disp);
    recur_cnt--;
    return ret;
}

static int dj64_ctrl(int handle, int libid, int fn, uint8_t *sp)
{
//    struct udisp *u;
//    struct dj64_symtab *st;
    __dpmi_regs *regs = (__dpmi_regs *)sp;
    assert(handle < MAX_HANDLES);
//    u = &udisps[handle];
    switch (fn) {
    case DL_SET_SYMTAB: {
        uint32_t addr = regs->d.ecx;
        const char *elf;
        djloudprintf("addr %x\n", addr);
        elf = (char *)djaddr2ptr(addr);
        djloudprintf("data %p(%s)\n", elf, elf);
        return 0;
    }
    }
    return -1;
}

static dj64cdispatch_t *ops[] = { dj64_call, dj64_ctrl };

dj64cdispatch_t **DJ64_INIT_FN(int handle,
    dj64dispatch_t *disp,
    dj64symtab_t *st)
{
    struct udisp *u;
    assert(handle < MAX_HANDLES);
    u = &udisps[handle];
    u->disp = disp;
    u->st = st;
    return ops;
}

int DJ64_INIT_ONCE_FN(const struct dj64_api *api, int api_ver)
{
    int ret = 0;
    if (api_ver != DJ64_API_VER)
        return -1;
    if (!dj64api)
        ret++;
    dj64api = api;
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
