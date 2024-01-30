/*
 *  Copyright (C) 2023  stsp
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
#include <sys/nearptr.h>
#include <dpmi.h>
#include <string.h>
#include <assert.h>
#include <sys/fmemcpy.h>

static inline int get_segment_base_address(int selector, unsigned *addr)
{
#ifdef __LP64__
    return __dpmi_get_segment_base_address(selector, addr);
#else
    return __dpmi_get_segment_base_address(selector, (unsigned long *)addr);
#endif
}

void fmemcpy1(__dpmi_paddr dst, const void *src, unsigned len)
{
    unsigned base;
    void *ptr;

    get_segment_base_address(dst.selector, &base);
    __djgpp_nearptr_enable();
    ptr = DATA_PTR(base + dst.offset32 + __djgpp_conventional_base);
    memcpy(ptr, src, len);
    __djgpp_nearptr_disable();
}

void fmemcpy2(void *dst, __dpmi_paddr src, unsigned len)
{
    unsigned base;
    const void *ptr;

    get_segment_base_address(src.selector, &base);
    __djgpp_nearptr_enable();
    ptr = DATA_PTR(base + src.offset32 + __djgpp_conventional_base);
    memcpy(dst, ptr, len);
    __djgpp_nearptr_disable();
}

/* similar to sys/movedata.h's movedata(), but the src/dst swapped! */
void fmemcpy12(__dpmi_paddr dst, __dpmi_paddr src, unsigned len)
{
    unsigned sbase, dbase;
    const void *sptr;
    void *dptr;

    get_segment_base_address(src.selector, &sbase);
    get_segment_base_address(dst.selector, &dbase);
    __djgpp_nearptr_enable();
    sptr = DATA_PTR(sbase + src.offset32 + __djgpp_conventional_base);
    dptr = DATA_PTR(dbase + dst.offset32 + __djgpp_conventional_base);
    memcpy(dptr, sptr, len);
    __djgpp_nearptr_disable();
}
