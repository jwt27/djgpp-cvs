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

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "djdev64/dj64init.h"
#include "djdev64/djdev64.h"
#include "elf_priv.h"
#ifdef __GLIBC__
#define HAVE_DLMOPEN 1
#endif

static int handles;
#define HNDL_MAX 5
struct dj64handle {
    void *dlobj;
    dj64cdispatch_t *cdisp;
    dj64cdispatch_t *ctrl;
    dj64done_t *done;
    char *path;
};
static struct dj64handle dlhs[HNDL_MAX];

static pthread_mutex_t init_mtx = PTHREAD_MUTEX_INITIALIZER;

static const struct elf_ops eops = {
    djelf_open,
    djelf_close,
    djelf_getsymoff,
};

#define __S(x) #x
#define _S(x) __S(x)
#define FORMAT(T,A,B) __attribute__((format(T,A,B)))

FORMAT(printf, 2, 3)
static void loudprintf(const struct dj64_api *api, const char *str, ...)
{
    va_list args;

    va_start(args, str);
    api->print(DJ64_PRINT_LOG, str, args);
    va_end(args);
    va_start(args, str);
    api->print(DJ64_PRINT_TERMINAL, str, args);
    va_end(args);
    va_start(args, str);
    api->print(DJ64_PRINT_SCREEN, str, args);
    va_end(args);
}

static char *findmnt(const char *path)
{
    const char *tmpl = "findmnt -n -o target --target %s";
    static char buf[1024];
    FILE *f;
    int rd;

    snprintf(buf, sizeof(buf), tmpl, path);
    f = popen(buf, "r");
    if (!f)
        return NULL;
    rd = fread(buf, 1, sizeof(buf), f);
    if (rd <= 0)
        goto err;
    if (buf[rd - 1] == '\n')
        rd--;
    buf[rd] = '\0';
    pclose(f);
    return buf;

err:
    pclose(f);
    return NULL;
}

static const char *var_list[] = { "_crt0_startup_flags", NULL };

static void *emu_dlmopen(int handle, const char *filename, int flags,
    char **r_path)
{
    int err;
    int len = strlen(filename) + 1 + 16;
    char *path = malloc(len);
    const char *p;
    void *ret;

    /* fake soname to cheat same-lib detection */
    snprintf(path, len, "%.*s.%i", (int)strlen(filename), filename, handle);
    p = strrchr(filename, '/');
    if (!p)
        p = filename;
    else
        p++;
    unlink(path);
    err = symlink(filename, path);
    if (err) {
        perror("symlink()");
        goto err_free;
    }
    ret = dlopen(path, flags);
    if (!ret)
        goto err_free;

    *r_path = path;
    return ret;

err_free:
    unlink(path);
    free(path);
    return NULL;
}

#define FLG_STATIC 1

static int _djdev64_open(const char *path, const struct dj64_api *api,
    int api_ver, unsigned flags)
{
    int rc;
    dj64init_t *init;
    dj64done_t *done;
    dj64init_once_t *init_once;
    dj64cdispatch_t **cdisp;
    void *main;
    void *dlh;
    const char **v;
    char *path2 = NULL;
    struct dj64handle *h;
    int use_dlm = 0;

    if (handles >= HNDL_MAX) {
        fprintf(stderr, "out of handles\n");
        return -1;
    }
    /* RTLD_DEEPBIND has similar effect to -Wl,-Bsymbolic */
    if (flags & FLG_STATIC) {
        /* for static linking we fully emulated dlmopen(), so set use_dlm */
        use_dlm = 1;
        dlh = emu_dlmopen(handles, path, RTLD_LOCAL | RTLD_NOW | RTLD_DEEPBIND,
                &path2);
    } else {
#ifdef HAVE_DLMOPEN
        use_dlm = 1;
        dlh = dlmopen(LM_ID_NEWLM, path, RTLD_LOCAL | RTLD_NOW | RTLD_DEEPBIND);
#else
        fprintf(stderr, "dlmopen() not supported, use static linking\n");
        dlh = emu_dlmopen(handles, path, RTLD_LOCAL | RTLD_NOW | RTLD_DEEPBIND,
                &path2);
#endif
    }
    if (!dlh) {
        char cmd[1024];
        const char *d = findmnt(path);
        fprintf(stderr, "cannot dlopen %s: %s\n", path, dlerror());
        snprintf(cmd, sizeof(cmd), "grep %.256s /proc/mounts | grep noexec", d);
        if (system(cmd) == 0) {
            loudprintf(api, "\nDJ64 ERROR: Your %s is mounted with noexec option.\n"
                      "Please execute:\n"
                      "\tsudo mount -o remount,exec %s\n"
                      "and try running the program again.\n",
                      d, d
            );
        }
        return -1;
    }

    main = dlsym(dlh, "main");
    if (!main) {
        fprintf(stderr, "cannot find main\n");
        goto err_close;
    }
    init_once = dlsym(dlh, _S(DJ64_INIT_ONCE_FN));
    if (!init_once) {
        fprintf(stderr, "cannot find " _S(DJ64_INIT_ONCE_FN) "\n");
        goto err_close;
    }
    init = dlsym(dlh, _S(DJ64_INIT_FN));
    if (!init) {
        fprintf(stderr, "cannot find " _S(DJ64_INIT_FN) "\n");
        goto err_close;
    }
    done = dlsym(dlh, _S(DJ64_DONE_FN));
    if (!done) {
        fprintf(stderr, "cannot find " _S(DJ64_DONE_FN) "\n");
        goto err_close;
    }

    rc = init_once(api, api_ver);
    if (rc == -1) {
        fprintf(stderr, _S(DJ64_INIT_ONCE_FN) " failed\n");
        goto err_close;
    }
    cdisp = init(handles, &eops, main, use_dlm);
    if (!cdisp) {
        fprintf(stderr, _S(DJ64_INIT_FN) " failed\n");
        goto err_close;
    }

    for (v = var_list; *v; v++) {
        char buf[256];
        int *vh1, *vh2;

        snprintf(buf, sizeof(buf), "_%s", *v);
        vh1 = dlsym(dlh, *v);
        vh2 = dlsym(dlh, buf);
        if (vh1 && vh2)
            *vh2 = *vh1;
    }

    h = &dlhs[handles];
    h->dlobj = dlh;
    h->cdisp = cdisp[0];
    h->ctrl = cdisp[1];
    h->done = done;
    h->path = path2;
    return handles++;

err_close:
    dlclose(dlh);
    if (path2) {
        int err = unlink(path2);
        if (err)
            perror("unlink()");
        free(path2);
    }
    return -1;
}

int djdev64_open(const char *path, const struct dj64_api *api, int api_ver,
    unsigned flags)
{
    int ret;

    /* Init sequence is inherently thread-unsafe: at dlmopen() the ctors
     * register the dispatch fn, which is stored in a global pointer until
     * init() is called. Also we increment handles non-atomically. */
    pthread_mutex_lock(&init_mtx);
    ret = _djdev64_open(path, api, api_ver, flags);
    pthread_mutex_unlock(&init_mtx);
    return ret;
}

int djdev64_call(int handle, int libid, int fn, unsigned esi,
        unsigned char *sp)
{
    if (handle >= handles || !dlhs[handle].dlobj)
        return -1;
    return dlhs[handle].cdisp(handle, libid, fn, esi, sp);
}

int djdev64_ctrl(int handle, int libid, int fn, unsigned esi,
        unsigned char *sp)
{
    if (handle >= handles || !dlhs[handle].dlobj)
        return -1;
    return dlhs[handle].ctrl(handle, libid, fn, esi, sp);
}

void djdev64_close(int handle)
{
    struct dj64handle *h;
    int err;

    if (handle >= handles)
        return;
    h = &dlhs[handle];
    h->done(handle);
    dlclose(h->dlobj);
    h->dlobj = NULL;
    if (h->path) {
        err = unlink(h->path);
        if (err)
            perror("unlink()");
        free(h->path);
    }
    while (handles > 0 && !dlhs[handles - 1].dlobj)
        handles--;
}
