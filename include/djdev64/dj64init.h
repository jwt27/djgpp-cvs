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

#ifndef DJ64INIT_H
#define DJ64INIT_H

#include <stdarg.h>
#include "dj64thnk.h"

typedef int (dj64cdispatch_t)(int handle, int libid, int fn, unsigned esi,
        uint8_t *sp);
#define DJ64_API_VER 2
enum { DJ64_PRINT_LOG, DJ64_PRINT_TERMINAL };

/* pushal */
typedef struct {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
} __attribute__((packed)) dpmi_regs;

typedef struct {
  uint32_t offset32;
  unsigned short selector;
} dpmi_paddr;

enum { ASM_CALL_OK, ASM_CALL_ABORT };

struct dj64_api {
    uint8_t *(*addr2ptr)(uint32_t addr);
    uint8_t *(*addr2ptr2)(uint32_t addr, uint32_t len);
    uint32_t (*ptr2addr)(const uint8_t *ptr);
    void (*print)(int prio, const char *format, va_list ap);
    int (*asm_call)(dpmi_regs *regs, dpmi_paddr pma, uint8_t *sp, uint8_t len);
    void (*asm_noret)(dpmi_regs *regs, dpmi_paddr pma, uint8_t *sp,
            uint8_t len);
    uint8_t *(*inc_esp)(uint32_t len);
};
#define DJ64_INIT_ONCE_FN dj64init_once
typedef int (dj64init_once_t)(const struct dj64_api *api, int api_ver);
int DJ64_INIT_ONCE_FN(const struct dj64_api *api, int api_ver);

struct elf_ops {
    void *(*open)(char *addr, uint32_t size);
    void (*close)(void *arg);
    uint32_t (*getsym)(void *arg, const char *name);
};

#define DJ64_INIT_FN dj64init
typedef dj64cdispatch_t **(dj64init_t)(int handle,
    dj64dispatch_t *disp, const struct elf_ops *ops,
    void *athunks, int num_athunks);
dj64cdispatch_t **DJ64_INIT_FN(int handle,
    dj64dispatch_t *disp, const struct elf_ops *ops,
    void *athunks, int num_athunks);

#endif
