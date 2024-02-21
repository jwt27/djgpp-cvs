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

#include <dj64/thunks_a.h>

#ifndef IN_ASMOBJ
#include <dj64/asm_inc.h>
#define IN_ASMOBJ 3
#include <dj64/asm_inc.h>
#endif

#define __S(x) #x
#define _S(x) __S(x)
#define _ASM(v) { _S(_##v), &__##v, 0 }
#define __ASM(t, v) _ASM(v)
#define __ASM_PTR(t, v) _ASM(v)
#define __ASM_ARR(t, v) _ASM(v)
#define __ASM_FUNC(v) _ASM(v)
#define __ASM_N(t, v) _ASM(v)
#define SEMIC ,
struct athunk _U(asm_thunks)[] = {
    #include "glob_asm.h"
};

const int _U(num_athunks) = _countof(_U(asm_thunks));

#ifndef NO_ASMC_THUNKS
struct athunk _U(asm_cthunks)[] = {
#define ASMCSYM(s, n) [n] = { _S(_##s), NULL, 0 },
#include "plt_asmc.h"
};

const int _U(num_cthunks) = _countof(_U(asm_cthunks));

uint32_t _U(asm_tab)[_countof(_U(asm_cthunks))];
#endif
