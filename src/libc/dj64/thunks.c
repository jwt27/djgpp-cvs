#include <assert.h>
#include <djdev64/dj64init.h>
#include <libc/djthunks.h>
#include <dpmi.h>
#include <stddef.h>
#include <crt0.h>
#include "plt.h"
#include "dosobj.h"
#include "thunks_c.h"
#include "thunks_p.h"
#include "thunks_a.h"

static dpmi_regs s_regs;
static unsigned _cs;
static int recur_cnt;

struct udisp {
    dj64dispatch_t *disp;
    const struct elf_ops *eops;
};
#define MAX_HANDLES 10
static struct udisp udisps[MAX_HANDLES];
static const struct dj64_api *dj64api;

void djloudprintf(const char *format, ...)
{
    va_list vl;

    va_start(vl, format);
    dj64api->print(DJ64_PRINT_LOG, format, vl);
    va_end(vl);
    va_start(vl, format);
    dj64api->print(DJ64_PRINT_TERMINAL, format, vl);
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

uint32_t djptr2addr(const uint8_t *ptr)
{
    return dj64api->ptr2addr(ptr);
}

static int _dj64_call(int libid, int fn, dpmi_regs *regs, uint8_t *sp,
    unsigned esi, dj64dispatch_t *disp)
{
    int len;
    UDWORD res;
    enum DispStat stat;

//    assert(fdpp);

    s_regs = *regs;
    _cs = esi;
    res = (libid ? disp : dj64_thunk_call)(fn, sp, &stat, &len);
    *regs = s_regs;
    if (stat == DISP_NORET)
        return (res == ASM_NORET ? DJ64_RET_NORET : DJ64_RET_ABORT);
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
    struct udisp *u;
    dpmi_regs *regs = (dpmi_regs *)sp;
    sp += sizeof(*regs);
    assert(handle < MAX_HANDLES);
    u = &udisps[handle];
    recur_cnt++;
    ret = _dj64_call(libid, fn, regs, sp, esi, u->disp);
    recur_cnt--;
    return ret;
}

static int dj64_ctrl(int handle, int libid, int fn, unsigned esi, uint8_t *sp)
{
//    struct udisp *u;
//    struct dj64_symtab *st;
    dpmi_regs *regs = (dpmi_regs *)sp;
    assert(handle < MAX_HANDLES);
//    u = &udisps[handle];
    switch (fn) {
    case DL_SET_SYMTAB: {
        struct udisp *u = &udisps[handle];
        uint32_t addr = regs->ebx;
        uint32_t size = regs->ecx;
        uint32_t mem_base = regs->edx;
        char *elf;
        void *eh;
        int i, ret = 0;
        djloudprintf("addr 0x%x mem_base 0x%x\n", addr, mem_base);
        elf = (char *)djaddr2ptr(addr);
        djloudprintf("data %p(%s)\n", elf, elf);
        eh = u->eops->open(elf, size);
        for (i = 0; i < num_athunks; i++) {
            struct athunk *t = &asm_thunks[i];
            uint32_t off = u->eops->getsym(eh, t->name);
            if (off) {
                *t->ptr = djaddr2ptr(mem_base + off);
            } else {
                djloudprintf("symbol %s not resolved\n", t->name);
                ret = -1;
                break;
            }
        }
        for (i = 0; i < num_cthunks; i++) {
            struct athunk *t = &asm_cthunks[i];
            asm_tab[i] = u->eops->getsym(eh, t->name);
            if (!asm_tab[i]) {
                djloudprintf("symbol %s not resolved\n", t->name);
                ret = -1;
                break;
            }
        }
        u->eops->close(eh);
        return ret;
    }
    }
    return -1;
}

void dj64_init(void)
{
    dosobj_init(dosobj_page, 4096);
}

static dj64cdispatch_t *dops[] = { dj64_call, dj64_ctrl };

dj64cdispatch_t **DJ64_INIT_FN(int handle,
    dj64dispatch_t *disp, const struct elf_ops *ops)
{
    struct udisp *u;
    assert(handle < MAX_HANDLES);
    u = &udisps[handle];
    u->disp = disp;
    u->eops = ops;
    return dops;
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

uint32_t do_asm_call(int num, uint8_t *sp, uint8_t len, int flags)
{
    int rc;
    dpmi_paddr pma;
    assert(num < num_cthunks);
    pma.selector = _cs;
    pma.offset32 = asm_tab[num];
    djlogprintf("asm call %s: 0x%x:0x%x\n", asm_cthunks[num].name,
            pma.selector, pma.offset32);
    rc = dj64api->asm_call(&s_regs, pma, sp, len);
    djlogprintf("asm call %s returned %i:0x%x\n", asm_cthunks[num].name,
            rc, s_regs.eax);
    switch (rc) {
    case ASM_CALL_OK:
        break;
    case ASM_CALL_ABORT:
        djloudprintf("reboot jump, %i\n", recur_cnt);
//        fdpp_noret(ASM_ABORT);
        break;
    }
    return s_regs.eax;
}

uint8_t *clean_stk(size_t len)
{
    s_regs.esp += len;
    return NULL;
}
