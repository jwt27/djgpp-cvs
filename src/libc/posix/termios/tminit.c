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

/* tty buffers */
struct tty __libc_tty_internal = TTYDEFAULT;
struct tty *__libc_tty_p = &__libc_tty_internal;

/* global only in the termios functions */
int __libc_termios_hook_common_count = -1;


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
