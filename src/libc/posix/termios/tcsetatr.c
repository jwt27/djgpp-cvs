/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include <io.h>
#include <stddef.h>
#include <termios.h>
#include <sys/exceptn.h>
#include <libc/bss.h>
#include <libc/ttyprvt.h>
#include <libc/getdinfo.h>

int
tcsetattr (int handle, int action, const struct termios *termiosp)
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

  /* check arguments */
  if (termiosp == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  /* set the structure */
  switch (action)
    {
    case TCSANOW:
    case TCSAFLUSH:
    case TCSADRAIN:
      /* enable or disable ^C */
      if ((termiosp->c_lflag & ISIG) && ! (termiosp->c_iflag & IGNBRK)
	  && (termiosp->c_iflag & BRKINT) && (termiosp->c_cc[VINTR] == 0x03))
	__djgpp_set_ctrl_c (1);
      else
	__djgpp_set_ctrl_c (0);

      /* copy the structure */
      __libc_tty_p->t_termios = *termiosp;
      break;
    default:
      errno = EINVAL;
      return -1;
    }

  return 0;
}
