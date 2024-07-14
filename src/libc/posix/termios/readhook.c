/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
#include <libc/ttyprvt.h>
#include <libc/djctx.h>

ssize_t (*__libc_read_termios_hook)(int handle, void *buffer, size_t count,
				    ssize_t *rv) = NULL;

struct tmr_state {
  struct tmr_state *prev;
  ssize_t (*__libc_read_termios_hook)(int handle, void *buffer, size_t count,
				      ssize_t *rv);
};

static struct tmr_state *sts;

static void tmr_init(void)
{
  __libc_read_termios_hook = NULL;
}
static void tmr_pre(void)
{
#define _SV(x) sts->x = x
  _SV(__libc_read_termios_hook);
}
static void tmr_post(void)
{
#define _RS(x) x = sts->x
  _RS(__libc_read_termios_hook);
}
DJ64_DEFINE_SWAPPABLE_CONTEXT2(tmr_state, sts,
    tmr_init(), tmr_pre(), tmr_post());
