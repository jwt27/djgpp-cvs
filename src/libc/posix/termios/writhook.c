/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
#include <libc/ttyprvt.h>
#include <libc/djctx.h>

ssize_t (*__libc_write_termios_hook)(int handle, const void *buffer,
				     size_t count, ssize_t *rv) = NULL;

struct tmw_state {
  ssize_t (*__libc_write_termios_hook)(int handle, const void *buffer,
				       size_t count, ssize_t *rv);
};

static struct tmw_state *sts;

static void tmw_pre(void)
{
#define _SV(x) sts->x = x
  _SV(__libc_write_termios_hook);
}
static void tmw_post(void)
{
#define _RS(x) x = sts->x
  _RS(__libc_write_termios_hook);
}
DJ64_DEFINE_SWAPPABLE_CONTEXT2(tmw_state, sts, ((struct tmw_state){}),
    tmw_pre(), tmw_post());
