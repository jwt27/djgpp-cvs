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

#include <libc/djthunks.h>

#ifndef _f
#define _h #
#define _f(x) x
#define DEFINE(x, y) _f(_h)define x y
#endif

#ifndef IN_ASMOBJ

#define ASM(x) *__##x;/*
*/DEFINE(x, (*__##x))/*
*/

#define ASM_N(x) *__##x;/*
*/DEFINE(x, (*__##x))/*
*/

extern int *____djgpp_base_address;
#define _DP(l) djaddr2ptr(*____djgpp_base_address + (l))
#define ASM_P(t, x) unsigned *__##x;/*
*/DEFINE(x, ((t *)_DP(*__##x)))/*
*/

#define ASM_ARR(x) *__##x;/*
*/DEFINE(x, __##x)/*
*/

#define _PD(p) (djptr2addr(p) - *____djgpp_base_address)
#define ASM_F(x) unsigned char *__##x;/*
*/DEFINE(x, _PD(__##x))/*
*/

#define EXTERN extern

#else

#if IN_ASMOBJ == 1
#define ASM_N(x) _##x
#endif

#if IN_ASMOBJ == 2
#define ASM_N(x) *__##x
#endif

#if IN_ASMOBJ == 3

#undef ASM
#define ASM(x) *__##x

#undef ASM_F
#define ASM_F(x) unsigned char *__##x

#undef ASM_N
#define ASM_N(x) extern *__##x

#undef ASM_P
#define ASM_P(t, x) unsigned *__##x

#undef ASM_ARR
#define ASM_ARR(x) *__##x

#endif

#undef EXTERN
#define EXTERN

#endif
