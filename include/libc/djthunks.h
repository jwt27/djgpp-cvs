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

#ifndef DJTHUNKS_H
#define DJTHUNKS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void crt1_startup(int handle);

uint8_t *djaddr2ptr(uint32_t addr);
uint8_t *djaddr2ptr2(uint32_t addr, uint32_t len);
uint32_t djptr2addr(const uint8_t *ptr);
uint32_t djthunk_get(const char *name);
uint32_t djthunk_get_h(int handle, const char *name);
void djregister_ctor_dtor(void (*ctor)(void), void (*dtor)(void));

#define DJ64_DEFINE_SWAPPABLE_CONTEXT(t, c) \
static void t##_init(void) \
{ \
  struct t *x = malloc(sizeof(*x)); \
  memset(x, 0, sizeof(*x)); \
  x->prev = c; \
  c = x; \
} \
static void t##_deinit(void) \
{ \
  struct t *x = c; \
  c = x->prev; \
  free(x); \
} \
__attribute__((constructor)) \
static void static_##t##_init(void) \
{ \
  djregister_ctor_dtor(t##_init, t##_deinit); \
}

#endif
