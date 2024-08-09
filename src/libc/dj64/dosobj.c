/*
 *  dj64 - 64bit djgpp-compatible tool-chain
 *  Copyright (C) 2021-2024  @stsp
 *
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2018  Stas Sergeev (stsp)
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

/*
 * NOTE: this file is a port of dosobj.cc file from fdpp project.
 * In fdpp project it is distributed under the terms of GNU GPLv3+
 * and is copyrighted (C) 2018  Stas Sergeev (stsp).
 * As a sole author of the aforementioned dosobj.cc, I donate the
 * code to dj64dev project, allowing to re-license it under the terms
 * of GNU LGPLv3+.
 *
 * --stsp
 */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dpmi.h>
#include <dj64/util.h>
#include <libc/djthunks.h>
#include <libc/djctx.h>
#include "smalloc.h"
#include "dosobj.h"

struct dost {
  smpool pool;
  uint32_t base;
  int initialized;
};

static struct dost *dst;
DJ64_DEFINE_SWAPPABLE_CONTEXT(dost, dst)
#define pool dst->pool
#define base dst->base
#define initialized dst->initialized

static void err_printf(int prio, const char *fmt, ...) PRINTF(2);
static void err_printf(int prio, const char *fmt, ...)
{
    va_list vl;

    va_start(vl, fmt);
    djloudvprintf(fmt, vl);
    va_end(vl);
}

void dosobj_init(void *ptr, int size)
{
    assert(!initialized);
    sminit(&pool, ptr, size);
    smregister_error_notifier(&pool, err_printf);
    base = PTR_DATA(ptr);
    initialized = 1;
}

static void do_free(void)
{
    int leaked;

    assert(initialized);
    leaked = smdestroy(&pool);
    assert(!leaked);
}

void dosobj_free(void)
{
    do_free();
    initialized = 0;
}

void dosobj_reinit(void *ptr, int size)
{
    do_free();
    sminit(&pool, ptr, size);
    base = PTR_DATA(ptr);
}

uint32_t mk_dosobj(uint16_t len)
{
    void *ptr;
    uint32_t offs;

    assert(initialized);
    ptr = smalloc(&pool, len);
    if (!ptr) {
        djloudprintf("dosobj: OOM! len=%i\n", len);
        abort();
    }
    offs = (uintptr_t)ptr - (uintptr_t)smget_base_addr(&pool);
    return base + offs;
}

void pr_dosobj(uint32_t fa, const void *data, uint16_t len)
{
    void *ptr = DATA_PTR(fa);

    memcpy(ptr, data, len);
}

void cp_dosobj(void *data, uint32_t fa, uint16_t len)
{
    void *ptr = DATA_PTR(fa);

    memcpy(data, ptr, len);
}

void rm_dosobj(uint32_t fa)
{
    void *ptr = (char *)smget_base_addr(&pool) + (fa - base);

    smfree(&pool, ptr);
}

void dosobj_dump(void)
{
    smdump(&pool);
}
