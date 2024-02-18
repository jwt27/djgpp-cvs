#include <djdev64/dj64thnk.h>
#include <dj64/util.h>
#include <dj64/thunks_c.h>

#define ___assert(x)

#define _ARG(n, t, ap) (*(t *)(ap + n))
#define _ARG_PTR(n, t, ap) ((t *)(ap + n))
#define _ARG_R(t) t
#define _ARG_RPTR(t) t *
#define _RET(r) r
#define _RET_PTR(r) PTR_DATA(r)

UDWORD DJ64_DISPATCH_FN(int fn, UBYTE *sp, int *r_len)
{
    UDWORD ret;
    UBYTE rsz = 0;

#define _SP sp
#define _DISP_CMN(f, c) { \
    djlogprintf("DJ64: dispatch " #f "\n"); \
    c; \
    djlogprintf("DJ64: dispatch " #f " done\n"); \
}
#define _DISPATCH(r, rv, rc, f, ...) _DISP_CMN(f, { \
    rv _r; \
    _r = f(__VA_ARGS__); \
    ret = rc(_r); \
    rsz = (r); \
})
#define _DISPATCH_v(f, ...) _DISP_CMN(f, { \
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

    *r_len = rsz;
    return ret;
}
