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

#include <assert.h>

void djregister_ctx_hooks(void (*)(int), void (*)(void), void (*)(void),
    void (*)(int));

#define SW_CTX_MAX 100

#define DJ64_DEFINE_SWAPPABLE_CONTEXT2(t, c, init, pre, post) \
static struct t t##_contexts[SW_CTX_MAX]; \
static void t##_init(int idx) \
{ \
  struct t *x = &t##_contexts[idx]; \
  assert(c != x); \
  *x = init; \
} \
static void t##_deinit(void) \
{ \
  c = NULL; \
} \
static void t##_save(void) \
{ \
  pre; \
} \
static void t##_restore(int idx) \
{ \
  struct t *x = &t##_contexts[idx]; \
  if (c == x) \
    return; \
  c = x; \
  post; \
} \
__attribute__((constructor)) \
static void static_##t##_init(void) \
{ \
  djregister_ctx_hooks(t##_init, t##_deinit, t##_save, t##_restore); \
}

#define DJ64_DEFINE_SWAPPABLE_CONTEXT(t, c) \
  static const struct t ctx_##t##_init = {}; \
  DJ64_DEFINE_SWAPPABLE_CONTEXT2(t, c, ctx_##t##_init,,)

#endif
