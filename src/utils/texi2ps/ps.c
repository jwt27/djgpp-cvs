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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "ps.h"
#include "word.h"

#define COURIER_ALL_SAME 1

#include "cou.h"
#if !COURIER_ALL_SAME
#include "coub.h"
#include "coui.h"
#include "coubi.h"
#endif
#include "ding.h"
#include "hlv.h"
#include "hlvb.h"
#include "hlvi.h"
#include "hlvbi.h"
#include "sym.h"
#include "tms.h"
#include "tmsb.h"
#include "tmsi.h"
#include "tmsbi.h"

static float *all_metrics[] = {
  fm_tms, fm_tmsb, fm_tmsi, fm_tmsbi,
#if COURIER_ALL_SAME
  fm_cou, fm_cou, fm_cou, fm_cou,
#else
  fm_cou, fm_coub, fm_coui, fm_coubi,
#endif
  fm_sym, fm_sym, fm_sym, fm_sym,
  fm_hlv, fm_hlvb, fm_hlvi, fm_hlvbi,
  fm_ding, fm_ding, fm_ding, fm_ding
};

float *font_metrics=fm_tms;
static int ps_col = 0;

void
psprintf(char *fmt, ...)
{
  char tmp[1000];
  int need;
  va_list args = 0;
  va_start(args, fmt);
  vsprintf(tmp, fmt, args);
  va_end(args);
  need = strlen(tmp)+1;
  if (ps_col+need > 78)
  {
    putchar('\n');
    ps_col = 0;
  }
  else if (ps_col)
    putchar(' ');
  fputs(tmp, stdout);
  ps_col += need;
}

void
pscomment(char *fmt, ...)
{
  char tmp[1000];

  if (ps_col)
    putchar('\n');
  if (fmt)
  {
    va_list args = 0;
    va_start(args, fmt);
    vsprintf(tmp, fmt, args);
    fputs("% ", stdout);
    fputs(tmp, stdout);
    va_end(args);
  }
  putchar('\n');
  ps_col = 0;
}

static const char PS_ESC[] = "()\\";

void
psputw(char *w)
{
  char *wp;
  int need = strlen(w) + 2;
  for (wp=w; *wp; wp++)
    if (strchr(PS_ESC, *wp))
      need++;

  if (ps_col+need > 78)
  {
    putchar('\n');
    ps_col = 0;
  }
  else if (ps_col)
    putchar(' ');

  putchar('(');
  while (*w)
  {
    if (strchr(PS_ESC, *w))
      putchar('\\');
    putchar(*w++);
  }
  putchar(')');
  ps_col += need;
}

typedef struct PS_FSTACK {
  struct PS_FSTACK *prev;
  int ps_font;
  int scale;
} PS_FSTACK;

static PS_FSTACK *ps_fstack = 0;
int ps_font=0;
int ps_fontsize=10;

static int old_ps_font = 0;
static int old_ps_fontsize = 0;

static void
do_font()
{
  if (ps_font == old_ps_font && ps_fontsize == old_ps_fontsize)
    return;
  old_ps_font = ps_font;
  old_ps_fontsize = ps_fontsize;
/*  psprintf("/%s findfont %d scalefont setfont", font_names[ps_font], ps_fontsize); */
  font_metrics = all_metrics[ps_font];
}

void
psf_setfont()
{
  old_ps_fontsize = -1;
  do_font();
}

static void
push()
{
  PS_FSTACK *f = (PS_FSTACK *)malloc(sizeof(PS_FSTACK));
  f->prev = ps_fstack;
  ps_fstack = f;
  f->ps_font = ps_font;
  f->scale = ps_fontsize;
}

void
psf_pushset(int fl)
{
  push();
  ps_font |= fl;
  do_font();
}

void
psf_pushfont(int fl)
{
  push();
  ps_font &= ~(PSF_bold|PSF_italic);
  ps_font |= fl;
  do_font();
}

void
psf_pushreset(int fl)
{
  push();
  ps_font &= ~fl;
  do_font();
}

void
psf_pushscale(int sc)
{
  push();
  ps_fontsize = sc;
  do_font();
}

void
psf_pop()
{
  PS_FSTACK *f;
  if (!ps_fstack)
    return;
  ps_font = ps_fstack->ps_font;
  ps_fontsize = ps_fstack->scale;
  f = ps_fstack;
  ps_fstack = ps_fstack->prev;
  free(f);
  do_font();
}

void
psdone()
{
  psprintf("%s %d\n%c",
           "restore\n%%Trailer\n%%Pages:", /* DSC Trailer */
           --current_page, '\004');        /* check_eop() incremented page */
}
