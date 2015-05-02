/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <errno.h>
#include <io.h>
#include <termios.h>
#include <unistd.h>
#include <libc/bss.h>
#include <libc/ttyprvt.h>
#include <libc/getdinfo.h>

int
tcflow (int handle, int action)
{
  short devmod;

  /* initialize */
  if (__libc_termios_hook_common_count != __bss_count)
    __libc_termios_init ();

  /* check handle whether valid or not */
  devmod = _get_dev_info (handle);
  if (devmod == -1)
    return -1;

  /* check console */
  if (! (devmod & _DEV_CDEV) || ! (devmod & (_DEV_STDIN|_DEV_STDOUT)))
    {
      errno = ENOTTY;
      return -1;
    }

  /* flow control */
  switch (action)
    {
    case TCOOFF:
    case TCOON:
      /* nothing */
      break;
    case TCIOFF:
    case TCION:
      {
	struct termios termiosbuf;
	unsigned char c;

	/* get the structure */
	if (tcgetattr (handle, &termiosbuf) == -1)
	  return -1;

	/* get flow character */
	c = termiosbuf.c_cc[(action == TCIOFF) ? VSTOP : VSTART];
	if (c != (unsigned char) _POSIX_VDISABLE)
	  {
	    /* now write on handle */
	    if (write (handle, &c, 1) < 0)
	      return -1;
	  }
      }
      break;
    default:
      errno = EINVAL;
      return -1;
    }

  return 0;
}
