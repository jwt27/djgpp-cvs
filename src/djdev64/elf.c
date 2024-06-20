/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2021  Stas Sergeev (stsp)
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <libelf.h>
#include <gelf.h>
#include <assert.h>
#include "elf_priv.h"

struct elfstate {
    size_t mapsize;
    Elf *elf;
    Elf_Scn *symtab_scn;
    GElf_Shdr symtab_shdr;
    char *addr;
};

static int do_getsym(struct elfstate *state, const char *name, GElf_Sym *r_sym)
{
    Elf_Data *data;
    int count, i;

    data = elf_getdata(state->symtab_scn, NULL);
    count = state->symtab_shdr.sh_size / state->symtab_shdr.sh_entsize;

    for (i = 0; i < count; i++) {
        GElf_Sym sym;
        gelf_getsym(data, i, &sym);
        if (strcmp(elf_strptr(state->elf, state->symtab_shdr.sh_link,
                sym.st_name), name) == 0) {
            *r_sym = sym;
            return 0;
        }
    }

    return -1;
}

static int do_elf_open(char *addr, uint32_t size, struct elfstate *ret)
{
    Elf         *elf;
    Elf_Scn     *scn = NULL;
    GElf_Shdr   shdr;

    elf_version(EV_CURRENT);

    elf = elf_memory(addr, size);
    if (!elf) {
        fprintf(stderr, "elf_memory() failed\n");
        return -1;
    }

    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        gelf_getshdr(scn, &shdr);
        if (shdr.sh_type == SHT_SYMTAB) {
            /* found a symbol table, go print it. */
            break;
        }
    }
    if (!scn) {
        fprintf(stderr, "elf: not found SHT_SYMTAB\n");
        goto err;
    }

    ret->mapsize = size;
    ret->elf = elf;
    ret->symtab_scn = scn;
    ret->symtab_shdr = shdr;
    return 0;

err:
    elf_end(elf);
    return -1;
}

void *djelf_open(char *addr, uint32_t size)
{
    struct elfstate es = {}, *ret;
    int err = do_elf_open(addr, size, &es);
    if (err)
        return NULL;
    ret = (struct elfstate *)malloc(sizeof(*ret));
    *ret = es;
    return ret;
}

void *djelf_open_dyn(void)
{
    struct elfstate es = {}, *ret;
    int err;
    int fd;
    char *addr;
    struct stat st;

    fd = open(CRT0, O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        perror("open()");
        return NULL;
    }
    fstat(fd, &st);
    addr = (char *)mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE,
        fd, 0);
    close(fd);
    if (addr == MAP_FAILED) {
        perror("mmap()");
        return NULL;
    }
    err = do_elf_open(addr, st.st_size, &es);
    if (err)
        return NULL;
    ret = (struct elfstate *)malloc(sizeof(*ret));
    *ret = es;
    ret->addr = addr;
    return ret;
}

void djelf_close(void *arg)
{
    struct elfstate *state = (struct elfstate *)arg;

    elf_end(state->elf);
    if (state->addr)
        munmap(state->addr, state->mapsize);
    free(state);
}

static uint32_t do_getsymoff(struct elfstate *state, const char *name)
{
    GElf_Sym sym;
    int err = do_getsym(state, name, &sym);
    assert(err || sym.st_value);  // make sure st_value!=0
    return (err ? 0 : sym.st_value);
}

uint32_t djelf_getsymoff(void *arg, const char *name)
{
    struct elfstate *state = (struct elfstate *)arg;
    return do_getsymoff(state, name);
}
