#include <errno.h>
#include <io.h>
#include <stddef.h>
#include <termios.h>
#include <sys/exceptn.h>
#include <libc/bss.h>
#include <libc/ttyprvt.h>

#define _DEV_STDIN  0x0001
#define _DEV_STDOUT 0x0002
#define _DEV_NUL    0x0004
#define _DEV_CLOCK  0x0008
#define _DEV_RAW    0x0020
#define _DEV_CDEV   0x0080
#define _DEV_IOCTRL 0x4000

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
      if ((__libc_tty_p->t_lflag & ISIG) && ! (termiosp->c_iflag & IGNBRK)
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
