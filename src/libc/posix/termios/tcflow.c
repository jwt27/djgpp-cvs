#include <libc/stubs.h>
#include <errno.h>
#include <io.h>
#include <termios.h>
#include <unistd.h>
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
