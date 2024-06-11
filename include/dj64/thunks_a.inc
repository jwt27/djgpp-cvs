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

#include <dj64/util.h>
#ifdef _IN_DJ64
#include "thunks_a.h"
#endif

#ifndef IN_ASMOBJ
#include <dj64/asm_inc.h>
#define IN_ASMOBJ 3
#include <dj64/asm_inc.h>
#endif

#define _ASM(v) { _S(_##v), &__##v }
#define __ASM(t, v) _ASM(v)
#define __ASM_PTR(t, v) _ASM(v)
#define __ASM_ARR(t, v, l) _ASM(v)
#define __ASM_FUNC(v) _ASM(v)
#define __ASM_N(t, v) _ASM(v)
#define SEMIC ,

#ifndef _IN_DJ64
#define STATIC static
#else
#define STATIC
#endif

STATIC struct athunk _U(asm_thunks)[] = {
    #include "glob_asm.h"
};

STATIC const int _U(num_athunks) = _countof(_U(asm_thunks));

#ifndef _IN_DJ64
__attribute__((constructor))
static void init(void)
{
    register_athunks(_U(asm_thunks), _U(num_athunks));
}
#endif
