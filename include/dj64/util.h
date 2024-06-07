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

#ifndef DJ64UTIL_H
#define DJ64UTIL_H

#define PRINTF(n) __attribute__((format(printf, n, n + 1)))
void djloudprintf(const char *format, ...) PRINTF(1);
void djlogprintf(const char *format, ...) PRINTF(1);

typedef uint32_t (dj64dispatch_t)(int fn, uint8_t *sp, int *r_len);
void register_dispatch_fn(dj64dispatch_t *fn);
struct pthunks;
void register_pthunks(struct pthunks *pt, int *handle_p);

struct athunk {
    const char *name;
    unsigned *ptr;
    unsigned flags;
};

#ifdef _IN_DJ64
#define _U(x) x
#else
#define _U(x) x##_user
#endif

#define __S(x) #x
#define _S(x) __S(x)

#define _countof(array) (sizeof(array) / sizeof(array[0]))

#endif
