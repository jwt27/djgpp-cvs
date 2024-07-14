/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/*
 * tminit.c - initializer and main part of termios emulation.
 *   designed for DJGPP by Daisuke Aoyama <jack@st.rim.or.jp>
 *   special thanks to Ryo Shimizu
 */

#include <libc/stubs.h>
#include <stdio.h>
#include <termios.h>
#include <libc/bss.h>
#include <libc/file.h>
#include <libc/ttyprvt.h>
#include <libc/djctx.h>

/* tty buffers */
struct tty __libc_tty_internal = TTYDEFAULT;
struct tty *__libc_tty_p = &__libc_tty_internal;

/* global only in the termios functions */
int __libc_termios_hook_common_count = -1;

struct tm_state {
  struct tm_state *prev;
  struct tty __libc_tty_internal;
  struct tty *__libc_tty_p;
  int __libc_termios_hook_common_count;
};

static struct tm_state *tms;

static struct tm_state tms_init =
{
  .__libc_tty_internal = (struct tty)TTYDEFAULT,
  .__libc_tty_p = &__libc_tty_internal,
  .__libc_termios_hook_common_count = -1,
};

static void tms_pre(void)
{
#define _SV(x) tms->x = x
  _SV(__libc_tty_internal);
  _SV(__libc_tty_p);
  _SV(__libc_termios_hook_common_count);
}
static void tms_post(void)
{
#define _RS(x) x = tms->x
  _RS(__libc_tty_internal);
  _RS(__libc_tty_p);
  _RS(__libc_termios_hook_common_count);
}
DJ64_DEFINE_SWAPPABLE_CONTEXT2(tm_state, tms, tms_init,
    tms_pre(), tms_post());

/* static functions */
static void __libc_termios_fflushall(void);

/******************************************************************************/
/* initialize function ********************************************************/

static void
__libc_termios_fflushall(void)
{
  fflush(NULL);
}

void
__libc_termios_init(void)
{
  if (__libc_termios_hook_common_count != __bss_count)
  {
    __libc_termios_hook_common_count = __bss_count;

    /* flush all buffered streams */
    __libc_termios_fflushall ();

    __libc_termios_init_read();
    __libc_termios_init_write();

    /* import parameters */
    /* __libc_tty_p = ...; */
  }
}
