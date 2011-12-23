/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "word.h"
#include "ps.h"

typedef struct Word {
  struct Word *next;
  int x, y;
  char vskip;
  char pskip;
  char *word;
} Word;

typedef struct WordCache {
  int font;
  int size;
  struct Word *words;
  struct WordCache *next;
} WordCache;

static WordCache *word_cache = 0;

char *font_names[] = {
  "Times-Roman",
  "Times-Bold",
  "Times-Italic",
  "Times-BoldItalic",
  "Courier",
  "Courier-Bold",
  "Courier-Oblique",
  "Courier-BoldOblique"
  "Symbol",
  "Symbol",
  "Symbol",
  "Symbol",
  "Helvetica",
  "Helvetica-Bold",
  "Helvetica-Oblique",
  "Helvetica-BoldOblique",
  "Zapf-Dingbats",
  "Zapf-Dingbats",
  "Zapf-Dingbats",
  "Zapf-Dingbats"
};

static char word_buf[1000];
static int at_bol = 1;
static int baseline_offset = 0;
static int bol_size = 0;
static int ends_sentence = 0;
static int in_word_ws = 0;
static int marks_this_page = 0;
static int need_lb = 0;
static int need_wrap = 0;
static int page_flushing = 0;
static int page_number = 1;
static int para_open = 0;
static int pending_whitespace = 0;
static int ps_col = 0;
static int saw_indent = 0;
static int wp = 0;

int prevailing_indent = 0;
int current_page;
int vskip_enabled = 0;
int two_columns = 0;

int MARGIN;
int RIGHT;
int TOP;
static int halfpage = 0;

static int x, y;
static int x_last_ws;

static char *ps_init_tab[] = {
  "%!PS-Adobe-1.0\n",
  "%%Title: Texinfo converted to crude PS\n",
  "%%Creator: texi2ps\n",
  "%%CreationDate: ",
  "%%Pages: (atend)\n",
  "%%EndComments\n",
  "/m { moveto show } bind def\n",
#if 0
  "/sp1 { 450 150 translate 0.5 0.5 scale save \n",
  "0 792 moveto 612 792 lineto 612 0 lineto 0 0 lineto 0 792 lineto stroke } def\n",
#else
  "/sp1 { save } def\n",
#endif
  "/sp { restore showpage sp1 } def\n",
  /* load these before the save - these are the popular ones */
  "/Times-Roman findfont pop\n",
  "/Courier findfont pop\n",
  "/Helvetica findfont pop\n",
  "/Helvetica-Bold findfont pop\n",
  "sp1\n",
  0
};

void word_set_margins(int pts)
{
  MARGIN = pts;
  if (two_columns)
    RIGHT = (612 - MARGIN*3)/2;
  else
    RIGHT = 612 - MARGIN*2;
  TOP = 792 - pts*2;
}

void word_init(void)
{
  int i;
  for (i=0; ps_init_tab[i]; i++)
  {
    fputs(ps_init_tab[i], stdout);
    if (strcmp(ps_init_tab[i], "%%CreationDate: ") == 0)
    {
      time_t tod = time((time_t *)0);
      fputs(ctime(&tod), stdout);
    }
  }
  current_page = 1;
  psf_setfont();
  x = prevailing_indent;
  y = TOP - ps_fontsize;
}

void para_close(void)
{
  if (!para_open)
    return;
  line_skip();
  para_open = 0;
  saw_indent = 0;
}

void para_set_prevailing_indent(int amount)
{
  prevailing_indent += amount;
}

void para_set_indent(int amount)
{
  x = prevailing_indent + amount;
  saw_indent = 1;
}

static void check_eop()
{
  WordCache *wc;
  Word *we, *wet;
  if (y < 0 || page_flushing)
  {
    page_flushing = 0;

    if (halfpage == 0 && two_columns)
      {
	halfpage = 612 / 2 - MARGIN/2;
	pscomment("");    /* force line break */
	psprintf("%s %d %d", "%%Page:", page_number, page_number); /* DSC page */
	pscomment("Page %d", page_number);
      }
    else
      {
	halfpage = 0;
      }

    for (wc=word_cache; wc; wc=wc->next)
      if (wc->words)
      {
	pscomment("Page %d, Font %s, Size %d",
		  page_number, font_names[wc->font], wc->size);
	psprintf("restore save");
	psprintf("/%s findfont %d scalefont setfont", font_names[wc->font], wc->size);
	for (we=wc->words, wc->words=0; we; we=wet)
	{
	  if (we->vskip)
	    we->y -= y;
	  wet = we->next;
	  if (we->pskip)
	  {
	    we->next = wc->words;
	    wc->words = we;
	    we->pskip = we->vskip = 0;
	  }
	  else
	  {
	    psputw(we->word);
	    psprintf("%d %d m", we->x, we->y);
	    free(we->word);
	    free(we);
	  }
	}
      }
    vskip_enabled = 0;
    if (halfpage == 0)
      {
	pscomment("End of Page %d", page_number);
	psf_pushfont(PSF_helvetica);
	psf_pushscale(8);
	psprintf("restore save /Helvetica findfont 8 scalefont setfont");
	psprintf("(-  Page %d  -) dup stringwidth pop 2 div neg 288 add %d m",
		 page_number, MARGIN-15);
	psf_pop();
	psf_pop();
	psprintf("sp");
	current_page ++;
	page_number ++;
	marks_this_page = 0;
	psf_setfont();
      }
    y = TOP - ps_fontsize;
  }
}

static void lb()
{
  if (!in_word_ws)
    word_ws();
  if (need_lb || bol_size < ps_fontsize)
  {
    page_flushing = 0;
    check_eop();
    need_lb = 0;
    y -= ps_fontsize - bol_size;
    bol_size = ps_fontsize;
    x = prevailing_indent;
  }
}

static void ls()
{
  lb();
  y -= ps_fontsize;
}

static void w(const char *s)
{
  WordCache *wc;
  Word *we;
  float width = 0;
  int i;
  bol_size = 0;
  for (i=0; s[i]; i++)
    width += font_metrics[(unsigned char)s[i]];
  width *= ps_fontsize;

  if (width + x > RIGHT)
    need_wrap = 1;

  if (!saw_indent)
    para_set_indent(0);

  check_eop();

  for (wc=word_cache; wc; wc=wc->next)
    if (wc->font == ps_font && wc->size == ps_fontsize)
      break;
  if (!wc)
  {
    wc = (WordCache *)malloc(sizeof (WordCache));
    wc->font = ps_font;
    wc->size = ps_fontsize;
    wc->words = 0;
    wc->next = word_cache;
    word_cache = wc;
  }
  we = (Word *)malloc(sizeof(Word));
  we->next = wc->words;
  wc->words = we;
  we->vskip = vskip_enabled;
  we->pskip = 0;
  we->word = (char *)malloc(strlen(s)+1);
  strcpy(we->word, s);
  we->x = x+MARGIN + halfpage;
  we->y = y+MARGIN + baseline_offset;
  
#if 0
  {
    char psbuf[100];
    psputw(s);
    sprintf(psbuf, "%d %d m", x+MARGIN, y+MARGIN);
    psprintf(psbuf);
  }
#endif

  need_lb = 1;
  bol_size = 0;
  x += width;
  marks_this_page = 1;
}

static void n()
{
  if (need_lb)
    x += font_metrics['n'] * ps_fontsize;
}

static void m()
{
  if (need_lb)
    x += font_metrics['M'] * ps_fontsize;
}

void line_break(void)
{
  if (at_bol && bol_size >= ps_fontsize)
    return;
  lb();
  at_bol = 1;
  ps_col = 0;
  pending_whitespace = 0;
}

void line_skip(void)
{
  line_break();
  ls();
}

void word_add_char(int c)
{
  word_buf[wp++] = c;
  if (c == '.' || c == '?' || c == '!')
    ends_sentence = 1;
  else
    ends_sentence = 0;
}

void word_add_string(char *s)
{
  while (*s)
    word_add_char(*s++);
}

void word_add_quoted(int c)
{
  word_buf[wp++] = c;
  ends_sentence = 0;
}

void word_emit(void)
{
  if (wp == 0)
    return;
  para_open = 1;
  word_buf[wp] = 0;
  w(word_buf);
  pending_whitespace = 1;
  wp = 0;
  at_bol = 0;
}

void word_ws(void)
{
  if (need_wrap)
  {
    int expect_new_page = 0;
    int dx = prevailing_indent - x_last_ws;
    int dy = -ps_fontsize;
    WordCache *wc;
    Word *we;

    if (y + dy < 0)
    {
      dy = (TOP-ps_fontsize) - y;
      if (two_columns)
	{
	  if (halfpage)
	    dx -= halfpage;
	  else
	    dx += 612 / 2 - MARGIN/2;
	}
      expect_new_page = 1;
    }

    for (wc=word_cache; wc; wc=wc->next)
      for (we=wc->words; we; we=we->next)
      {
	if (we->x-MARGIN-halfpage >= x_last_ws
	    && we->y-MARGIN <= y)
	{
	  we->x += dx;
	  we->y += dy;
	  we->pskip = expect_new_page;
	}
      }
    need_wrap = 0;
    dx = x - x_last_ws;
    in_word_ws++;
    lb();
    in_word_ws--;
    x = dx + prevailing_indent;
    pending_whitespace = need_lb = 1;
    bol_size = 0;
    marks_this_page = 1;
  }
  if (!pending_whitespace)
    return;
  pending_whitespace = 0;
  if (ends_sentence)
    m();
  else
    n();
  x_last_ws = x;
}

void
word_symbol(int sym)
{
  word_emit();
  psf_pushfont(PSF_symbol);
  word_add_char(sym);
  word_emit();
  psf_pop();
}

void
page_flush(void)
{
  if (!marks_this_page)
    return;
  page_flushing = 1;
  check_eop();
}

void
page_flush_final(void)
{
  if (!marks_this_page)
    return;
  halfpage = 1;
  page_flushing = 1;
  check_eop();
}

void
word_adjust_baseline(int pts)
{
  baseline_offset += pts;
}
