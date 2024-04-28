/*
 *  dj64 - 64bit djgpp-compatible tool-chain
 *  Copyright (C) 2021-2024  @stsp
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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