/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/*
 * tminit.c - initializer and main part of termios emulation.
 *   designed for DJGPP by Daisuke Aoyama <jack@st.rim.or.jp>
 *   special thanks to Ryo Shimizu
 */
/* ECMA-48 commands implemented by Mark E. <snowball3@softhome.net>
   except color support ported from GNU ls color support
   by Eli Zaretskii.  */

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

/* Information about the screen.  */
struct screen_info
{
  unsigned char init_attrib;
  unsigned char attrib;
  int max_row;
  int max_col;
  unsigned char active_page;
  unsigned long video_buffer;
  unsigned int is_mono : 1;
  unsigned int norm_blink : 1;
  unsigned int cur_blink : 1;
};


/* Pointers to functions that modify the screen.  */
struct termios_screen_driver
{
  void (*write_ch)(unsigned char ch, int *x, int *y);
  void (*scroll_up)(int y1, int y2, int delta);
  void (*scroll_down)(int y1, int y2, int delta);
  void (*scroll_forward)(int x1, int y1, int x2, int y2, int xdst, int ydst);
  void (*scroll_backward)(int x1, int y1, int x2, int y2, int xdst, int ydst);
  void (*clear)(int x1, int y1, int x2, int y2);
};


/* Console command parser */

enum cmd_parser_states { need_esc = 0, have_esc, have_lbracket,
                         have_arg, have_cmd};

/* Color values. From conio.h.  */
enum COLORS {
    /*  dark colors     */
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    LIGHTGRAY,
    /*  light colors    */
    DARKGRAY, /* "light black" */
    LIGHTBLUE,
    LIGHTGREEN,
    LIGHTCYAN,
    LIGHTRED,
    LIGHTMAGENTA,
    YELLOW,
    WHITE
};

/* Static variables used by tmwrite.c  */
static struct screen_info screen;

static struct termios_screen_driver *screen_driver;

static enum cmd_parser_states cmd_state;

static enum COLORS screen_color[] = {BLACK, RED, GREEN, BROWN,
                                     BLUE, MAGENTA, CYAN, LIGHTGRAY};

/* termios tty functions.  */
static ssize_t __libc_termios_write (int handle, const void *buffer, size_t count, ssize_t *rv);
static ssize_t __libc_termios_write_tty(int handle, const void *buffer, size_t count, int cooked);

/* Output command functions.  */
static size_t parse_console_command(const unsigned char *buf, size_t count);
static void execute_console_command(const unsigned char cmd, unsigned char argc, unsigned int args[NPAR]);

/* Screen manipulation functions.  */
static void set_cursor(int col, int row);
static void move_cursor(int x_delta, int y_delta);
static void get_cursor(int *col, int *row);

/* Attribute helpers.  */
static void set_blink_attrib(int enable_blink);

/* Functions to output directly to the video buffer.  */
static inline unsigned long get_video_offset(int col, int row);
static void direct_write_ch(unsigned char ch, int *col, int *row);
static void direct_clear(int x1, int y1, int x2, int y2);
static void direct_scroll_up(int y1, int y2, int delta);
static void direct_scroll_down(int y1, int y2, int delta);
static void direct_scroll_forward(int x1, int y1, int x2, int y2, int xdst, int ydst);
static void direct_scroll_backward(int x1, int y1, int x2, int y2, int xdst, int ydst);

/* This driver outputs directly to the video buffer.  */
static struct termios_screen_driver direct_screen_driver =
  { direct_write_ch, direct_scroll_up, direct_scroll_down,
    direct_scroll_forward, direct_scroll_backward, direct_clear };


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

  /* Does it normally blink when bg has its 3rd bit set?  */
  screen.norm_blink = (_farnspeekb(0x465) & 0x20) ? 1 : 0;
  screen.cur_blink = screen.norm_blink;

  /* Determine the video offset.  */
  screen.video_buffer = _go32_info_block.linear_address_of_primary_screen;
  screen.is_mono = (screen.video_buffer == 0xb000 * 16);

  r.h.ah = 0x08;
  r.h.bh = screen.active_page;
  __dpmi_int(0x10, &r);

  screen.init_attrib = r.h.ah;
  screen.attrib = r.h.ah;

  screen_driver = &direct_screen_driver;
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
  int col, row;

  bytes = count;

  get_cursor(&col, &row);

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

      if (ch == '\e')
      {
        int crsr_col, crsr_row;

        get_cursor(&crsr_col, &crsr_row);
        if (crsr_col != col || crsr_row != row)
          set_cursor(col, row);
      }

      delta = parse_console_command(rp, n);

      /* Cursor may have moved.  */
      get_cursor(&col, &row);

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
      screen_driver->write_ch(' ', &col, &row);
      while ((col % 8) != 0)
        screen_driver->write_ch(' ', &col, &row);
      continue;
    }

    if (!raw)
    {
      /* NOTE: multibyte character don't contain control character */
      /* map NL to CRNL */
      if (ch == '\n' && (__libc_tty_p->t_oflag & ONLCR))
        screen_driver->write_ch('\r', &col, &row);
      /* map CR to NL */
      else if (ch == '\r' && (__libc_tty_p->t_oflag & OCRNL))
        ch = '\n';
    }
    screen_driver->write_ch(ch, &col, &row);
  }

  set_cursor(col, row);

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
  int col, row;
  int col_arg, row_arg;

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
          screen_driver->clear(col, row, screen.max_col, screen.max_row);
          break;

        /* Erase from start of screen to cursor.  */
        case 1:
          get_cursor(&col, &row);
          screen_driver->clear(0, 0, col, row);
          break;

        /* Erase the whole screen.  */
        case 2:
          screen_driver->clear(0, 0, screen.max_col, screen.max_row);
          break;
      }

    /* EL: Edit in Line */
    case 'K':
      switch (GET_ARG(0, 0))
      {
        /* Clear from cursor to end of line.  */
        case 0:
          get_cursor(&col, &row);
          screen_driver->clear(col, row, screen.max_col, row);
          break;

        /* Clear from start of line to cursor.  */
        case 1:
          get_cursor(&col, &row);
          screen_driver->clear(0, row, col, row);
          break;

        /* Clear the whole line.  */
        case 2:
          get_cursor(&col, &row);
          screen_driver->clear(0, row, screen.max_col, row);
          break;
      }

    /* IL: Insert Line */
    case 'L':
      get_cursor(&col, &row);
      row_arg = row + GET_ARG(0, 1);
      if (row_arg <= screen.max_row)
        screen_driver->scroll_down(row, screen.max_row, row_arg);
      else
        screen_driver->clear(0, row, screen.max_col, screen.max_row);
      break;

    /* DL: Delete Line */
    case 'M':
      get_cursor(&col, &row);
      row_arg = GET_ARG(0, 1);
      if (row_arg <= screen.max_row)
        screen_driver->scroll_up(row_arg, screen.max_row, row);
      break;

    /* SU: Scroll Up */
    case 'S':
      row_arg = GET_ARG(0, 1);
      if (row_arg <= screen.max_row)
        screen_driver->scroll_up(row_arg, screen.max_row, 0);
      else
        screen_driver->clear(0, 0, screen.max_col, screen.max_row);
      break;

    /* ST: Scroll Down */
    case 'T':
      row_arg = GET_ARG(0, 1);
      if (row_arg <= screen.max_row)
        screen_driver->scroll_down(0, screen.max_row, row_arg);
      else
        screen_driver->clear(0, 0, screen.max_col, screen.max_row);
      break;

    /* ICH: Insert Character */
    case '@':
      get_cursor(&col, &row);
      col_arg = col + GET_ARG(0, 1);
      if (col_arg <= screen.max_col)
        screen_driver->scroll_forward(col, row, screen.max_col, row,
                                      col_arg, row);
      else
        screen_driver->clear(col, row, screen.max_col, row);
      break;

    /* DCH: Delete Character */
    case 'P':
      get_cursor(&col, &row);
      col_arg = col + GET_ARG(0, 1);
      if (col_arg <= screen.max_col)
        screen_driver->scroll_backward(col_arg, row,
                                       screen.max_col, row,
                                       col, row);
      else
        screen_driver->clear(col, row, screen.max_col, row);
      break;

    /* ECH: Erase Character */
    case 'X':
      get_cursor(&col, &row);
      col_arg = col + GET_ARG(0, 1);
      if (col_arg > screen.max_col)
        col_arg = screen.max_col;
      screen_driver->clear(col, row, col_arg,  row);
      break;

    /* Color */

    /* SGR: Set Graphic Rendition */
    case 'm':
    {
      int i;
      unsigned char fg, bg;

      if (argc == 0)
      {
        screen.attrib = screen.init_attrib;
        break;
      }

      fg = screen.attrib & 0x0f;
      bg = (screen.attrib >> 4) & 0x0f;

      i = 0;
      while (i < argc)
      {
        switch(args[i])
        {
          case 0:
            fg = screen.init_attrib & 0x0f;
            bg = (screen.init_attrib >> 4) & 0x0f;
            break;

          case 1:	/* intensity on */
            fg |= 8;
            break;

          case 2:	/* intensity off */
            fg &= ~8;

          case 4:	/* underline on.  */
            fg |= 8;	/* We can't, so make it bold instead.  */
            break;

          case 5:	/* blink */
            if (screen.cur_blink == 0)
            {
              /* Ensure blinking is enabled.  */
              set_blink_attrib(1);
              screen.cur_blink = 1;
            }
            bg |= 8;
            break;

          case 7:	/* reverse video */
          {
            unsigned char t = fg;
            fg = bg;
            bg = t;

            /* If it was blinking before, let it blink after.  */
            if (fg & 8)
              bg |= 8;

            /* If the fg was bold, let the background be bold.  */
            if ((t & 8) && screen.cur_blink != 0)
            {
              set_blink_attrib(0);
              screen.cur_blink = 0;
            }
            break;
          }

          case 8:	/* concealed on */
            fg = (bg & 7) | 8;	/* make fg be like bg, only bright */
            break;

          case 25:	/* no blink */
            if (screen.cur_blink != 0)
            {
              /* Ensure we aren't in blink mode.  */
              set_blink_attrib(0);
              screen.cur_blink = 0;
            }
            bg &= ~8;
            break;

          case 30: case 31: case 32: case 33: /* foreground color */
          case 34: case 35: case 36: case 37:
            fg = (fg & 8) | (screen_color[args[i] - 30] & 15);
            break;

          case 40: case 41: case 42: case 43: /* background color */
          case 44: case 45: case 46: case 47:
            bg = (bg & 8) | (screen_color[args[i] - 40] & 15);
            break;

          default:
            break;
        }
        ++i;
      }

     /* They don't *really* want it invisible, do they?  */
     if (fg == (bg & 7))
        fg |= 8;  /* make it concealed instead */

      /* Construct the text attribute and set it.  */
      screen.attrib = (bg << 4) | fg;
    }

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


/* Functions to support direct output to the video buffer.  */

static inline void
__bios_output(unsigned char ch)
{
  __dpmi_regs r;

  if (ch == 0xff)
    return;

  r.h.ah = 0x0e;
  r.h.al = ch;
  r.h.bh = screen.active_page;
  __dpmi_int (16, &r);
}

static void
direct_write_ch(unsigned char ch, int *col, int *row)
{
  if (ch == '\n')
    ++(*row);
  else if (ch == '\r')
    *col = 0;
  else if (ch == '\b')
  {
    if (*col > 0)
      --(*col);
    else if (*row > 0)
    {
      --(*row);
      *col = screen.max_col;
    }
  }
  else if (ch == 0x07)
    __bios_output((unsigned char)ch);
  else
  {
    unsigned long ptr = get_video_offset(*col, *row);
    _farpokew(_dos_ds, ptr, ch | (screen.attrib << 8));
    ++(*col);
  }

  if (*col > screen.max_col)
  {
    *col = 0;
    ++(*row);
  }
  if (*row > screen.max_row)
  {
    direct_scroll_backward(0, 1, screen.max_col, screen.max_row, 0, 0);
    --(*row);
  }
}

/* Clear from (x1, y1) to (x2, y2).  */
static void
direct_clear(int x1, int y1, int x2, int y2)
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
direct_scroll_forward(int x1, int y1, int x2, int y2, int xdst, int ydst)
{
  unsigned short fill;
  unsigned long fill_l;
  unsigned long dst_ptr, src_ptr, dst_end, src_end;

  int xdst2, ydst2;

  xdst2 = xdst + (x2 - x1);
  ydst2 = ydst + (y2 - y1);

  /* Compute our beginning and ending offsets.  */
  dst_ptr = get_video_offset(xdst2, ydst2);
  src_ptr = get_video_offset(x2, y2);
  dst_end = get_video_offset(xdst, ydst);
  src_end = get_video_offset(x1, y1);

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
direct_scroll_backward(int x1, int y1, int x2, int y2, int xdst, int ydst)
{
  unsigned short fill;
  unsigned long fill_l;
  unsigned long dst_ptr, src_ptr, dst_end, src_end;
  int xdst2, ydst2;

  xdst2 = xdst + (x2 - x1);
  ydst2 = ydst + (y2 - y1);

  /* Compute our beginning and ending offsets.  */
  dst_ptr = get_video_offset(xdst, ydst);
  src_ptr = get_video_offset(x1, y1);
  dst_end = get_video_offset(xdst2, ydst2);
  src_end = get_video_offset(x2, y2);

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

  /* Align fill on 4-byte boundary. */
  if ((dst_end % 4) != 0)
  {
    _farnspokew(dst_end, fill);
    dst_end += 2;
  }

  /* Fill 4-bytes or 2 characters at a time.  */
  while (dst_end < src_end)
  {
    _farnspokel(dst_end, fill_l);
     dst_end += 4;
  }

  /* Special handling when last character isn't on a 4-byte boundary.  */
  if (dst_end == src_end)
    _farnspokew(src_end, fill);
}

static void direct_scroll_up(int y1, int y2, int ydst)
{
  direct_scroll_backward(0, y1, screen.max_col, y2, 0, ydst);
}

static void direct_scroll_down(int y1, int y2, int ydst)
{
  direct_scroll_forward(0, y1, screen.max_col, y2, 0, ydst);
}

static void
set_blink_attrib(int enable_blink)
{
  __dpmi_regs r;

  r.h.bh = 0;
  r.h.bl = (enable_blink) ? 1 : 0;
  r.x.ax = 0x1003;
  __dpmi_int(0x10, &r);
}

/* Restore the BIOS blinking bit to its original value.  Called at exit.  */
static void __attribute__((destructor))
restore_blink_bit(void)
{
  if (screen.cur_blink != screen.norm_blink)
    set_blink_attrib(screen.norm_blink);
}

