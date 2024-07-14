/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <libc/file.h>
#include <libc/local.h>

static void fcloseall_helper(FILE *f)
{
  fflush(f);
  if (fileno(f) > 2)
    fclose(f);
}

void __stdio_cleanup_proc(void);
void __stdio_cleanup_proc(void)
{
  _fwalk(fcloseall_helper);
}

void (*__stdio_cleanup_hook)(void) = __stdio_cleanup_proc;

struct std_state {
  struct std_state *prev;
  void (*__stdio_cleanup_hook)(void);
};

static struct std_state *sts;

static void std_init(void)
{
  __stdio_cleanup_hook = __stdio_cleanup_proc;
}
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
DJ64_DEFINE_SWAPPABLE_CONTEXT2(std_state, sts,
    std_init(), std_pre(), std_post());
