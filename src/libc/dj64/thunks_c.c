#include <djdev64/dj64thnk.h>
#include "thunk_incs.h"
#include "thunks_c.h"

enum { ASM_CALL_OK, ASM_CALL_ABORT };
#define ___assert(x)

#define _ARG(n, t, ap) (*(t *)(ap + n))
#define _ARG_PTR(n, t, ap) ((t *)(ap + n))
#define _ARG_R(t) t
#define _ARG_RPTR(t) t *
#define _RET(r) r
#define _RET_PTR(r) PTR_DATA(r)

UDWORD dj64_thunk_call(int fn, UBYTE *sp, enum DispStat *r_stat, int *r_len)
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
