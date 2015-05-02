/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <bios.h>
#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <keys.h>
#include <setjmp.h>
#include <stdarg.h>
#include "ed.h"
#include "screen.h"
void *xmalloc (size_t);
/* ------------------------------------------------------------------------- */
static char *colours[] = {
  "black",        "blue",         "green",        "cyan",
  "red",          "purple",       "brown",        "grey",
  "darkgrey",     "lightblue",    "lightgreen",   "lightcyan",
  "lightred",     "lightmagenta", "yellow",       "white"};
/* ------------------------------------------------------------------------- */
/* At position (X,Y) place TXT in the debugger screen.  */

inline void
put (int x, int y, char *txt)
{
  unsigned char *p
    = (unsigned char *)(debug_screen_save + 3) + 2 * (cols * y + x);
  unsigned char *ptxt = (unsigned char *)txt;

  while (*ptxt)
    *p++ = *ptxt++,
    *p++ = screen_attr;
}
/* ------------------------------------------------------------------------- */
/* At position (X,Y) left-justify to width L this TXT in the debugger
   screen.  */

inline void
putl (int x, int y, int l, char *txt)
{
  unsigned char *p
    = (unsigned char *)(debug_screen_save + 3) + 2 * (cols * y + x);
  unsigned char *ptxt = (unsigned char *)txt;

  while (*ptxt && l > 0)
    *p++ = *ptxt++,
    *p++ = screen_attr,
    l--;
  while (l-- > 0)
    *p++ = ' ',
    *p++ = screen_attr;
}
/* ------------------------------------------------------------------------- */
/* At position (X,Y) place CHAR.  Add DELTA to the screen address and repeat
   COUNT times.  This is mainly used to draw frames.  */

inline void
draw (int x, int y, unsigned char ch, int delta, int count)
{
  short unsigned *p
    = (short unsigned *)(debug_screen_save + 3) + (cols * y + x);
  short unsigned attrch = ((unsigned)screen_attr << 8) + ch;

  while (count--)
    *p = attrch,
    p += delta;
}
/* ------------------------------------------------------------------------- */
/* Highlight the text at (X,Y) length LEN by updating the attribute.  */

void
highlight (int x, int y, int len)
{
  unsigned short *p
    = (unsigned short *)(debug_screen_save + 4) + (cols * y + x);

  while (len--)
    *(unsigned char *)p = screen_attr,
    p++;
}
/* ------------------------------------------------------------------------- */
/* Draw a frame with its corners at (X1,Y1) and (X2,Y2).  */

void
frame (int x1, int y1, int x2, int y2)
{
  draw (x1 + 1, y1, 'Ä', 1,    x2 - x1 - 1);
  draw (x1 + 1, y2, 'Ä', 1,    x2 - x1 - 1);
  draw (x1, y1 + 1, '³', cols, y2 - y1 - 1);
  draw (x2, y1 + 1, '³', cols, y2 - y1 - 1);
  put (x1, y1, "Ú");
  put (x2, y1, "¿");
  put (x1, y2, "À");
  put (x2, y2, "Ù");
}
/* ------------------------------------------------------------------------- */
/* Display (on the physical screen) some virtual SCREEN.  */

void
put_screen (char *screen)
{
  int addr;

  switch (*screen++)
    {
    case 0:
      /* Text screen, primary display.  */
      ScreenSetCursor (screen[0], screen[1]);
      ScreenUpdate (screen + 2);
      break;
    case 1:
      /* Text screen, secondary display.  */
      addr = (screen[0] * cols + screen[1]) * 2;
      outportb (0x3b4, 0xf); outportb (0x3b5, addr & 0xff);
      outportb (0x3b4, 0xe); outportb (0x3b5, addr >> 8);
      movedata (_go32_my_ds (), (unsigned) (screen + 2),
		_go32_conventional_mem_selector (), 0xb0000,
		cols * rows * 2);
      break;
    }
}
/* ------------------------------------------------------------------------- */
/* Return a (malloc'ed) virtual copy of the currently displayed screen.  */

char *
get_screen (void)
{
  char *p;
  int r, c, addr;

  p = xmalloc (cols * rows * 2 + 3);
  if (dual_monitor_p)
    {
      /* Get the secondary display's contents.  */

      p[0] = 1;
      addr = (outportb (0x3b4, 0xf), inportb (0x3b5))
	+ (outportb (0x3b4, 0xe), inportb (0x3b5) << 8);
      p[1] = (addr / 2) / cols;
      p[2] = (addr / 2) % cols;
      movedata (_go32_conventional_mem_selector (), 0xb0000,
		_go32_my_ds (), (unsigned) (p + 3),
		cols * rows * 2);
    }
  else
    {
      /* Get the primary display's contents.  */
      ScreenGetCursor (&r, &c);
      p[0] = 0;
      p[1] = r;
      p[2] = c;
      ScreenRetrieve (p + 3);
    }
  return p;
}
/* ------------------------------------------------------------------------- */
/* Display the debugger screen (if not already displayed).  */

void
debug_screen (void)
{
  if (!debug_screen_p)
    {
      if (!dual_monitor_p)
	{
	  user_screen_save = get_screen ();
	  put_screen (debug_screen_save);
	}
      debug_screen_p = 1;
    }
}
/* ------------------------------------------------------------------------- */
/* Display the user screen (if not already displayed).  */

void
user_screen (void)
{
  if (debug_screen_p)
    {
      if (!dual_monitor_p)
	{
	  put_screen (user_screen_save);
	  free (user_screen_save);
	}
      debug_screen_p = 0;
    }
}
/* ------------------------------------------------------------------------- */
/* Reportedly, `sleep' & `gettimeofday' are buggy under Dpmi -- emulate.
   Anyway, this means that we can make a keypress abort the sleeping.  */

static int
mysleep (int secs)
{
  struct time now;
  unsigned now100, then100;

  gettime (&now);
  then100
    = ((now.ti_hour * 60 + now.ti_min) * 60 + now.ti_sec + secs) * 100
      + now.ti_hund;
  do
    {
      gettime (&now);
      now100 = ((now.ti_hour * 60 + now.ti_min) * 60 + now.ti_sec) * 100
	+ now.ti_hund;
      if (now100 < 100 || kbhit ())
	break; /* Day turnover or key pressed.  */
    }
  while (now100 < then100);
  return 0;
}
/* ------------------------------------------------------------------------- */
void
message (CL_TYPE class, char *fmt, ...)
{
  va_list args;
  char *save, *buf = alloca (cols);
  unsigned char saveattr = screen_attr;
  int len, y = rows / 2;

  va_start(args, fmt);
  vsprintf (buf, fmt, args);
  va_end(args);

  len = strlen (buf);
  save = debug_screen_save;
  debug_screen_save = get_screen ();
  switch (class)
    {
    case CL_Error:
      screen_attr = screen_attr_error;
      break;
    default:
      screen_attr = screen_attr_message;
    }
  frame (1, y - 1, cols - 2, y + 1);
  draw (2, y, ' ', 1, cols - 4);
  put ((cols - len) / 2, y, buf);
  screen_attr = saveattr;
  put_screen (debug_screen_save);
  switch (class)
    {
    case CL_Info:
      mysleep (2);
      break;
    case CL_Msg:
    case CL_Error:
      (void) getxkey ();
    }
  free (debug_screen_save);
  debug_screen_save = save;
  put_screen (debug_screen_save);
}
/* ------------------------------------------------------------------------- */
/* Read a string from the keyboard to `read_buffer'.  The entry is started
   with STARTTEXT.  This function uses an obscene amount of redrawing, but
   who cares?  */

int
read_string (char *starttext)
{
  char *save;
  int key, esc = 0, pos, leave = 0, y = rows / 2;

  save = debug_screen_save;
  debug_screen_save = get_screen ();
  strcpy (read_buffer, starttext);
  pos = strlen (read_buffer);

  frame (1, y - 1, cols - 2, y + 1);
  do
    {
      draw (2, y, ' ', 1, cols - 4);
      put (3, y, read_buffer);
      pos = strlen (read_buffer);
      put_screen (debug_screen_save);
      ScreenSetCursor (y, 3 + pos);
      key = getxkey ();
      switch (key)
	{
	case K_Return:
	  leave = 1;
	  break;
	case K_Escape:
	  leave = 1;
	  esc = 1;
	  read_buffer[0] = '\0';
	  break;
	case K_BackSpace:
	  if (pos > 0)
	    read_buffer[--pos] = '\0';
	  break;
	default:
	  if (key >= ' ' && key <= 0xff && pos <= cols - 7)
	    {
	      read_buffer[pos++] = key & 0xff;
	      read_buffer[pos] = '\0';
	    }
	}
    }
  while (!leave);
  free (debug_screen_save);
  debug_screen_save = save;
  put_screen (debug_screen_save);
  return esc;
}
/* ------------------------------------------------------------------------- */
void
init_screen (void)
{
  if (dual_monitor_p)
    rows = 25, cols = 80;
  else
    {
      cols = ScreenCols ();
      rows = ScreenRows ();
      if (cols < 80 || rows < 25)
	{
	  fprintf (stderr, "\n\
Debugger error:\n\
There are only %d columns and %d rows\n\
in this display mode.\n\
The debugger needs at least\n\
80 columns and 25 rows.\n",
		   cols, rows);
	  exit (1);
	}
    }
  toplines = (rows / 2) + 4;
  bottomlines = rows - 3 - toplines;
}
/* ------------------------------------------------------------------------- */
/* Set the default colours.  */

void
init_colours (void)
{
  switch (dual_monitor_p ? 7 : ScreenMode ())
    {
    case 2:
    case 7:
      /* Mono */
      screen_attr_normal    = (A_black << 4) + A_grey;
      screen_attr_source    = screen_attr_normal;
      screen_attr_focus	    = (A_grey  << 4) + A_black;
      screen_attr_break	    = (A_black << 4) + A_white;
      screen_attr_message   = (A_grey  << 4) + A_white;
      screen_attr_error	    = (A_grey  << 4) + A_white;
      screen_attr_menu      = (A_grey  << 4) + A_black;
      screen_attr_menufocus = (A_black << 4) + A_white;
      screen_attr_editframe = (A_grey  << 4) + A_black;
      screen_attr_edittxt   = (A_grey  << 4) + A_black;
      screen_attr_editfield = (A_grey  << 4) + A_black;
      screen_attr_editfocus = (A_black << 4) + A_white;
      break;
    default:
      /* Colour */
      screen_attr_normal    = (A_cyan  << 4) + A_blue;
      screen_attr_source    = (A_cyan  << 4) + A_black;
      screen_attr_focus	    = (A_blue  << 4) + A_white;
      screen_attr_break	    = (A_red   << 4) + A_white;
      screen_attr_message   = (A_green << 4) + A_white;
      screen_attr_error	    = (A_red   << 4) + A_white;
      screen_attr_menu      = (A_grey  << 4) + A_black;
      screen_attr_menufocus = (A_black << 4) + A_white;
      screen_attr_editframe = (A_blue  << 4) + A_grey;
      screen_attr_edittxt   = (A_blue  << 4) + A_grey;
      screen_attr_editfield = (A_blue  << 4) + A_white;
      screen_attr_editfocus = (A_red   << 4) + A_white;
    }
}
/* ------------------------------------------------------------------------- */
void
screen_mode (int rows)
{
  union REGS regs;
  int font, cursor;

  switch (rows)
    {
    case 43:
    case 50:
      font = 0x1112;
      cursor = 0x0607;
      break;
    case 25:
    case 28:
    default:
      font = 0x1111;
      cursor = 0x0607;
    }
  regs.w.ax = 0x0003;
  int86 (0x10, &regs, &regs);
  regs.w.ax = font;
  regs.h.bl = 0;
  int86 (0x10, &regs, &regs);
  regs.w.ax = 0x1200;
  regs.h.bl = 32;
  int86 (0x10, &regs, &regs);
  if (rows == 25)
    {
      regs.w.ax = 0x0003;
      int86 (0x10, &regs, &regs);
    }
  regs.w.ax = 0x0100;
  regs.w.cx = cursor;
  int86 (0x10, &regs, &regs);
}
/* ------------------------------------------------------------------------- */
int
menu (char *title, MENU_ITEM *m, int *focus)
{
  char *save;
  int width, i, count, state;
  unsigned char saveattr = screen_attr;
  int x0, y0;

  save = debug_screen_save;
  debug_screen_save = get_screen ();

  width = title ? strlen (title) : 0;
  for (count = 0; m[count].txt; count++)
    {
      i = strlen (m[count].txt);
      if (i > width) width = i;
    }
  width += 2;
  x0 = cols / 2 - width / 2;
  y0 = rows / 2 - count / 2;

  screen_attr = screen_attr_menu;
  frame (x0 - 1, y0 - 1, x0 + width, y0 + count);
  if (title)
    put (cols / 2 - strlen (title) / 2, y0 - 1, title);

  state = 0;
  while (state == 0)
    {
      for (i = 0; i < count; i++)
	{
	  screen_attr
	    = (i == *focus) ? screen_attr_menufocus : screen_attr_menu;
	  putl (x0 + 1, y0 + i, width - 1, m[i].txt);
	  put (x0, y0 + i, " ");
	}
      put_screen (debug_screen_save);
      switch (getxkey ())
	{
	case K_Escape:
	  state = 1;
	  break;
	case K_Return:
	  state = 2;
	  break;
	case K_Up:
	case K_EUp:
	case K_Left:
	case K_ELeft:
	  if (*focus) (*focus)--;
	  break;
	case K_Down:
	case K_EDown:
	case K_Right:
	case K_ERight:
	  if (*focus < count - 1) (*focus)++;
	  break;
	case K_Home:
	case K_EHome:
	  *focus = 0;
	  break;
	case K_End:
	case K_EEnd:
	  *focus = count - 1;
	  break;
	}
    }
  screen_attr = saveattr;
  free (debug_screen_save);
  debug_screen_save = save;
  put_screen (debug_screen_save);
  if (state == 2)
    {
      if (m[*focus].handler)
	m[*focus].handler (m[*focus].info);
      return *focus;
    }
  else
    return -1;
}
/* ------------------------------------------------------------------------- */
int
edit (char *title, EDIT_ITEM *fields, int focus)
{
  char *save;
  unsigned char saveattr = screen_attr;
  int i, leave, esc = 0, count, key = 0, width, *length;
  int x0, y0, x, y, len;
  char *data;

  save = debug_screen_save;
  debug_screen_save = get_screen ();

  for (count = 0; fields[count].txt; count++)
    /* Nothing.  */;
  x0 = 3;
  y0 = rows / 2 - count / 2;
  screen_attr = screen_attr_editframe;
  length = alloca (count * sizeof (int));
  frame (x0 - 1, y0 - 1, cols - x0, y0 + count);
  if (title)
    put (cols / 2 - strlen (title) / 2, y0 - 1, title);

  screen_attr = screen_attr_edittxt;
  for (i = 0; i < count; i++)
    {
      put (x0, y0 + i, " ");
      put (x0 + 1, y0 + i, fields[i].txt);
      length[i] = strlen (fields[i].txt);
    }

  leave = 0;
  while (!leave)
    {
      for (i = 0; i < count; i++)
	{
	  screen_attr = screen_attr_editfield;
	  width = cols - 8 - length[i];
	  x = x0 + length[i] + 1, y = y0 + i;
	  put (x - 1, y, " ");
	  putl (x, y, 1 + width, fields[i].data);
	}
      data = fields[focus].data;
      screen_attr = screen_attr_editfocus;
      width = cols - 8 - length[focus];
      x = x0 + length[focus] + 1, y = y0 + focus;
      put (x - 1, y, " ");
      len = strlen (data);
      while (!leave)
	{
	  putl (x, y, 1 + width, data);
	  put_screen (debug_screen_save);
	  ScreenSetCursor (y, x + len);
	  switch ((key = getxkey ()))
	    {
	    case K_Left:
	    case K_ELeft:
	    case K_BackSpace:
	      if (len)
		data[--len] = 0;
	      break;
	    case ' ' ... '~':
	      if (len < width)
		data[len++] = key, data[len] = 0;
	      break;
	    default:
	      leave = 1;
	      break;
	    }
	}
      leave = 0;
      switch (key)
	{
	case K_Escape:
	  leave = esc = 1;
	  break;
	case K_Up:
	case K_EUp:
	  focus = focus ? focus - 1 : count - 1;
	  break;
	case K_Return:
	  leave = (focus == count - 1);
	  /* Fall through.  */
	case K_Down:
	case K_EDown:
	  focus = (focus == count - 1) ? 0 : focus + 1;
	  break;
	case K_Home:
	case K_EHome:
	  focus = 0;
	  break;
	case K_End:
	case K_EEnd:
	  focus = count - 1;
	  break;
	}
    }

  screen_attr = saveattr;
  free (debug_screen_save);
  debug_screen_save = save;
  put_screen (debug_screen_save);
  return !esc;
}
/* ------------------------------------------------------------------------- */
static void
attr2text (char *buf, unsigned char attr)
{
  sprintf (buf, "%s on %s%s", 
	   colours[attr & 15], colours[(attr >> 4) & 7], \
	   (attr & 128) ? ", blinking" : "");
  buf[0] = toupper (buf[0]);
}

static int
text2attr (char *s)
{
  char *s1, *s2, *cs;
  int f, b, l;

  l = strlen (s) + 1;
  s = strcpy (alloca (l), s);
  s1 = alloca (l);
  s2 = alloca (l);
  if ((cs = strchr (s, ',')))
  {
    if (stricmp (cs, ", blinking"))
      return -1;
    else
      *cs = 0;
  }
  if (sscanf (s, "%s on %s", s1, s2) == 2)
    {
      for (f = 0; f < 16; f++)
	if (stricmp (colours[f], s1) == 0)
	  break;
      for (b = 0; b < 8; b++)
	if (stricmp (colours[b], s2) == 0)
	  break;
      if (f < 16 && b < 8)
	return (cs ? 128 : 0) | (b << 4) | f;
    }
  return -1;
}

void
edit_colours (int dummy)
{
  int i, focus;
  static EDIT_ITEM form[] = {
    {"Normal ..............: ", 0},
    {"Source Code .........: ", 0},
    {"Foci ................: ", 0},
    {"Breakpoints .........: ", 0},
    {"Messages ............: ", 0},
    {"Errors ..............: ", 0},
    {"Menus ...............: ", 0},
    {"Menu Foci ...........: ", 0},
    {"Form Frames .........: ", 0},
    {"Form Legends ........: ", 0},
    {"Form Fields .........: ", 0},
    {"Form Foci ...........: ", 0},
    {0, 0}};

  for (i = 0; form[i].txt; i++) form[i].data = alloca (cols);
  attr2text (form[ 0].data, screen_attr_normal);
  attr2text (form[ 1].data, screen_attr_source);
  attr2text (form[ 2].data, screen_attr_focus);
  attr2text (form[ 3].data, screen_attr_break);
  attr2text (form[ 4].data, screen_attr_message);
  attr2text (form[ 5].data, screen_attr_error);
  attr2text (form[ 6].data, screen_attr_menu);
  attr2text (form[ 7].data, screen_attr_menufocus);
  attr2text (form[ 8].data, screen_attr_editframe);
  attr2text (form[ 9].data, screen_attr_edittxt);
  attr2text (form[10].data, screen_attr_editfield);
  attr2text (form[11].data, screen_attr_editfocus);
  focus = 0;
  while (edit ("Edit Colour Scheme", form, focus))
    {
      for (focus = 0; form[focus].txt; focus++)
	if (text2attr (form[focus].data) == -1)
	  break;
      if (form[focus].txt == 0)
	{
	  screen_attr_normal    = text2attr (form[ 0].data);	  
	  screen_attr_source    = text2attr (form[ 1].data);
	  screen_attr_focus     = text2attr (form[ 2].data);
	  screen_attr_break     = text2attr (form[ 3].data);
	  screen_attr_message   = text2attr (form[ 4].data);
	  screen_attr_error     = text2attr (form[ 5].data);
	  screen_attr_menu      = text2attr (form[ 6].data);
	  screen_attr_menufocus = text2attr (form[ 7].data);
	  screen_attr_editframe = text2attr (form[ 8].data);
	  screen_attr_edittxt   = text2attr (form[ 9].data);
	  screen_attr_editfield = text2attr (form[10].data);
	  screen_attr_editfocus = text2attr (form[11].data);
	  return;
	}
    }
}
/* ------------------------------------------------------------------------- */
