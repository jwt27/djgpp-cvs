/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/*
 * tminit.c - initializer and main part of termios emulation.
 *   designed for DJGPP by Daisuke Aoyama <jack@st.rim.or.jp>
 *   special thanks to Ryo Shimizu
 */
/* ECMA-48 commands implemented by Mark E. <snowball3@softhome.net>.  */

#include <libc/stubs.h>
#include <go32.h>
#include <io.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <libc/file.h>
#include <libc/ttyprvt.h>
#include <libc/farptrgs.h>
#include <libc/getdinfo.h>

#define NPAR 16

struct screen_info
{
  unsigned char init_attrib;
  unsigned char attrib;
  int max_row;
  int max_col;
  unsigned char active_page;
  unsigned long video_buffer;
  unsigned int is_mono : 1;
};

/* Console command parser */

enum cmd_parser_states { need_esc = 0, have_esc, have_lbracket,
                         have_arg, have_cmd};

static enum cmd_parser_states cmd_state;

static struct screen_info screen;

/* static functions */
static ssize_t __libc_termios_write (int handle, const void *buffer, size_t count, ssize_t *rv);
static ssize_t __libc_termios_write_tty(int handle, const void *buffer, size_t count, int cooked);
static size_t parse_console_command(const unsigned char *buf, size_t count);
static void execute_console_command(const unsigned char cmd, unsigned char argc, unsigned int args[NPAR]);

/* direct I/O functions */
static inline void __direct_output (unsigned char ch);
static inline void __direct_outputs (const unsigned char *s);
static inline unsigned long get_video_offset(int col, int row);

/* Screen manipulation functions.  */
static void set_cursor(int col, int row);
static void move_cursor(int x_delta, int y_delta);
static void get_cursor(int *col, int *row);
static void clear_screen(int x1, int y1, int x2, int y2);
static void scroll_forward(int x1, int y1, int x2, int y2, int xdst, int ydst);
static void scroll_backward(int x1, int y1, int x2, int y2, int xdst, int ydst);

/* Initialize the screen portion of termios.  */
void __libc_termios_init_write(void)
{
  __dpmi_regs r;

  /* set special hooks */
  __libc_write_termios_hook = __libc_termios_write;

  /* Initialize the screen variable.  */
  _farsetsel(_dos_ds);
  screen.active_page = _farnspeekb(0x462);
  screen.max_row = (int)_farnspeekb(0x484);
  screen.max_col = (int)_farnspeekw(0x44a) - 1;

  /* Use the current mode to determine
     if the screen is in monochrome mode.  */
  screen.is_mono = (_farnspeekb(0x449) == 7);

  /* Determine the video offset.  */
  screen.video_buffer = screen.is_mono ? 0xb000 * 16 : 0xb800 * 16;

  r.h.ah = 0x08;
  r.h.bh = screen.active_page;
  __dpmi_int(0x10, &r);

  screen.init_attrib = r.h.ah;
  screen.attrib = r.h.ah;
}

/* Calculate the offset into the video buffer given the column and row.  */
static inline unsigned long
get_video_offset(int col, int row)
{
  return screen.video_buffer + row * (2 * (screen.max_col + 1)) + (col * 2);
}

/* Characters that can be a command.  */
static inline int
is_command_char(unsigned char ch)
{
  return isalpha(ch) || ch == '@' || ch == '`';
}

/******************************************************************************/
/* direct I/O function (inlined) **********************************************/

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
/* special write function *****************************************************/

static ssize_t
__libc_termios_write (int handle, const void *buffer, size_t count, ssize_t *rv)
{
  short devmod;
  int raw;

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
  if ((devmod & _DEV_CDEV) == 0 || (devmod & (_DEV_STDIN|_DEV_STDOUT)) == 0)
    return 0;

  /* Process output? */
  raw = ((devmod & _DEV_RAW) && (__file_handle_modes[handle] & O_BINARY))
        || ((__libc_tty_p->t_oflag & OPOST) == 0);

  *rv = __libc_termios_write_tty(handle, buffer, count, raw);
  return 1;
}

static ssize_t
__libc_termios_write_tty(int handle, const void *buffer, size_t count,
                         int raw)
{
  ssize_t bytes;
  const unsigned char *rp;
  unsigned char ch;
  ssize_t n;

  bytes = count;

  rp = buffer;
  n = count;
  while (n > 0)
  {
    /* get character */
    ch = *rp;

    /* Handle console commands */
    if (ch == '\e' || cmd_state != need_esc)
    {
      size_t delta;
      delta = parse_console_command(rp, n);

      /* Skip the characters parsed.  */
      n -= delta;
      rp += delta;
      continue;
    }

    ++rp;
    --n;

    /* produce spaces until the next TAB stop */
    if (ch == '\t')
    {
      int col;

      /* current column (cursor position) on the active page */
      col = _farpeekw(_dos_ds, 0x450 + screen.active_page) & 0xff;

      for (__direct_output(' '), col += 1; col % 8; col++)
      {
        if (col >= screen.max_col)
          col = -1;
        __direct_output(' ');
      }
      continue;
    }

    if (!raw)
    {
      /* NOTE: multibyte character don't contain control character */
      /* map NL to CRNL */
      if (ch == '\n' && (__libc_tty_p->t_oflag & ONLCR))
        __direct_output('\r');
      /* map CR to NL */
      else if (ch == '\r' && (__libc_tty_p->t_oflag & OCRNL))
        ch = '\n';
    }
    __direct_output(ch);
  }

  return bytes;
}

static size_t
parse_console_command(const unsigned char *buf, size_t count)
{
  static unsigned int args[NPAR];
  static unsigned char arg_count;
  static unsigned char ignore_cmd;

  const unsigned char *ptr = buf;
  const unsigned char *ptr_end = buf + count;

  while (ptr < ptr_end)
  {
    switch (cmd_state)
    {
      case need_esc:
        /* Find an escape or bail out.  */
        if (*ptr != '\e')
          return (ptr - buf);
        ++ptr;
        cmd_state = have_esc;
        if (ptr >= ptr_end)
          break;

      case have_esc:
        /* Find a left bracket or bail out.  */
        if (*ptr != '[')
          return (ptr - buf);

        cmd_state = have_lbracket;
        ++ptr;
        if (ptr == ptr_end)
          break;

      case have_lbracket:
        /* After the left bracket, either an argument
           or the command follows.  */
        arg_count = NPAR;
        arg_count = 0;
        args[0] = 0;
        if (isdigit(*ptr))
        {
          cmd_state = have_arg;
        }
        else if (*ptr == '[')
        {
          /* Ignore function keys. */
          ignore_cmd = 1;
          ++ptr;
          break;
        }
        else
        {
          cmd_state = have_cmd;
          break;
        }

      case have_arg:
      {
        /* Parse the argument.  Quit when there are no more digits.  */
        if (isdigit(*ptr))
        {
          do
          {
            args[arg_count] *= 10;
            args[arg_count] += *ptr - '0';
            ++ptr;
          } while (ptr < ptr_end && isdigit(*ptr));
          break;
        }
        else if (*ptr == ';')
        {
          /* Argument separator.  */
          ++arg_count;
          if (arg_count < NPAR)
            args[arg_count] = 0;
          ++ptr;
          break;
        }
        else
        {
          /* End of the current argument.
             Assume the command has been reached.  */
          cmd_state = have_cmd;
          ++arg_count;
        }
      }
      case have_cmd:
      {
        /* Execute the command.  */
        if (is_command_char(*ptr) && !ignore_cmd)
          execute_console_command(*ptr, arg_count, args);

        /* Reset the parse state.  */
        cmd_state = need_esc;
        ignore_cmd = 0;
        ++ptr;
        return ptr - buf;
      }
    }
  }
  return ptr - buf;
}

#define GET_ARG(I, DEFVAL) ((argc > I) ? (args[I]) : (DEFVAL))

static void
execute_console_command(const unsigned char cmd, unsigned char argc,
                        unsigned int args[NPAR])
{
  int row, col;

  switch (cmd)
  {
    /* Cursor Motion.  */

    /* CUU, VPB: Cursor up.  */
    case 'A':
    case 'k':
      move_cursor(0, -GET_ARG(0, 1));
      break;

    /* CUD, VPR: Cursor down.  */
    case 'B':
    case 'e':
      move_cursor(0, GET_ARG(0, 1));
      break;

    /* CUF, HPR: Cursor right.  */
    case 'C':
    case 'a':
      move_cursor(GET_ARG(0, 1), 0);
      break;

    /* CUB, HPB: Cursor Left.  */
    case 'D':
    case 'j':
      move_cursor(-GET_ARG(0, 1), 0);
      break;

    /* CNL: Cursor next line.  */
    case 'E':
      get_cursor(&col, &row);
      set_cursor(0, row + GET_ARG(0, 1));
      break;

    /* CPL: Cursor to previous line.  */
    case 'F':
      get_cursor(&col, &row);
      set_cursor(0, row - GET_ARG(0, 1));
      break;

    /* CHA: Cursor to column.  */
    case 'G':
      get_cursor(&col, &row);
      set_cursor(GET_ARG(0, 1) - 1, row);
      break;

    /* CUP, HVP: Cursor to row and column.  */
    case 'H':
    case 'f':
      set_cursor(GET_ARG(0, 1) - 1, GET_ARG(1, 1) - 1);
      break;

    /* VPA: Cursor to row.  */
    case 'd':
      get_cursor(&col, &row);
      set_cursor(col, GET_ARG(0, 1) - 1);
      break;

    /* CHT: Cursor to next tab stop.  */
    case 'I':
      get_cursor(&col, &row);
      set_cursor(col / 8 * 8 + GET_ARG(0, 1) * 8, row);
      break;

    /* CBT: Cursor to previous tab stop.  */
    case 'Z':
      get_cursor(&col, &row);
      if (col > 0 && GET_ARG(0, 1) > 0)
        set_cursor((col - 1) / 8 * 8 - (GET_ARG(0, 1) - 1) * 8, row);
      break;

    /* Edit commands.  */

    /* ED: Erase in Display.  */
    case 'J':
      switch (GET_ARG(0, 0))
      {
        /* Erase from cursor to end of screen.  */
        case 0:
          get_cursor(&col, &row);
          clear_screen(col, row, screen.max_col, screen.max_row);
          break;

        /* Erase from start of screen to cursor.  */
        case 1:
          get_cursor(&col, &row);
          clear_screen(0, 0, col, row);
          break;

        /* Erase the whole screen.  */
        case 2:
          clear_screen(0, 0, screen.max_col, screen.max_row);
          break;
      }

    /* EL: Edit in Line */
    case 'K':
      switch (GET_ARG(0, 0))
      {
        /* Clear from cursor to end of line.  */
        case 0:
          get_cursor(&col, &row);
          clear_screen(col, row, screen.max_col, row);
          break;

        /* Clear from start of line to cursor.  */
        case 1:
          get_cursor(&col, &row);
          clear_screen(0, row, col, row);
          break;

        /* Clear the whole line.  */
        case 2:
          get_cursor(&col, &row);
          clear_screen(0, row, screen.max_col, row);
          break;
      }

    /* IL: Insert Line */
    case 'L':
      get_cursor(&col, &row);
      scroll_forward(0, row, 0, screen.max_row, 0, row + GET_ARG(0, 1));
      break;

    /* DL: Delete Line */
    case 'M':
      get_cursor(&col, &row);
      scroll_backward(0, row, 0, screen.max_row, 0, row - GET_ARG(0, 1));
      break;

    /* SU: Scroll Up */
    case 'S':
      scroll_backward(0, GET_ARG(0, 1), 0, screen.max_row, 0, 0);
      break;

    /* ST: Scroll Down */
    case 'T':
      scroll_forward(0, 0, 0, screen.max_row, 0, GET_ARG(0, 1));
      break;

    /* ICH: Insert Character */
    case '@':
      get_cursor(&col, &row);
      scroll_forward(col, row, screen.max_col, screen.max_row,
                     col + GET_ARG(0, 1), row);
      break;

    /* DCH: Delete Character */
    case 'P':
      get_cursor(&col, &row);
      scroll_backward(col + GET_ARG(0, 1), row, screen.max_col, screen.max_row,
                      col, row);
      break;

    /* ECH: Erase Character */
    case 'X':
      get_cursor(&col, &row);
      clear_screen(col, row, col + GET_ARG(0, 1),  row);
      break;

    /* Unrecognized command. Do nothing.  */
    default:
      break;
  }
}

/* Get the coordinates of the cursor.  */
static void
get_cursor(int *col, int *row)
{
  unsigned short rowcol;

  rowcol = _farnspeekw(0x450 + screen.active_page);
  *row = (int)rowcol >> 8;
  *col = (int)(rowcol & 0xff);
}

/* Set the cursor to an absolute position.  */
static void
set_cursor(int col, int row)
{
  __dpmi_regs r;


  if (row < 0)
    row = 0;
  else if (row > screen.max_row)
    row = screen.max_row;

  if (col < 0)
    col = 0;
  else if (col > screen.max_col)
    col = screen.max_col;

  r.h.ah = 2;
  r.h.bh = screen.active_page;
  r.h.dh = row;
  r.h.dl = col;
  __dpmi_int(0x10, &r);
}

/* Move the cursor by a relative amount.  */
static void
move_cursor(int x_delta, int y_delta)
{
  int row, col;
  unsigned short rowcol;
  __dpmi_regs r;

  /* Get cursor's current position.  */
  rowcol = _farpeekw(_dos_ds, 0x450 + screen.active_page);
  row = rowcol >> 8;
  col = rowcol & 0xff;

  /* Sanity check for the row.  */
  if ((int)row + y_delta < 0)
    row = 0;
  else if (row + y_delta > screen.max_row)
    row = screen.max_row;

  /* Sanity check for the column.  */
  if ((int)col + x_delta < 0)
    col = 0;
  else if (col + x_delta > screen.max_col)
    col = screen.max_col;

  /* Do it.  */
  r.h.ah = 2;
  r.h.bh = screen.active_page;
  r.h.dh = row + y_delta;
  r.h.dl = col + x_delta;
  __dpmi_int(0x10, &r);
}

/* Clear from (x1, y1) to (x2, y2).  */
static void
clear_screen(int x1, int y1, int x2, int y2)
{
  unsigned long ptr, ptr_end;
  unsigned short fill;
  unsigned long fill_l;

  ptr = get_video_offset(x1, y1);
  ptr_end = get_video_offset(x2, y2);

  fill = ' ' | (screen.attrib << 8);
  fill_l = fill | (fill << 16);

  /* Align the upcoming 4-byte poke on a 4-byte boundary.  */
  _farsetsel(_dos_ds);
  if (ptr % 4 != 0)
  {
    _farnspokew(ptr, fill);
    ptr += 2;
  }

  /* Fill 4 bytes or 2 characters at a time.  */
  while (ptr < ptr_end)
  {
    _farnspokel(ptr, fill_l);
    ptr += 4;
  }
  /* Fill the last character by itself if
     the end isn't on a 4-byte boundary.  */
  if (ptr == ptr_end)
    _farnspokew(ptr, fill);
}

/* Move the area (x1, y1), (x2, y2) to
   (xdst, ydst), ((xdst + (x2 - x1)), (ydst + (y2 - y1))). */
static void
scroll_forward(int x1, int y1, int x2, int y2, int xdst, int ydst)
{
  unsigned short fill;
  unsigned long fill_l;
  unsigned long dst_ptr, src_ptr, dst_end, src_end;
  unsigned long screen_end;

  int xdst2, ydst2;

  xdst2 = xdst + (x2 - x1);
  ydst2 = ydst + (y2 - y1);

  /* Compute our beginning and ending offsets.  */
  dst_ptr = get_video_offset(xdst2, ydst2);
  src_ptr = get_video_offset(x2, y2);
  dst_end = get_video_offset(xdst, ydst);
  src_end = get_video_offset(x1, y1);
  screen_end = get_video_offset(screen.max_col, screen.max_row);

  /* Quit if the start of the source area is off the screen.  */
  if (src_end > screen_end)
    return;

  /* Clear area if all of the destination area is off the screen.  */
  if (dst_end > screen_end)
  {
    clear_screen(x1, y1, screen.max_col, screen.max_row);
    return;
  }

  /* Adjust pointers if part of destination area is past end of screen.  */
  if (dst_ptr > screen_end)
  {
    src_ptr -= (dst_ptr - screen_end);
    dst_ptr = screen_end;
  }

  fill = ' ' | (screen.attrib << 8);
  fill_l = fill | (fill << 16);

  /* Align the 4-byte copy at a 4-byte boundary if possible.  */
  _farsetsel(_dos_ds);
  if ((dst_ptr % 4) != 0 && (src_ptr % 4) != 0)
  {
    _farnspokew(dst_ptr, _farnspeekw(src_ptr));
    dst_ptr -= 2;
    src_ptr -= 2;
  }

  /* Copy 4-bytes at a time.  */
  while (dst_ptr >= dst_end)
  {
    _farnspokel(dst_ptr, _farnspeekl(src_ptr));
    dst_ptr -= 4;
    src_ptr -= 4;
  }

  /* If the last character isn't on a 4-byte boundary,
     copy it seperately.  */
  if (dst_end - dst_ptr == 2)
    _farnspokew(dst_end, _farnspeekw(src_end));

  /* Fill in the space vacated.  */
  dst_end -= 2;

  /* Special case for a one character fill.  */
  if (dst_end == src_end)
  {
    _farnspokew(dst_end, fill);
    return;
  }

  dst_end -= 2;

  /* Fill 4-bytes or 2 characters at a time.  */
  while (dst_end >= src_end)
  {
    _farnspokel(dst_end, fill_l);
     dst_end -= 4;
  }

  /* Special handling when last character isn't on a 4-byte boundary.  */
  if (src_end - dst_end == 2)
    _farnspokew(src_end, fill);
}

/* Move the area (x1, y1), (x2, y2) to
   (xdst, ydst), ((xdst + (x2 - x1)), (ydst + (y2 - y1))). */
static void
scroll_backward(int x1, int y1, int x2, int y2, int xdst, int ydst)
{
  unsigned short fill;
  unsigned long fill_l;
  unsigned long dst_ptr, src_ptr, dst_end, src_end;
  unsigned long screen_end;
  int xdst2, ydst2;

  xdst2 = xdst + (x2 - x1);
  ydst2 = ydst + (y2 - y1);

  /* Compute our beginning and ending offsets.  */
  dst_ptr = get_video_offset(xdst, ydst);
  src_ptr = get_video_offset(x1, y1);
  dst_end = get_video_offset(xdst2, ydst2);
  src_end = get_video_offset(x2, y2);
  screen_end = get_video_offset(screen.max_col, screen.max_row);

  /* Quit if the start of the source area is off the screen.  */
  if (src_ptr > screen_end)
    return;

  /* Quit if the end of the destination area is off the screen.  */
  if (dst_end < screen.video_buffer)
    return;

  /* Don't bother with copying outside the video buffer area.  */
  if (dst_ptr < screen.video_buffer)
  {
    src_ptr += (screen.video_buffer - dst_ptr);
    dst_ptr = screen.video_buffer;
  }

  /* On to business.  */
  fill = ' ' | (screen.attrib << 8);
  fill_l = fill | (fill << 16);

  /* Align the 4-byte copy at a 4-byte boundary if possible.  */
  _farsetsel(_dos_ds);
  if ((dst_ptr % 4) != 0 && (src_ptr % 4) != 0)
  {
    _farnspokew(dst_ptr, _farnspeekw(src_ptr));
    dst_ptr += 2;
    src_ptr += 2;
  }

  /* Copy 4-bytes at a time.  */
  while (dst_ptr <= dst_end)
  {
    _farnspokel(dst_ptr, _farnspeekl(src_ptr));
    dst_ptr += 4;
    src_ptr += 4;
  }

  /* If the last character isn't on a 4-byte boundary,
     copy it seperately.  */
  if (dst_ptr - dst_end == 2)
    _farnspokew(dst_end, _farnspeekw(src_end));

  dst_end += 2;

  /* Special case for a one character fill.  */
  if (dst_end == src_end)
  {
    _farnspokew(dst_end, fill);
    return;
  }

  dst_end += 2;

  /* Fill 4-bytes or 2 characters at a time.  */
  while (dst_end <= src_end)
  {
    _farnspokel(dst_end, fill_l);
     dst_end += 4;
  }

  /* Special handling when last character isn't on a 4-byte boundary.  */
  if (dst_end - src_end == 2)
    _farnspokew(src_end, fill);
}

