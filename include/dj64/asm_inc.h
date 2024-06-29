/*
 *  dj64 - 64bit djgpp-compatible tool-chain
 *  Copyright (C) 2021-2024  @stsp
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <libc/asmobj.h>

#define __ASM(t, v) EXTERN ASM(t, v)
#define __ASM_PTR(t, v) EXTERN ASM_P(t, v)
#define __ASM_ARR(t, v, l) EXTERN ASM_ARR(t, v, l)
#define __ASM_FUNC(v) EXTERN ASM_F(v)
#define __ASM_N(t, v) EXTERN ASM_N(t, v)
#define SEMIC ;
#include "glob_asm.h"
#undef __ASM
#undef __ASM_PTR
#undef __ASM_ARR
#undef __ASM_FUNC
#undef __ASM_N
#undef SEMIC
