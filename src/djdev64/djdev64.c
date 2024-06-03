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
#include <pthread.h>
#include "djdev64/dj64init.h"
#include "djdev64/djdev64.h"
#include "elf_priv.h"

static int handles;
#define HNDL_MAX 5
struct dj64handle {
    void *dlobj;
    dj64cdispatch_t *cdisp;
    dj64cdispatch_t *ctrl;
};
static struct dj64handle dlhs[HNDL_MAX];

static pthread_mutex_t init_mtx = PTHREAD_MUTEX_INITIALIZER;

static const struct elf_ops eops = {
    elf_open,
    elf_close,
    elf_getsymoff,
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

static int _djdev64_open(const char *path, const struct dj64_api *api,
    int api_ver)
{
  int h, rc;
  dj64init_t *init;
  dj64init_once_t *init_once;
  dj64cdispatch_t **cdisp;
  void *th;
  int *nth;
  void *dlh;
  const char **v;

  if (handles >= HNDL_MAX) {
    fprintf(stderr, "out of handles\n");
    return -1;
  }
  dlh = dlmopen(LM_ID_NEWLM, path, RTLD_LOCAL | RTLD_NOW);
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
  init_once = dlsym(dlh, _S(DJ64_INIT_ONCE_FN));
  if (!init_once) {
    fprintf(stderr, "cannot find " _S(DJ64_INIT_ONCE_FN) "\n");
    dlclose(dlh);
    return -1;
  }
  init = dlsym(dlh, _S(DJ64_INIT_FN));
  if (!init) {
    fprintf(stderr, "cannot find " _S(DJ64_INIT_FN) "\n");
    dlclose(dlh);
    return -1;
  }
  th = dlsym(dlh, "asm_thunks_user");
  if (!th) {
    fprintf(stderr, "cannot find asm_thunks_user\n");
    dlclose(dlh);
    return -1;
  }
  nth = dlsym(dlh, "num_athunks_user");
  if (!nth) {
    fprintf(stderr, "cannot find num_athunks_user\n");
    dlclose(dlh);
    return -1;
  }
  rc = init_once(api, api_ver);
  if (rc == -1) {
    fprintf(stderr, _S(DJ64_INIT_ONCE_FN) " failed\n");
    dlclose(dlh);
    return -1;
  }
  cdisp = init(handles, &eops, th, *nth);
  if (!cdisp) {
    fprintf(stderr, _S(DJ64_INIT_FN) " failed\n");
    dlclose(dlh);
    return -1;
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

  h = handles++;
  dlhs[h].dlobj = dlh;
  dlhs[h].cdisp = cdisp[0];
  dlhs[h].ctrl = cdisp[1];
  return h;
}

int djdev64_open(const char *path, const struct dj64_api *api, int api_ver)
{
  int ret;

  /* Init sequence is inherently thread-unsafe: at dlmopen() the ctors
   * register the dispatch fn, which is stored in a global pointer until
   * init() is called. Also we increment handles non-atomically. */
  pthread_mutex_lock(&init_mtx);
  ret = _djdev64_open(path, api, api_ver);
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
    if (handle >= handles)
        return;
    dlclose(dlhs[handle].dlobj);
    dlhs[handle].dlobj = NULL;
    while (handles > 0 && !dlhs[handles - 1].dlobj)
        handles--;
}
