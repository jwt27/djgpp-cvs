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

#include <setjmp.h>
#include <assert.h>
#include <djdev64/dj64init.h>
#include <dj64/thunks_p.h>
#include <dj64/util.h>
#include <libc/djthunks.h>
#include <libc/internal.h>
#include <dpmi.h>
#include <sys/nearptr.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <crt0.h>
#include "thunks_a.h"
#include "thunks_c.h"
#include "plt.h"
#include "dosobj.h"

int __crt0_startup_flags;

static dpmi_regs s_regs;
static unsigned _cs;
static int recur_cnt;
static int objcnt;
static jmp_buf *noret_jmp;

struct udisp {
    dj64dispatch_t *disp;
    const struct elf_ops *eops;
    struct athunks *at;
    struct athunks *pt;
    struct athunks core_at;
    struct athunks core_pt;
    main_t *main;
};
#define MAX_HANDLES 10
static struct udisp udisps[MAX_HANDLES];
static const struct dj64_api *dj64api;
#define MAX_OBJS 50
#define MAX_RECUR 10
static uint32_t objs[MAX_RECUR][MAX_OBJS];
static dj64dispatch_t *disp_fn;
static struct athunks *u_athunks;
static struct athunks *u_pthunks;
static int *u_handle_p;

static void do_rm_dosobj(uint32_t fa);

void djloudvprintf(const char *format, va_list vl)
{
    va_list cpy;

    va_copy(cpy, vl);
    dj64api->print(DJ64_PRINT_LOG, format, cpy);
    va_end(cpy);
    dj64api->print(DJ64_PRINT_TERMINAL, format, vl);
}

void djloudprintf(const char *format, ...)
{
    va_list vl;

    va_start(vl, format);
    djloudvprintf(format, vl);
    va_end(vl);
}

void djlogprintf(const char *format, ...)
{
    va_list vl;

    va_start(vl, format);
    dj64api->print(DJ64_PRINT_LOG, format, vl);
    va_end(vl);
}

uint8_t *djaddr2ptr(uint32_t addr)
{
    return dj64api->addr2ptr(addr);
}

uint8_t *djaddr2ptr2(uint32_t addr, uint32_t len)
{
    return dj64api->addr2ptr2(addr, len);
}

uint32_t djptr2addr(const uint8_t *ptr)
{
    return dj64api->ptr2addr(ptr);
}

static int _dj64_call(int libid, int fn, dpmi_regs *regs, uint8_t *sp,
    unsigned esi, dj64dispatch_t *disp)
{
    int len;
    UDWORD res;
    int rc;
    jmp_buf noret;

    s_regs = *regs;
    if ((rc = setjmp(noret))) {
        int i;
        /* gc lost objects, esp in ABORT case */
        for (i = 0; i < MAX_OBJS; i++) {
            if (objs[recur_cnt - 1][i])
                do_rm_dosobj(objs[recur_cnt - 1][i]);
        }
        return (rc == ASM_NORET ? DJ64_RET_NORET : DJ64_RET_ABORT);
    }
    noret_jmp = &noret;
    res = (libid ? disp : dj64_thunk_call)(fn, sp, &len);
    *regs = s_regs;
    switch (len) {
    case 0:
        break;
    case 1:
    case 2:
    case 4:
        regs->eax = res;
        break;
    default:
//        _fail();
        break;
    }
    return DJ64_RET_OK;
}

static int dj64_call(int handle, int libid, int fn, unsigned esi, uint8_t *sp)
{
    int ret;
    int last_objcnt;
    struct udisp *u;
    jmp_buf *saved_noret;
    dpmi_regs *regs = (dpmi_regs *)sp;

    sp += sizeof(*regs) + 8;  // skip regs, ebp, eip to get stack args
    assert(handle < MAX_HANDLES);
    u = &udisps[handle];
    assert(recur_cnt < MAX_RECUR);
    recur_cnt++;
    saved_noret = noret_jmp;
    last_objcnt = objcnt;
    ret = _dj64_call(libid, fn, regs, sp, esi, u->disp);
    assert(objcnt == last_objcnt);  // make sure no leaks, esp on NORETURN
    noret_jmp = saved_noret;
    recur_cnt--;
    return ret;
}

static int process_athunks(struct athunks *at, uint32_t mem_base,
	const struct elf_ops *eops, void *eh)
{
    int i, ret = 0;

    for (i = 0; i < at->num; i++) {
        const struct athunk *t = &at->at[i];
        uint32_t off = eops->getsym(eh, t->name);
        if (off) {
            at->tab[i] = mem_base + off;
        } else {
            djloudprintf("symbol %s not resolved\n", t->name);
            ret = -1;
            break;
        }
    }
    return ret;
}

static int process_pthunks(struct athunks *pt,
	const struct elf_ops *eops, void *eh)
{
    int i, ret = 0;

    for (i = 0; i < pt->num; i++) {
        const struct athunk *t = &pt->at[i];
        pt->tab[i] = eops->getsym(eh, t->name);
        if (!pt->tab[i]) {
            djloudprintf("symbol %s not resolved\n", t->name);
            ret = -1;
            break;
        }
    }
    return ret;
}

static ASMh(int, _crt0_startup_flags)

static void do_early_init(int handle)
{
    *____crt0_startup_flags(handle) = __crt0_startup_flags;
}

static int dj64_ctrl(int handle, int libid, int fn, unsigned esi, uint8_t *sp)
{
    dpmi_regs *regs = (dpmi_regs *)sp;
    assert(handle < MAX_HANDLES);
    switch (fn) {
    case DL_SET_SYMTAB: {
        struct udisp *u = &udisps[handle];
        uint32_t addr = regs->ebx;
        uint32_t size = regs->ecx;
        uint32_t mem_base = regs->edx;
        char *elf;
        void *eh;
        int ret;

        _cs = esi;
        djlogprintf("addr 0x%x mem_base 0x%x\n", addr, mem_base);
        elf = (char *)djaddr2ptr(addr);
        djlogprintf("data %p(%s)\n", elf, elf);
        eh = u->eops->open(elf, size);
        if (!eh)
            return -1;
        ret = process_athunks(&u->core_at, mem_base, u->eops, eh);
        if (ret)
            goto err;
        ret = process_athunks(u->at, mem_base, u->eops, eh);
        if (ret)
            goto err;
        ret = process_pthunks(&u->core_pt, u->eops, eh);
        if (ret)
            goto err;
        if (u->pt) {
            ret = process_pthunks(u->pt, u->eops, eh);
            if (ret)
                goto err;
        }
        u->eops->close(eh);

        do_early_init(handle);
        return ret;
    err:
        u->eops->close(eh);
        return ret;
    }
    }
    return -1;
}

void dj64_init(void)
{
    __djgpp_nearptr_enable();  // speeds up things considerably
    dosobj_init(dosobj_page, 4096);
}

static dj64cdispatch_t *dops[] = { dj64_call, dj64_ctrl };

dj64cdispatch_t **DJ64_INIT_FN(int handle, const struct elf_ops *ops,
        void *main)
{
    struct udisp *u;

    assert(handle < MAX_HANDLES);
    u = &udisps[handle];
    u->disp = disp_fn;
    disp_fn = NULL;
    u->eops = ops;
    u->main = main;

    u->at = u_athunks;
    u_athunks = NULL;

    u->pt = u_pthunks;
    if (u_handle_p)
        *u_handle_p = handle;
    u_pthunks = NULL;
    u_handle_p = NULL;

    u->core_at = asm_thunks;
    u->core_at.tab = malloc(sizeof(asm_thunks.tab[0]) * asm_thunks.num);
    u->core_pt = pthunks;
    u->core_pt.tab = malloc(sizeof(pthunks.tab[0]) * pthunks.num);

    return dops;
}

void DJ64_DONE_FN(int handle)
{
    struct udisp *u;

    assert(handle < MAX_HANDLES);
    u = &udisps[handle];
    free(u->core_at.tab);
    free(u->core_pt.tab);
}

int DJ64_INIT_ONCE_FN(const struct dj64_api *api, int api_ver)
{
    int ret = 0;
    if (api_ver != DJ64_API_VER)
        return -1;
    if (!dj64api)
        ret++;
    dj64api = api;
    return ret;
}

static uint64_t do_asm_call(struct athunks *pt, int num, uint8_t *sp,
        uint8_t len, int flags)
{
    int rc;
    dpmi_paddr pma;
    assert(num < pt->num);
    pma.selector = _cs;
    pma.offset32 = pt->tab[num];
    if (flags & _TFLG_NORET) {
        djlogprintf("NORET call %s: 0x%x:0x%x\n", pt->at[num].name,
            pma.selector, pma.offset32);
        dj64api->asm_noret(&s_regs, pma, sp, len);
        longjmp(*noret_jmp, ASM_NORET);
    }
    djlogprintf("asm call %s: 0x%x:0x%x\n", pt->at[num].name,
            pma.selector, pma.offset32);
    rc = dj64api->asm_call(&s_regs, pma, sp, len);
    djlogprintf("asm call %s returned %i:0x%x\n", pt->at[num].name,
            rc, s_regs.eax);
    switch (rc) {
    case ASM_CALL_OK:
        break;
    case ASM_CALL_ABORT:
        djlogprintf("reboot jump, %i\n", recur_cnt);
        longjmp(*noret_jmp, ASM_ABORT);
        break;
    }
    return s_regs.eax | ((uint64_t)s_regs.edx << 32);
}

uint64_t dj64_asm_call(int num, uint8_t *sp, uint8_t len, int flags)
{
    int handle = dj64api->get_handle();
    assert(handle < MAX_HANDLES);
    return do_asm_call(&udisps[handle].core_pt, num, sp, len, flags);
}

uint64_t dj64_asm_call_u(int handle, int num, uint8_t *sp, uint8_t len,
        int flags)
{
    assert(handle < MAX_HANDLES);
    return do_asm_call(udisps[handle].pt, num, sp, len, flags);
}

uint8_t *dj64_clean_stk(size_t len)
{
    return dj64api->inc_esp(len);
}

static uint32_t *find_oh(uint32_t addr)
{
    int i;

    for (i = 0; i < MAX_OBJS; i++) {
        if (objs[recur_cnt - 1][i] == addr)
            return &objs[recur_cnt - 1][i];
    }
    return NULL;  // should not happen
}

uint32_t dj64_obj_init(const void *data, uint16_t len)
{
    uint32_t ret;
    if (dj64api->is_dos_ptr(data))
        return PTR_DATA(data);
    ret = mk_dosobj(len);
    pr_dosobj(ret, data, len);
    objcnt++;
    *find_oh(0) = ret;
    return ret;
}

void dj64_obj_done(void *data, uint32_t fa, uint16_t len)
{
    if (dj64api->is_dos_ptr(data))
        return;
    cp_dosobj(data, fa, len);
    rm_dosobj(fa);
    *find_oh(fa) = 0;
    objcnt--;
}

static void do_rm_dosobj(uint32_t fa)
{
    rm_dosobj(fa);
    *find_oh(fa) = 0;
    objcnt--;
}

void dj64_rm_dosobj(const void *data, uint32_t fa)
{
    if (dj64api->is_dos_ptr(data))
        return;
    do_rm_dosobj(fa);
}

void register_dispatch_fn(dj64dispatch_t *fn)
{
    assert(!disp_fn);
    disp_fn = fn;
}

void register_athunks(struct athunks *at)
{
    assert(!u_athunks);
    u_athunks = at;
}

void register_pthunks(struct athunks *pt, int *handle_p)
{
    assert(!u_pthunks);
    u_pthunks = pt;
    u_handle_p = handle_p;
}

void crt1_startup(int handle)
{
    assert(handle < MAX_HANDLES);
    __crt1_startup(udisps[handle].main);
}

static uint32_t do_thunk_get(const struct athunks *at, const char *name)
{
    int i;

    for (i = 0; i < at->num; i++) {
        if (strcmp(at->at[i].name, name) == 0)
            return at->tab[i];
    }
    return (uint32_t)-1;
}

uint32_t djthunk_get_h(int handle, const char *name)
{
    struct udisp *u;
    uint32_t ret;

    assert(handle < MAX_HANDLES);
    u = &udisps[handle];
    ret = do_thunk_get(u->at, name);
    if (ret == (uint32_t)-1)
        ret = do_thunk_get(&u->core_at, name);
    assert(ret != (uint32_t)-1);
    return ret;
}

uint32_t djthunk_get(const char *name)
{
    return djthunk_get_h(dj64api->get_handle(), name);
}
