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
#include <fcntl.h>
#include <go32.h>
#include <pc.h>
#include <io.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/exceptn.h>
#include <libc/bss.h>
#include <libc/file.h>
#include <libc/dosio.h>
#include <libc/ttyprvt.h>
#include <libc/farptrgs.h>
#include <libc/getdinfo.h>

#define CPMEOF 0x1a /* Ctrl+Z */

#define SENSE_NO_KEY	0
#define SENSE_REG_KEY	1
#define SENSE_EXT_KEY	2

/* tty buffers */
unsigned char __libc_tty_queue_buffer[_TTY_QUEUE_SIZE];
struct tty __libc_tty_internal = TTYDEFAULT;
struct tty *__libc_tty_p = &__libc_tty_internal;
struct tty_editline __libc_tty_editline = { 0, { 0 }, { 0 }, };

/* global only in the termios functions */
int __libc_termios_hook_common_count = -1;

/* static data */
static unsigned ah_key_sense;
static unsigned ah_key_get;
static unsigned ah_ctrl_sense;

static const unsigned char *ext_key_string;

/* static functions */
static void __libc_termios_fflushall (void);
static ssize_t __libc_termios_read (int handle, void *buffer, size_t count, ssize_t *rv);
static ssize_t __libc_termios_read_cooked_tty (int handle, void *buffer, size_t count);
static ssize_t __libc_termios_read_raw_tty (int handle, void *buffer, size_t count);
static ssize_t __libc_termios_write (int handle, const void *buffer, size_t count, ssize_t *rv);
static ssize_t __libc_termios_write_cooked_tty (int handle, const void *buffer, size_t count);
static ssize_t __libc_termios_write_raw_tty (int handle, const void *buffer, size_t count);
static void __libc_termios_echo_ctrl (unsigned char ch);
static void __libc_termios_maybe_echo_ctrl (unsigned char ch);
static void __libc_termios_maybe_echo (unsigned char ch);
static void __libc_termios_maybe_erase1 (void);
static void __libc_termios_erase_editline (void);
static void __libc_termios_kill_editline (void);
static void __libc_termios_insert_editline (unsigned char ch);
static void __libc_termios_clear_queue (void);
static int __libc_termios_get_queue (void);
static int __libc_termios_put_queue (unsigned char ch);
static void __libc_termios_fill_queue (void);

/* direct I/O functions */
static inline int __direct_keysense (void);
static inline unsigned char __direct_keyinput (void);
static inline int __direct_ctrlsense (void);
static inline void __direct_output (unsigned char ch);
static inline void __direct_outputs (const unsigned char *s);

/******************************************************************************/
/* initialize function ********************************************************/

static void
__libc_termios_fflushall (void)
{
#if 0 /* don't work on djgpp */
  fflush (NULL);
#else
  _fwalk ((void (*) (FILE*)) fflush);
#endif
}

void
__libc_termios_init (void)
{
  if (__libc_termios_hook_common_count != __bss_count)
    {
      __libc_termios_hook_common_count = __bss_count;

      /* flush all buffered streams */
      __libc_termios_fflushall ();

      if (_farpeekb (_dos_ds, 0x496) & 0x10)
      {
        ah_key_get = 0x10;
        ah_key_sense = 0x11;
        ah_ctrl_sense = 0x12;
      }
      else
      {
        ah_key_get = 0x00;
        ah_key_sense = 0x01;
        ah_ctrl_sense = 0x02;
      }

      /* set special hooks */
      __libc_read_termios_hook = __libc_termios_read;
      __libc_write_termios_hook = __libc_termios_write;

      /* import parameters */
      /* __libc_tty_p = ...; */
    }
}

/******************************************************************************/
/* direct I/O function (inlined) **********************************************/

#define _REG_STATUS_ZF 0x40

static inline int
__direct_keysense(void)
{
  __dpmi_regs r;
  char is_ext_key;

  r.h.ah = ah_key_sense;
  __dpmi_int(0x16, &r);
  if (r.x.flags & _REG_STATUS_ZF)
    return SENSE_NO_KEY;

  is_ext_key = (r.h.al == 0x00 || r.h.al == 0xe0);
  return is_ext_key ? SENSE_EXT_KEY : SENSE_REG_KEY;
}

static inline unsigned char
__direct_keyinput (void)
{
  __dpmi_regs r;

  r.h.ah = ah_key_get;
  __dpmi_int (0x16, &r);

  return r.h.al;
}

/* Get an extended key and return its encoding.  */
static inline
const unsigned char *
set_ext_key_string(void)
{
  __dpmi_regs r;

  r.h.ah = ah_key_get;
  __dpmi_int(0x16, &r);

  return (ext_key_string = __get_extended_key_string((int)r.h.ah));
}

#define _KEY_INS  0x80
#define _KEY_CAPS 0x40
#define _KEY_NUM  0x20
#define _KEY_SCRL 0x10
#define _KEY_ALT  0x08
#define _KEY_CTRL 0x04
#define _KEY_LSFT 0x02
#define _KEY_RSFT 0x01

static inline int
__direct_ctrlsense (void)
{
  __dpmi_regs r;

  r.h.ah = 0x02;
  __dpmi_int (0x16, &r);
  if (r.h.al & _KEY_CTRL)
    return 1;

  return 0;
}

static inline void
__direct_output (unsigned char ch)
{
  __dpmi_regs r;

  if (ch == 0xff)
    return;

  r.h.al = ch;
  __dpmi_int (0x29, &r);
}

static inline void
__direct_outputs (const unsigned char *s)
{
  while (*s)
    __direct_output (*s++);
}

/******************************************************************************/
/* special read function ******************************************************/

static ssize_t
__libc_termios_read (int handle, void *buffer, size_t count, ssize_t *rv)
{
  short devmod;

  /* check handle whether valid or not */
  devmod = _get_dev_info (handle);
  if (devmod == -1)
    {
      *rv = -1;
      return 1;
    }

  /* special case */
  if (count == 0)
    {
      *rv = 0;
      return 1;
    }

  /* console only... */
  if ((devmod & _DEV_CDEV) && (devmod & (_DEV_STDIN|_DEV_STDOUT)))
    {
      /* character device */
      if ((devmod & _DEV_RAW) && (__file_handle_modes[handle] & O_BINARY))
	*rv = __libc_termios_read_raw_tty (handle, buffer, count);
      else
	*rv = __libc_termios_read_cooked_tty (handle, buffer, count);
      return 1;
    }

  return 0;
}

static ssize_t
__libc_termios_read_cooked_tty (int handle, void *buffer, size_t count)
{
  unsigned char *wp;
  ssize_t n;

  wp = buffer;
  n = count;

#if 0
  /* clear cooked queue */
  if (__libc_termios_exist_queue ())
    __libc_termios_clear_queue ();
#endif

  if (__libc_tty_p->t_lflag & ICANON)
    {
      /* get inputs (wait for NL or EOT) */
      if (! __libc_termios_exist_queue ())
	__libc_termios_fill_queue ();

      while (--n >= 0)
	{
	  if (! __libc_termios_exist_queue ())
	    break;

	  *wp++ = __libc_termios_get_queue ();
	}
    }
  else
    {
      /* block until getting inputs */
      while (! __libc_termios_exist_queue ())
	{
	  __dpmi_yield ();
	  __libc_termios_fill_queue ();
	}

      while (--n >= 0)
	{
	  *wp++ = __libc_termios_get_queue ();

	  if (! __libc_termios_exist_queue ())
	    {
	      __libc_termios_fill_queue ();
	      if (! __libc_termios_exist_queue ())
		break;
	    }
	}
    }

  return (ssize_t) (wp - (unsigned char *) buffer);
}

static ssize_t
__libc_termios_read_raw_tty (int handle, void *buffer, size_t count)
{
  unsigned char *wp;
  unsigned char ch;
  ssize_t n;
  int sense;

  n = count;
  wp = buffer;

  /* clear cooked queue */
  if (__libc_termios_exist_queue ())
    __libc_termios_clear_queue ();

  /* block until getting inputs */
  while (ext_key_string == NULL && __direct_keysense() == SENSE_NO_KEY)
    __dpmi_yield ();

  while (--n >= 0)
    {
      /* exhaust inputs ? */
      if (ext_key_string == NULL
          && (sense = __direct_keysense()) == SENSE_NO_KEY)
	break;

      if (ext_key_string)
      {
        ch = *ext_key_string;
        ++ext_key_string;
        if (*ext_key_string == '\0')
          ext_key_string = NULL;
      }
      else if (sense == SENSE_REG_KEY)
      {
        ch = __direct_keyinput();

        /* replace CTRL+SPACE with 0x00 */
        if (ch == ' ' && __direct_ctrlsense ())
	  ch = '\0';
      }
      else
      {
        if (set_ext_key_string() == NULL)
        {
          /* This extended key has no encoding.  If there are no keys
             already stored in the buffer, wait for another key to ensure
             at least one character is put into the buffer.  */
          ++n;
          if (wp == (unsigned char *)buffer)
          {
            while (__direct_keysense() == SENSE_NO_KEY)
              __dpmi_yield();
          }
          continue;
        }
        ch = *ext_key_string;
        ++ext_key_string;
      }

      /* copy a character into buffer and echo */
      *wp++ = ch;
      __libc_termios_maybe_echo (ch);
    }

  return (ssize_t) (wp - (unsigned char *) buffer);
}

/******************************************************************************/
/* special write function *****************************************************/

static ssize_t
__libc_termios_write (int handle, const void *buffer, size_t count, ssize_t *rv)
{
  short devmod;

  /* check handle whether valid or not */
  devmod = _get_dev_info (handle);
  if (devmod == -1)
    {
      *rv = -1;
      return 1;
    }

  /* special case */
  if (count == 0)
    {
      *rv = 0;
      return 1;
    }

  /* console only... */
  if ((devmod & _DEV_CDEV) && (devmod & (_DEV_STDIN|_DEV_STDOUT)))
    {
      /* character device */
      if ((devmod & _DEV_RAW) && (__file_handle_modes[handle] & O_BINARY))
	*rv = __libc_termios_write_raw_tty (handle, buffer, count);
      else
	*rv = __libc_termios_write_cooked_tty (handle, buffer, count);
      return 1;
    }

  return 0;
}

static ssize_t
__libc_termios_write_cooked_tty (int handle, const void *buffer, size_t count)
{
  ssize_t bytes;

  /* output process if need */
  if (__libc_tty_p->t_oflag & OPOST)
    {
      const unsigned char *rp;
      unsigned char ch;
      ssize_t n;

      rp = buffer;
      n = count;
      while (--n >= 0)
	{
	  /* get character */
	  ch = *rp++;

	  /* NOTE: multibyte character don't contain control character */
	  /* map NL to CRNL */
	  if (ch == '\n' && (__libc_tty_p->t_oflag & ONLCR))
	    __direct_output ('\r');
	  /* map CR to NL */
	  else if (ch == '\r' && (__libc_tty_p->t_oflag & OCRNL))
	    ch = '\n';
	  /* produce spaces until the next TAB stop */
	  else if (ch == '\t')
	    {
	      int col, max_col;

	      _farsetsel (_dos_ds);
	      /* current column (cursor position) on the active page */
	      col = _farnspeekw (0x450 + _farnspeekb (0x462)) & 0xff;
	      /* the number of displayed character columns */
	      max_col = _farnspeekw (0x44a);

	      for (__direct_output (' '), col += 1; col % 8; col++)
		{
		  if (col >= max_col - 1)
		    col = -1;
		  __direct_output (' ');
		}
	      continue;
	    }

	  __direct_output (ch);
	}

      /* don't count CR */
      bytes = count;
    }
  else
    {
      /* output with no effect */
      bytes = __libc_termios_write_raw_tty (handle, buffer, count);
    }

  return bytes;
}

static ssize_t
__libc_termios_write_raw_tty (int handle, const void *buffer, size_t count)
{
  const unsigned char *rp;
  ssize_t n;

  rp = buffer;
  n = count;
  while (--n >= 0)
    __direct_output (*rp++);

  return count;
}

/******************************************************************************/
/* echo routines **************************************************************/

static void
__libc_termios_echo_ctrl (unsigned char ch)
{
  ch ^= 0x40;
  __direct_output ('^');
  __direct_output (ch);
}

static void
__libc_termios_maybe_echo_ctrl (unsigned char ch)
{
  if (__libc_tty_p->t_lflag & ECHOCTL)
    __libc_termios_echo_ctrl (ch);
}

static void
__libc_termios_maybe_echo (unsigned char ch)
{
  if (! (__libc_tty_p->t_lflag & ECHO))
    {
      /* don't echo but newline is required ? */
      if (ch == '\n' && (__libc_tty_p->t_lflag & ECHONL))
	{
	  if (__libc_tty_p->t_oflag & ONLCR)
	    __direct_output ('\r');
	  __direct_output ('\n');
	}
      return;
    }

  /* check literal flag */
  if ((__libc_tty_p->t_status & _TS_LNCH) && (ch < 0x20))
    {
      __libc_termios_echo_ctrl (ch);
      return;
    }

  /* output process if need */
  if (__libc_tty_p->t_oflag & OPOST)
    {
      /* map NL to CRNL */
      if (ch == '\n' && (__libc_tty_p->t_oflag & ONLCR))
	{
	  __direct_output ('\r');
	  __direct_output ('\n');
	  return;
	}
      /* map CR to NL */
      else if (ch == '\r' && (__libc_tty_p->t_oflag & OCRNL))
	{
	  __direct_output ('\n');
	  return;
	}
      /* discard EOT */
      else if (_CC_EQU (VEOF, ch) && (__libc_tty_p->t_oflag & ONOEOT))
	{
	  return;
	}
    }

  /* control character except newline */
  if ((ch < 0x20 || ch == 0x7f) && (__libc_tty_p->t_lflag & ECHOCTL) && ch != '\n')
    {
      __libc_termios_echo_ctrl (ch);
      return;
    }

  /* no effect */
  __direct_output (ch);
}

/******************************************************************************/
/* edit routines **************************************************************/

static void
__libc_termios_maybe_erase1 (void)
{
  if (__libc_tty_p->t_lflag & ECHO)
    {
      /* eat prev. character by BS SPC BS */
      __direct_output ('\010');
      if (__libc_tty_p->t_lflag & ECHOE)
	{
	  __direct_output (' ');
	  __direct_output ('\010');
	}
    }
}

/* erase one prev. character of editline */
static void
__libc_termios_erase_editline (void)
{
  int col;
  char pf;

  col = __libc_tty_editline.col;
  if (col == 0) /* no more */
    return;

  /* get position flag */
  pf = __libc_tty_editline.flag[col - 1];
  if (pf == _TTY_EDITLINE_INVALID || pf < 0)
    {
      /* erase all invalid character */
      while (col > 0 && __libc_tty_editline.flag[col - 1] < 0)
	{
	  __libc_termios_maybe_erase1 ();
	  --col;
	}
    }
  else if (pf == _TTY_EDITLINE_CTRL)
    {
      if (__libc_tty_p->t_lflag & ECHOCTL)
	{
	  /* erase one of "^X" */
	  __libc_termios_maybe_erase1 ();
	  --col;
	}
      __libc_termios_maybe_erase1 ();
      --col;
    }
  else
    {
      /* erase single or multibyte charcter */
      while (--pf >= 0)
	{
	  __libc_termios_maybe_erase1 ();
	  --col;
	}
    }

  /* set next insert position */
  __libc_tty_editline.col = col;
}

/* erase all editline */
static void
__libc_termios_kill_editline (void)
{
  if (__libc_tty_p->t_lflag & ECHOKE)
    {
      while (__libc_tty_editline.col != 0)
	__libc_termios_erase_editline ();
    }
  else
    __libc_tty_editline.col = 0;

  if (__libc_tty_p->t_lflag & ECHOK)
    __libc_termios_maybe_echo ('\n');
}

static void
__libc_termios_insert_editline (unsigned char ch)
{
  int mbsize;
  int col;

  col = __libc_tty_editline.col;
  if (col >= _TTY_EDITLINE_SIZE)
    {
      /* detected over flow */
      if (__libc_tty_p->t_iflag & IMAXBEL)
	__direct_output ('\007');
      return;
    }

  __libc_tty_editline.buf[col] = ch;
  if (col == 0 || ((col > 0)
		   && __libc_tty_editline.flag[col - 1] != _TTY_EDITLINE_INVALID))
    {
      /* check multibyte length */
      mbsize = mblen (__libc_tty_editline.buf + col, 1);
      if (mbsize == 1)
	{
	  /* single character */
	  if ((ch < 0x20) || ch == 0x7f)
	    __libc_tty_editline.flag[col] = _TTY_EDITLINE_CTRL;
	  else
	    __libc_tty_editline.flag[col] = _TTY_EDITLINE_SINGLE;
	  __libc_termios_maybe_echo (ch);
	}
      else
	{
	  /* first of multibyte or invalid character */
	  __libc_tty_editline.flag[col] = _TTY_EDITLINE_INVALID;
	  __libc_tty_p->t_status |= _TS_LNCH;
	}
    }
  else
    {
      int pcol;

      /* look for non fixed flag */
      pcol = col;
      while (__libc_tty_editline.flag[pcol - 1] == _TTY_EDITLINE_INVALID)
	if (--pcol <= 0)
	  break;

      /* check whether it's multibyte sequence */
      mbsize = mblen (__libc_tty_editline.buf + pcol, (col - pcol + 1));
      if (mbsize > 1)
	{
	  /* multibyte sequence is good */
	  while (pcol < col)
	    {
	      /* set negative size for dividing sequence */
	      __libc_tty_editline.flag[pcol] = -mbsize;
	      __libc_termios_maybe_echo (__libc_tty_editline.buf[pcol]);
	      pcol++;
	    }
	  /* last flag is always positive size */
	  __libc_tty_editline.flag[col] = mbsize;
	  __libc_termios_maybe_echo (ch);
	}
      else
	{
	  if ((col - pcol + 1) > MB_CUR_MAX)
	    {
	      /* it's terrible... */
	      while (pcol <= col)
		{
		  /* replace all with valid character */
		  __libc_tty_editline.flag[pcol] = _TTY_EDITLINE_SINGLE;
		  __libc_tty_editline.buf[pcol] = 'X';
		  __libc_termios_maybe_echo ('X');
		  pcol++;
		}
	    }

	  /* continuous multibyte character */
	  __libc_tty_editline.flag[col] = _TTY_EDITLINE_INVALID;
	  __libc_tty_p->t_status |= _TS_LNCH;
	}
    }

  /* set next insert position */
  col++;
  __libc_tty_editline.col = col;
}

/******************************************************************************/
/* queued input routines ******************************************************/

int
__libc_termios_exist_queue (void)
{
  return __libc_tty_p->t_count;
}

static void
__libc_termios_clear_queue (void)
{
  __libc_tty_p->t_count = 0;
  __libc_tty_p->t_rpos = __libc_tty_p->t_top;
  __libc_tty_p->t_wpos = __libc_tty_p->t_top;
}

static int
__libc_termios_get_queue (void)
{
  int ch;

  if (__libc_tty_p->t_count == 0)
    return -1;

  ch = *__libc_tty_p->t_rpos++;
  __libc_tty_p->t_count--;

  if (__libc_tty_p->t_rpos >= __libc_tty_p->t_bottom)
    __libc_tty_p->t_rpos = __libc_tty_p->t_top;

  return ch;
}

static int
__libc_termios_put_queue (unsigned char ch)
{
  if (__libc_tty_p->t_count >= __libc_tty_p->t_size)
    return -1;

  *__libc_tty_p->t_wpos++ = ch;
  __libc_tty_p->t_count++;

  if (__libc_tty_p->t_wpos >= __libc_tty_p->t_bottom)
    __libc_tty_p->t_wpos = __libc_tty_p->t_top;

  return 0;
}

static void
__libc_termios_fill_queue (void)
{
  unsigned char ch;
  int sense;

  while (1)
    {
      /* exhaust inputs ? */
      if (ext_key_string == NULL
          && (sense = __direct_keysense()) == SENSE_NO_KEY)
	{
	  if (__libc_tty_p->t_lflag & ICANON)
	    {
	      /* wait for NL or EOT */
	      __dpmi_yield ();
	      continue;
	    }
	  return;
	}

      /* really get */
      if (ext_key_string)
      {
        ch = *ext_key_string;
        ++ext_key_string;
        if (*ext_key_string == '\0')
          ext_key_string = NULL;
      }
      else  if (sense == SENSE_REG_KEY)
      {
        ch = __direct_keyinput();

        /* replace CTRL+SPACE with 0x00 */
        if (ch == ' ' && __direct_ctrlsense())
	      ch = '\0';
      }
      else
      {
        if (set_ext_key_string() == NULL)
          continue;
        ch = *ext_key_string;
        ++ext_key_string;
      }

      /* input process if need */
      if (! (__libc_tty_p->t_status & _TS_LNCH) || ch != (unsigned char) _POSIX_VDISABLE)
	{
	  /* software signals */
	  if (__libc_tty_p->t_lflag & ISIG)
	    {
	      if (! (__libc_tty_p->t_iflag & IGNBRK) && _CC_EQU (VINTR, ch))
		{
		  /* Only raise the signal if SIGINT generation by the
		     INTR key is disabled; otherwise the signal was
		     already raised and what we see here is the key
		     which generated it that wasn't removed from the
		     keyboard buffer.  */
		  if (__libc_tty_p->t_iflag & BRKINT)
		    {
		      if (__djgpp_set_ctrl_c (-1) == 0)
			{
			  __libc_termios_maybe_echo_ctrl (ch);
			  kill (getpid (), SIGINT);
			}
		      continue;
		    }
		  else
		    {
		      ch = '\0';
		      goto proc_skip;
		    }
		}
	      /* See the commentary about SIGINT above.  */
	      else if (_CC_EQU (VQUIT, ch))
		{
		  if (__djgpp_set_ctrl_c (-1) == 0)
		    {
		      __libc_termios_maybe_echo_ctrl (ch);
		      kill (getpid(), SIGQUIT);
		    }
		  continue;
		}
	      else if (_CC_EQU (VSUSP, ch))
		{
		  __libc_termios_maybe_echo_ctrl (ch);
#ifdef SIGTSTP
		  kill (getpid(), SIGTSTP);
#else /* djgpp don't have ... */
		  {
		    char oldcwd[PATH_MAX];
		    int fds[5] = { -1, -1, -1, -1, -1 };
		    int i;

		    for (i = 0; i < 5; i++)
		      if ((fds[i] = fcntl (i, F_DUPFD, 20)) < 0)
		        __direct_outputs ("Suspend: cannot save fds\r\n");

		    __direct_outputs ("\r\nSuspended\r\n");
		    /* keep cwd on exec */
		    getcwd (oldcwd, sizeof (oldcwd));
		    system ("");
		    chdir (oldcwd);

		    for (i = 0; i < 5; i++)
		      if (fds[i] >= 0)
			{
			  dup2 (fds[i], i);
			  close (fds[i]);
			}
		  }
#endif /* !SIGTSTP */
		  continue;
		}
	    }

	  /* flow control... */
	  if (_CC_EQU (VSTART, ch) && (__libc_tty_p->t_iflag & IXOFF))
	    {
	      continue;
	    }
	  else if (_CC_EQU (VSTOP, ch) && (__libc_tty_p->t_iflag & IXOFF))
	    {
	      continue;
	    }

	  /* CR/LF process */
	  if (ch == '\r')
	    {
	      if (__libc_tty_p->t_iflag & IGNCR)
		continue;
	      if (__libc_tty_p->t_iflag & ICRNL)
		ch = '\n';
	    }
	  else if ((ch == '\n') && (__libc_tty_p->t_iflag & INLCR))
	    ch = '\r';

	  /* strip 8th-bit */
	  if (__libc_tty_p->t_iflag & ISTRIP)
	    ch &= 0x7f;
	}

proc_skip:
      if (__libc_tty_p->t_lflag & ICANON)
	{
	  if (__libc_tty_p->t_status & _TS_LNCH)
	    {
	      __libc_tty_p->t_status &= ~_TS_LNCH;
	      __libc_termios_insert_editline (ch);
	    }
	  else
	    {
	      if (_CC_EQU (VERASE, ch))
		__libc_termios_erase_editline ();
	      else if (_CC_EQU (VKILL, ch))
		__libc_termios_kill_editline ();
	      else if (ch == '\n' || _CC_EQU (VEOF, ch) || _CC_EQU (VEOL, ch))
		{
		  int col = __libc_tty_editline.col;
		  unsigned char *p = __libc_tty_editline.buf;

		  /* clear column for next access */
		  __libc_tty_editline.col = 0;

		  /* copy editline into tty queue */
		  while (--col >= 0)
		    __libc_termios_put_queue (*p++);

		  /* echo terminate character and put it */
		  __libc_termios_maybe_echo (ch);
		  if (_CC_NEQU (VEOF, ch))
		    __libc_termios_put_queue (ch);

		  return;
		}
	      else
		__libc_termios_insert_editline (ch);
	    } /* !_TS_LNCH */
	}
      else /* !ICANON */
	{
	  __libc_termios_maybe_echo (ch);
	  __libc_tty_p->t_status &= ~_TS_LNCH;
	  __libc_termios_put_queue (ch);
	}
    } /* end of while (1) */
}

/******************************************************************************/

