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

#ifndef DJCTX_H
#define DJCTX_H

#include <stdlib.h>
#include <string.h>

void djregister_ctor_dtor(void (*ctor)(void), void (*dtor)(void));

#define DJ64_DEFINE_SWAPPABLE_CONTEXT2(t, c, init, pre, post) \
static void t##_init(void) \
{ \
  struct t *x = malloc(sizeof(*x)); \
  *x = init; \
  x->prev = c; \
  if (c) \
    pre; \
  c = x; \
  post; \
} \
static void t##_deinit(void) \
{ \
  struct t *x = c; \
  c = x->prev; \
  free(x); \
  if (c) \
    post; \
} \
__attribute__((constructor)) \
static void static_##t##_init(void) \
{ \
  djregister_ctor_dtor(t##_init, t##_deinit); \
}

#define DJ64_DEFINE_SWAPPABLE_CONTEXT(t, c) \
  static const struct t ctx_##t##_init = {}; \
  DJ64_DEFINE_SWAPPABLE_CONTEXT2(t, c, ctx_##t##_init,,)

#endif
