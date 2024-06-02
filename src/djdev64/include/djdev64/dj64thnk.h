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

#ifndef DJ64THNK_H
#define DJ64THNK_H

#include <stdint.h>

enum { DJ64_RET_ABORT = -1, DJ64_RET_OK, DJ64_RET_NORET };

#ifdef _IN_DJ64
#define DJ64_DISPATCH_FN dj64_thunk_call
#else
#define DJ64_DISPATCH_FN dj64dispatch
#endif

typedef uint32_t (dj64dispatch_t)(int fn, uint8_t *sp, int *r_len);
uint32_t DJ64_DISPATCH_FN(int fn, uint8_t *sp, int *r_len);

#endif
