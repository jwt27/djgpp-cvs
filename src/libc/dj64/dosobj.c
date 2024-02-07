/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2018  Stas Sergeev (stsp)
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

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dpmi.h>
#include "libc/djthunks.h"
#include "smalloc.h"
#include "dosobj.h"

static smpool pool;
static uint32_t base;
static int initialized;

static void err_printf(int prio, const char *fmt, ...) PRINTF(2);
static void err_printf(int prio, const char *fmt, ...)
{
    va_list vl;

    va_start(vl, fmt);
    vprintf(fmt, vl);
    va_end(vl);
}

void dosobj_init(uint32_t fa, int size)
{
    void *ptr = DATA_PTR(fa);

    if (initialized)
        smdestroy(&pool);
    sminit(&pool, ptr, size);
    smregister_error_notifier(&pool, err_printf);
    base = fa;
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

void dosobj_reinit(uint32_t fa, int size)
{
    void *ptr = DATA_PTR(fa);

    do_free();
    sminit(&pool, ptr, size);
    base = fa;
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
    void *ptr = DATA_PTR(fa);

    smfree(&pool, ptr);
}

void dosobj_dump(void)
{
    smdump(&pool);
}
