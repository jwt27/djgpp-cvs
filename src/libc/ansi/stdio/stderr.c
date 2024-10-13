/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <libc/file.h>
#include <libc/djctx.h>

FILE __dj_stderr;

static const FILE __dj_stderr_init = {
  0, 0, 0, 0,
  _IOWRT | _IONBF,
  2
};

struct ste_state {
  FILE __dj_stderr;
};

static struct ste_state *ste;

static const struct ste_state sinit =
{
  .__dj_stderr = __dj_stderr_init,
};

static void ste_pre(void)
{
#define _SV(x) ste->x = x
  _SV(__dj_stderr);
}
static void ste_post(void)
{
#define _RS(x) x = ste->x
  _RS(__dj_stderr);
}
DJ64_DEFINE_SWAPPABLE_CONTEXT2(ste_state, ste, sinit,
    ste_pre(), ste_post());
