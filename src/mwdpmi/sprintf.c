/* ---------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                 */
/* Distributed under the GPL, see COPYING for details.                    */
/* ---------------------------------------------------------------------- */

#include "code32.h"

static int
format (char *buf, const char *fmt, void *args)
{
  char *buf0 = buf;
  int doit = (buf != 0);

  while (1)
    switch (*fmt)
      {
      case 0:
	if (doit) *buf = 0;
	return buf - buf0;
      case '%':
	{
	  char *s, tmpbuf[8 * sizeof (long) + 2];
	  int zero_flag = 0;
	  int minus_flag = 0;
	  int alternate_flag = 0;
	  int width = -1, len;
	  int again = 1;

	  fmt ++;
	  while (again)
	    switch (*fmt)
	      {
	      case '0': zero_flag = 1; fmt++; break;
	      case '-': minus_flag = 1; fmt++; break;
	      case '#': alternate_flag = 1; fmt++; break;
	      default: again = 0;
	      }

	  if (*fmt >= '0' && *fmt <= '9')
	    {
	      width = 0;
	      while (*fmt >= '0' && *fmt <= '9')
		width = 10 * width + (*fmt++ - '0');
	    }

	  switch (*fmt++)
	    {
	    case 0:
	      if (doit) *buf = 0;
	      return buf - buf0;
	    case '%':
	      if (doit) *buf = '%';
	      buf++;
	      break;
	    case 'c':
	      tmpbuf[0] = *((int *) args);
	      tmpbuf[1] = 0;
	      args += sizeof (int);
	      s = tmpbuf;
	      zero_flag = 0;
	      goto string;
	    case 's':
	      s = *((char **) args);
	      args += sizeof (char *);
	      zero_flag = 0;
	      if (s == 0) s = "(null)";
	    string:
	      len = strlen (s);
	      if (width != -1 && !minus_flag)
		while (width > len)
		  if (doit)
		    *buf++ = (zero_flag ? '0' : ' '), width--;
		  else
		    buf++, width--;
	      while (*s)
		if (doit)
		  *buf++ = *s++, width--;
		else
		  buf++, s++, width--;
	      while (width > 0)
		if (doit)
		  *buf++ = (zero_flag ? '0' : ' '), width--;
		else
		  buf++, width--;
	      break;
	    case 'd':
	      s = itoa (*((int *) args), tmpbuf, 10);
	      args += sizeof (int);
	      goto string;
	    case 'p':
	      alternate_flag = 1;
	      /* Fall through.  */
	    case 'x':
	      tmpbuf[0] = '0';
	      tmpbuf[1] = 'x';
	      itoa (*((int *) args), tmpbuf + 2 * alternate_flag, 16);
	      args += sizeof (int);
	      s = tmpbuf + 2 * alternate_flag;
	      goto string;
	    default:
	      if (doit) *buf = '?';
	      buf++;
	    }
	}
	break;
      default:
	if (doit) *buf = *fmt;
	buf++, fmt++;
      }
}
/* ---------------------------------------------------------------------- */
void
sprintf (char *buf, const char *fmt, ...)
{
  format (buf, fmt, (&fmt) + 1);
}
/* ---------------------------------------------------------------------- */
/* These are not terrible efficient, but they'll have to do.  */

void
printf (const char *fmt, ...)
{
  int len = format (0, fmt, (&fmt) + 1);
  char *s = alloca (len + 1);

  format (s, fmt, (&fmt) + 1);
  write (DOS_STDOUT_FILENO, s, len);
}
/* ---------------------------------------------------------------------- */
void
eprintf (const char *fmt, ...)
{
  int len = format (0, fmt, (&fmt) + 1);
  char *s = alloca (len + 1);

  format (s, fmt, (&fmt) + 1);
  write (DOS_STDERR_FILENO, s, len);
}
/* ---------------------------------------------------------------------- */
