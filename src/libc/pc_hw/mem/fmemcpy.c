/*
 *  dj64 - 64bit djgpp-compatible tool-chain
 *  Copyright (C) 2024  @stsp
 *  Copyright (C) 2023  stsp, comcom32 project
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
 * NOTE: this file is a port of fmemcpy.c file from comcom32 project.
 * In comcom32 project it is distributed under the terms of GNU GPLv3+
 * and is copyrighted (C) 2023  stsp.
 * As a sole author of the aforementioned fmemcpy.c, I donate the
 * code to dj64dev project, allowing to re-license it under the terms
 * of GNU LGPLv3+.
 *
 * --stsp
 */

#include <sys/nearptr.h>
#include <dpmi.h>
#include <string.h>
#include <assert.h>
#include <crt0.h>
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
    int en_dis = !(_crt0_startup_flags & _CRT0_FLAG_NEARPTR);

    get_segment_base_address(dst.selector, &base);
    if (en_dis)
      __djgpp_nearptr_enable();
    ptr = DATA_PTR(base + dst.offset32 + __djgpp_conventional_base);
    memcpy(ptr, src, len);
    if (en_dis)
      __djgpp_nearptr_disable();
}

void fmemcpy2(void *dst, __dpmi_paddr src, unsigned len)
{
    unsigned base;
    const void *ptr;
    int en_dis = !(_crt0_startup_flags & _CRT0_FLAG_NEARPTR);

    get_segment_base_address(src.selector, &base);
    if (en_dis)
      __djgpp_nearptr_enable();
    ptr = DATA_PTR(base + src.offset32 + __djgpp_conventional_base);
    memcpy(dst, ptr, len);
    if (en_dis)
      __djgpp_nearptr_disable();
}

/* similar to sys/movedata.h's movedata(), but the src/dst swapped! */
void fmemcpy12(__dpmi_paddr dst, __dpmi_paddr src, unsigned len)
{
    unsigned sbase, dbase;
    const void *sptr;
    void *dptr;
    int en_dis = !(_crt0_startup_flags & _CRT0_FLAG_NEARPTR);

    get_segment_base_address(src.selector, &sbase);
    get_segment_base_address(dst.selector, &dbase);
    if (en_dis)
      __djgpp_nearptr_enable();
    sptr = DATA_PTR(sbase + src.offset32 + __djgpp_conventional_base);
    dptr = DATA_PTR(dbase + dst.offset32 + __djgpp_conventional_base);
    memcpy(dptr, sptr, len);
    if (en_dis)
      __djgpp_nearptr_disable();
}
