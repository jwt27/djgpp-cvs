/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* texi2ps -- convert texinfo format files into Postscript files.

   Copyright (C) 1995 DJ Delorie (dj@delorie.com)

   texi2ps is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.  No author or distributor accepts
   responsibility to anyone for the consequences of using it or for
   whether it serves any particular purpose or works at all, unless he
   says so in writing.  Refer to the GNU General Public License
   for full details.

   Everyone is granted permission to copy, modify and redistribute
   texi2ps, but only under the conditions described in the GNU
   General Public License.   A copy of this license is supposed to
   have been given to you along with texi2ps so you can know your
   rights and responsibilities.  It should be in a file named COPYING.
   Among other things, the copyright notice and this notice must be
   preserved on all copies.  */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "screenio.h"

static int last_l=0;
int screenio_enabled = 0;

static void
show(char *fmt, va_list a)
{
  int ol, l;
  fputc('\r', stderr);
  l = ol = vfprintf(stderr, fmt, a);
  while (l < last_l)
  {
    fputc(' ', stderr);
    l++;
  }
  fflush(stderr);
  last_l = ol;
}

void screenio_note(char *fmt, ...)
{
  va_list a=0;
  if (!screenio_enabled)
    return;
  va_start(a,fmt);
  show(fmt, a);
  va_end(a);
}

void screenio_print(char *fmt, ...)
{
  va_list a=0;
  if (!screenio_enabled)
    return;
  va_start(a,fmt);
  show(fmt, a);
  va_end(a);
  fputc('\n', stderr);
  last_l = 0;
}
