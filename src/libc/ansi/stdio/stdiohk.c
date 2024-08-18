/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <libc/file.h>
#include <libc/local.h>
#include <libc/djctx.h>

static void fcloseall_helper(FILE *f)
{
  fflush(f);
  if (fileno(f) > 2)
    fclose(f);
}

static void __stdio_cleanup_proc(void)
{
  _fwalk(fcloseall_helper);
}

void (*__stdio_cleanup_hook)(void) = __stdio_cleanup_proc;

struct std_state {
  void (*__stdio_cleanup_hook)(void);
};

static struct std_state *sts;

static const struct std_state sinit =
{
  .__stdio_cleanup_hook = __stdio_cleanup_proc,
};

static void std_pre(void)
{
#define _SV(x) sts->x = x
  _SV(__stdio_cleanup_hook);
}
static void std_post(void)
{
#define _RS(x) x = sts->x
  _RS(__stdio_cleanup_hook);
}
DJ64_DEFINE_SWAPPABLE_CONTEXT2(std_state, sts, sinit,
    std_pre(), std_post());
