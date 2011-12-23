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
#include <ctype.h>
#include <time.h>
#include <sys/cdefs.h>

#include "fileio.h"
#include "screenio.h"
#include "word.h"
#include "ps.h"
#include "ifset.h"

int suspend_output = 0;
int bye = 0;

static char buf[1000];
static int bufp;
static int capify=0;
static int eat_skips = 0;
static int fill_mode;
static int show_menus = 0;

typedef void FUNCTION ();	/* So I can say FUNCTION *foo; */

/* The list of commands that we hack in texinfo.  Each one
   has an associated function.  When the command is encountered in the
   text, the associated function is called with START as the argument.
   If the function expects arguments in braces, it remembers itself on
   the stack.  When the corresponding close brace is encountered, the
   function is called with END as the argument. */

#define START 0
#define END 1

typedef struct brace_element
{
  struct brace_element *prev;
  FUNCTION *proc;
  char *name;
  int flags;
  int index;
} BRACE_ELEMENT;

#define TABLE_LINE_BREAK	1
#define ENUM_LETTER		2
#define FILL_MODE		2

BRACE_ELEMENT *brace_stack = 0;
BRACE_ELEMENT *table_stack = 0;

void remember_brace_args(FUNCTION func, char *name)
{
  BRACE_ELEMENT *be = (BRACE_ELEMENT *)malloc(sizeof(BRACE_ELEMENT));
  be->proc = func;
  be->name = name ? strdup(name) : 0;
  be->prev = brace_stack;
  brace_stack = be;
}

void forget_brace_args()
{
  if (brace_stack)
  {
    BRACE_ELEMENT *be = brace_stack;
/*    printf("\033[31m[---@%s(END)---]\033[37m", be->name); */
    if (brace_stack->proc)
      brace_stack->proc(END);
    brace_stack = brace_stack->prev;
    if (be->name) free(be->name);
    free(be);
  }
}

void remember_table_cmd(FUNCTION func, char *name)
{
  BRACE_ELEMENT *be = (BRACE_ELEMENT *)malloc(sizeof(BRACE_ELEMENT));
  be->proc = func;
  be->name = name ? strdup(name) : 0;
  be->prev = table_stack;
  be->flags = 0;
  table_stack = be;
  para_set_prevailing_indent(ps_fontsize*3);
  para_set_indent(0);
}

void forget_table_cmd()
{
  if (table_stack)
  {
    BRACE_ELEMENT *be = table_stack;
    table_stack = table_stack->prev;
    if (be->name) free(be->name);
    free(be);
    para_set_prevailing_indent(-ps_fontsize*3);
  }
}

typedef struct
{
  char *name;
  FUNCTION *proc;
  int argument_in_braces;
} COMMAND;

static COMMAND CommandTable[];


static FUNCTION *lookup_command(char *name)
{
  int cmd;
  for (cmd=0; name[cmd]; cmd++)
    if (name[cmd] == '{')
    {
      name[cmd] = 0;
      break;
    }
  for (cmd=0; CommandTable[cmd].name; cmd++)
    if (strcmp(CommandTable[cmd].name, name) == 0)
    {
      return CommandTable[cmd].proc;
    }
  return 0;
}

#define NO_BRACE_ARGS 0
#define BRACE_ARGS 1
#define LINE_ARGS 2
#define WORD_ARGS 3
#define BRACE_WORD_ARGS 4
#define SELF_ARGS 5

void cm_example();
void cm_samp();

void
big_font(int se, char *arg, int szup)
{
  if (se == START)
  {
    word_emit();
    fileio_queue("}");
    fileio_queue(arg);
    remember_brace_args(big_font, "font_change");
    psf_pushscale(ps_fontsize+szup);
    line_break();
  }
  else
  {
    line_break();
    psf_pop();
    eat_skips = 1;
  }
}

void
cm_TeX(int se)
{
  if (se == START && !suspend_output)
  {
    word_emit();
    psf_pushfont(PSF_symbol);
    word_add_char(0124);
    word_emit();
    word_adjust_baseline(-ps_fontsize/4);
    word_add_char(0105);
    word_emit();
    word_adjust_baseline(ps_fontsize/4);
    word_add_char(0103);
    word_emit();
    psf_pop();
  }
}

void
cm_appendix(int se, char *arg)
{
  if (se == START)
    page_flush();
  big_font(se, arg, 8);
}

void cm_appendixsec(int se, char *arg) { big_font(se, arg, 6); }
void cm_appendixsubsec(int se, char *arg) { big_font(se, arg, 4); }
void cm_appendixsubsubsec(int se, char *arg) { big_font(se, arg, 2); }
void cm_asis(){}
     
void
cm_asterisk()
{
  line_break();
}

void
cm_author(int se, char *args)
{
  if (suspend_output)
    return;
  line_skip();
  line_skip();
  line_skip();
  if (se == START)
  {
    word_emit();
    psf_pushset(PSF_italic);
    word_add_string(args);
    word_emit();
    psf_pop();
  }
}
     
void
cm_bold(int se)
{
  if (se == START)
    psf_pushset(PSF_bold);
  else
    psf_pop();
}

void
cm_br(int se)
{
  if (se == START)
    para_close();
}

void
cm_bullet(int se)
{
  if (se == START)
  {
    if (!suspend_output)
      word_add_char(0267);
  }
}

void
cm_bye()
{
  page_flush_final();
  bye = 1;
}

void cm_cartouche(int se, char *arg) { cm_example(se, arg); }
void cm_center(){}
void
cm_chapheading(int se, char *arg)
{
  big_font(se, arg, 8);
}

void
cm_chapter(int se, char *arg)
{
  if (se == START)
    page_flush();
  big_font(se, arg, 8);
}

void cm_cindex(){}
void cm_cite(){}

void
cm_clear(int se, char *arg)
{
  char *p;

  if (se == END)
    fprintf(stderr, "\ncm_clear(END, %s) at %s", arg, fileio_where());
  else if (suspend_output)
    return;

  /* The first word is the flag name.  */
  p = take_name(arg);

  if (p == arg)
    {
      fprintf(stderr, "\n@clear with an empty name at %s", fileio_where());
      return;
    }

  *p = '\0';

  clear_flag(arg);
}

void
cm_code(int se)
{
  if (se == START)
    psf_pushfont(PSF_courier);
  else
    psf_pop();
}

void
cm_copyright(int se)
{
  if (suspend_output)
    return;
  if (se == START)
    word_symbol(0323);
}

void
cm_ctrl(int se, char *args)
{
  if (suspend_output)
    return;
  if (se == START)
  {
    word_emit();
    psf_pushfont(PSF_courier);
    word_add_char('^');
    word_add_string(args);
    word_emit();
    psf_pop();
  }
}

void cm_defcodeindex(){}
void cm_defindex(){}
void cm_defun(){}
void cm_dfn(){}
void cm_display(){}

void
cm_do_cond(int se)
{
}

void
cm_dots(int se)
{
  if (suspend_output)
    return;
  if (se == START)
    word_symbol(0274);
}

void
cm_emph(int se, char *args)
{
  if (se == START)
  {
    psf_pushset(PSF_italic);
  }
  else
  {
    psf_pop();
  }
}

void
cm_end(int se, char *w)
{
  FUNCTION *cmd = lookup_command(w);
  if (cmd)
    cmd(END);
}

void
helper_enumerate(int se)
{
  if (suspend_output)
    return;
  if (se == START)
  {
    char buf[10];
    if (table_stack->flags & ENUM_LETTER)
      sprintf(buf, "%c", table_stack->index);
    else
      sprintf(buf, "%d.", table_stack->index);
    table_stack->index ++;
    word_add_string(buf);
    word_emit();
    word_ws();
  }
}

void
cm_enumerate(int se, char *arg)
{
  if (se == START)
  {
    remember_table_cmd(helper_enumerate, "enumerate");
    if (arg && arg[0])
    {
      if (isdigit(arg[0]))
        table_stack->index = atoi(arg);
      else
      {
        table_stack->index = arg[0];
	table_stack->flags |= ENUM_LETTER;
      }
    }
    else
      table_stack->index = 1;
  }
  else
  {
    line_break();
    forget_table_cmd();
  }
}

void
cm_equiv(int se)
{
  if (suspend_output)
    return;
  if (se == START)
    word_symbol(0272);
}

void
cm_error(int se)
{
  if (suspend_output)
    return;
  if (se == START)
  {
    word_add_string("error");
    word_symbol(0256);
  }
}

void
cm_example(int se)
{
  if (se == START)
  {
    remember_table_cmd(0,0);
    if (fill_mode)
      table_stack->flags |= FILL_MODE;
    fill_mode = 0;
    psf_pushfont(PSF_courier);
  }
  else
  {
    psf_pop();
    fill_mode = (table_stack->flags & FILL_MODE) ? 1 : 0;
    forget_table_cmd();
  }
}

void
cm_exdent(int se)
{
  if (se == START)
    para_set_indent(-prevailing_indent);
}
     
void
cm_expansion(int se)
{
  if (suspend_output)
    return;
  if (se == START)
    word_symbol(0336);
}

void
cm_file(int se, char *args)
{
  cm_samp(se, args);
}

void cm_findex(){}
void cm_flushleft(){}
void cm_flushright(){}

void cm_italic();
void
cm_footnote(int se)
{
  cm_italic(se);
}

void cm_footnotestyle(){}
void cm_force_abbreviated_whitespace(){}

void
cm_format(int se, char *args)
{
  if (se == 0)
  {
    remember_table_cmd(cm_format, "format");
    if (fill_mode)
      table_stack->flags |= FILL_MODE;
  }
  else
  {
    fill_mode = (table_stack->flags & FILL_MODE) ? 1 : 0;
    forget_table_cmd();
  }
}

void cm_table();
void
cm_ftable(int se, char *arg)
{
  cm_table(se, arg);
}
void cm_group(){}
void cm_heading(int se, char *arg) { big_font(se, arg, 6); }

static int set_level, clear_level;

void
cm_ifclear(int se, char *arg)
{
  if (se == START)
  {
    clear_level++;
    suspend_output += ifset(arg);
  }
  else if (clear_level > 0)
  {
    clear_level--;
    if (suspend_output > 0)
      suspend_output--;
  }
  else
  {
    fprintf(stderr, "\nunmatched @end ifclear at %s", fileio_where());
    clear_level = 0;
  }
}

void
cm_ifset(int se, char *arg)
{
  if (se == START)
  {
    set_level++;
    suspend_output += !ifset(arg);
  }
  else if (set_level > 0)
  {
    set_level--;
    if (suspend_output > 0)
      suspend_output--;
  }
  else
  {
    fprintf(stderr, "\nunmatched @end ifset at %s", fileio_where());
    set_level = 0;
  }
}

void
cm_ignore_cond(int se)
{
  if (se == START)
    suspend_output ++;
  else if (suspend_output)
    suspend_output --;
  else
    fprintf(stderr, "\nunmatched @end XXX at %s", fileio_where());
}

void cm_ignore_line(){}

void
cm_ignore_sentence_ender(int se, char *arg)
{
  if (suspend_output)
    return;
  if (se == START)
    word_add_quoted(arg[0]);
}

void cm_include(int se, char *args) { fileio_include(args); }
void cm_infoinclude(int se, char *args) { fileio_include(args); }
void cm_inforef(){}

void
cm_italic(int se)
{
  if (se == START)
    psf_pushset(PSF_italic);
  else
    psf_pop();
}

void
cm_item(int se, char *s)
{
  static int nesting=0;
  if (nesting)
    return;
  nesting++;
  if (table_stack)
  {
    line_break();
    para_set_indent(-ps_fontsize*2);
    if (table_stack->flags & TABLE_LINE_BREAK)
      fileio_queue("@*");
    fileio_queue("}");
    fileio_queue(s);
    remember_brace_args(table_stack->proc, table_stack->name);
    if (table_stack->proc)
      table_stack->proc(START, "");
    word_emit();
    if (!(table_stack->flags & TABLE_LINE_BREAK))
      para_set_indent(0);
  }
  else
    fprintf(stderr, "@item without table_stack at %s\n", fileio_where());
  nesting--;
}

void
cm_itemize(int se, char *arg)
{
  if (se == START)
  {
    arg++;
    remember_table_cmd(lookup_command(arg), arg);
  }
  else
  {
    line_break();
    forget_table_cmd();
  }
}

void
cm_itemx(int se, char *arg)
{
  cm_item(se, arg);
}

void
cm_kbd(int se)
{
  if (se == START)
    psf_pushfont(PSF_helvetica);
  else
    psf_pop();
}
void cm_key(int se) { cm_kbd(se); }
void cm_kindex(){}
void cm_lisp(){}
void cm_lowersections(){}
void cm_macro(){}

void
cm_majorheading(int se, char *arg)
{
  big_font(se, arg, 8);
}

void cm_math(){}

void
cm_menu(int se)
{
  if (se == START)
  {
    if (!show_menus)
      suspend_output ++;
    remember_table_cmd(0,0);
    if (fill_mode)
      table_stack->flags |= FILL_MODE;
    fill_mode = 0;
  }
  else
  {
    fill_mode = (table_stack->flags & FILL_MODE) ? 1 : 0;
    forget_table_cmd();
    if (!show_menus)
      suspend_output --;
  }
}

void
cm_minus(int se)
{
  if (suspend_output)
    return;
  if (se == START)
    word_symbol(0055);
}

void
cm_node(int se, char *args)
{
  sscanf(args, "%[^,\n]", args);
  screenio_note("%3d %s", current_page, args);
}

void cm_noindent(){}

void
cm_page(int se)
{
  if (se == START)
    page_flush();
}

void cm_paragraphindent(){}
void cm_pindex(){}
void cm_point(){}
void cm_print(){}
void cm_printindex(){}
void cm_xref();
void
cm_pxref(int se, char *a)
{
  cm_xref(se, a);
}
void cm_quotation(){}
void cm_raisesections(){}
void cm_refill(){}

void
cm_result(int se)
{
  if (suspend_output)
    return;
  if (se == START)
    word_symbol(0336);
}

void
cm_roman(int se, char *arg)
{
  if (se == START)
    psf_pushfont(PSF_times);
  else
    psf_pop();
}

void
cm_samp(int se)
{
  cm_code(se);
}

void
cm_sc(int se)
{
  if (se == START)
  {
    psf_pushscale(ps_fontsize * 7 / 8);
    capify++;
  }
  else
  {
    capify--;
    psf_pop();
  }
}

void cm_section(int se, char *arg) { big_font(se, arg, 6); }

void
cm_set(int se, char *arg)
{
  char *p;

  if (se == END)
    fprintf(stderr, "\ncm_set(END, %s) at %s", arg, fileio_where());
  else if (suspend_output)
    return;

  /* The first word is the flag name, the rest, if any, its value.  */
  p = take_name(arg);

  if (p == arg)
    {
      fprintf(stderr, "\n@set with an empty name at %s", fileio_where());
      return;
    }

  if (*p)
    {
      /* Possible value after name.  It is the remainder of the line
         sans the whitespace.  */
      char *v = p;

      *v++ = '\0';
      while (*v == ' ' || *v == '\t')
        v++;
      p = v;
    }

  set_flag(arg, p);
}

void
cm_setfilename(){}

void
cm_smallexample(int se)
{
  cm_example(se);
}

void cm_smalllisp(){}

void
cm_sp(int se, char *a)
{
  if (se == START)
  {
    int l = atoi(a);
    word_emit();
    line_break();
    while (l--)
      line_skip();
  }
}

void cm_strong(int se)
{
  if (se == START)
    psf_pushset(PSF_bold);
  else
    psf_pop();
}

void cm_subheading(int se, char *arg) { big_font(se, arg, 4); }
void cm_subsection(int se, char *arg) { big_font(se, arg, 4); }
void cm_subsubheading(int se, char *arg) { big_font(se, arg, 2); }
void cm_subsubsection(int se, char *arg) { big_font(se, arg, 2); }
void cm_subtitle(int se, char *a) { big_font(se, a, 10); }
void cm_synindex(){}

void
cm_table(int se, char *arg)
{
  if (se == START)
  {
    arg++;
    remember_table_cmd(lookup_command(arg), arg);
    table_stack->flags = TABLE_LINE_BREAK;
  }
  else
  {
    line_break();
    forget_table_cmd();
  }
}

void cm_tindex(){}

void
cm_title(int se, char *arg)
{
  if (se == START)
  {
    word_emit();
    fileio_queue("}");
    fileio_queue(arg);
    remember_brace_args(cm_title, "title");
    psf_pushscale(ps_fontsize+18);
    line_skip();
    line_skip();
    line_skip();
    line_skip();
  }
  else
  {
    line_break();
    psf_pop();
    eat_skips = 1;
    line_skip();
    line_skip();
    line_skip();
    line_skip();
  }
}

void
cm_titlefont(int se)
{
  if (se == START)
    psf_pushscale(ps_fontsize * 2 + 18);
  else
  {
    para_close();
    psf_pop();
  }
}

void
cm_titlepage(int se)
{
  if (se == START)
    remember_brace_args(cm_titlepage, "titlepage");
  else
    page_flush();
}

void
cm_today(int se)
{
  if (suspend_output)
    return;
  if (se == START)
  {
    time_t now;
    struct tm *tmnow;
    char buf[100];
    time(&now);
    tmnow = localtime(&now);
    strftime(buf, 100, "%e %h %Y", tmnow);
    word_add_string(buf);
  }
}

void cm_top(){}
void cm_unmacro(){}

void
cm_unnumbered(int se, char *arg)
{
  if (se == START)
    page_flush();
  big_font(se, arg, 8);
}

void cm_unnumberedsec(int se, char *arg) { big_font(se, arg, 6); }
void cm_unnumberedsubsec(int se, char *arg) { big_font(se, arg, 4); }
void cm_unnumberedsubsubsec(int se, char *arg) { big_font(se, arg, 2); }

void
cm_value(int se, char *flag)
{
  if (suspend_output)
    return;
  if (se == START)
    {
      char *value = flag_value(flag);

      if (value)
        {
          word_add_string(value);
          free(value);
        }
      else
        {
          fprintf(stderr, "\nError: memory exhausted at %s", fileio_where());
          exit(1);
        }
    }
  else
    fprintf(stderr, "\ncm_value(END, %s) at %s", flag, fileio_where());
}

void cm_var(){}
void cm_vindex(){}

void
cm_vskip()
{
  vskip_enabled = 1;
}

void
cm_vtable(int se, char *arg)
{
  cm_table(se, arg);
}

void cm_w(){}

void
cm_xref(int se, char *a)
{
  if (suspend_output)
    return;
  if (se == START)
  {
    word_add_string("See");
    word_emit();
    word_ws();
    word_add_string(a);
  }
}

void command_name_condition(){}
void do_nothing(){}

void
insert_self(int se, char *a)
{
  if (suspend_output)
    return;
  if (se == START)
    word_add_quoted(a[0]);
}


static COMMAND CommandTable[] = {
  { "!", cm_ignore_sentence_ender, NO_BRACE_ARGS },
  { "'", insert_self, SELF_ARGS },
  { "*", cm_asterisk, NO_BRACE_ARGS },
  { ".", cm_ignore_sentence_ender, NO_BRACE_ARGS },
  { ":", cm_force_abbreviated_whitespace, NO_BRACE_ARGS },
  { "?", cm_ignore_sentence_ender, NO_BRACE_ARGS },
  { "|", do_nothing, NO_BRACE_ARGS },
  { "@", insert_self, SELF_ARGS },
  { " ", insert_self, SELF_ARGS },
  { "\n", insert_self, SELF_ARGS },
  { "TeX", cm_TeX, BRACE_ARGS },
  { "`", insert_self, SELF_ARGS },
  { "appendix", cm_appendix, LINE_ARGS },
  { "appendixsection", cm_appendixsec, LINE_ARGS },
  { "appendixsec", cm_appendixsec, LINE_ARGS },
  { "appendixsubsec", cm_appendixsubsec, LINE_ARGS },
  { "appendixsubsubsec", cm_appendixsubsubsec, LINE_ARGS },
  { "asis", cm_asis, BRACE_ARGS },
  { "author", cm_author, LINE_ARGS },
  { "b", cm_bold, BRACE_ARGS },
  { "bold", cm_bold, BRACE_ARGS },
  { "br", cm_br, BRACE_ARGS },
  { "bullet", cm_bullet, BRACE_ARGS },
  { "bye", cm_bye, NO_BRACE_ARGS },
  { "c", cm_ignore_line, LINE_ARGS },
  { "cartouche", cm_cartouche, NO_BRACE_ARGS },
  { "center", cm_center, NO_BRACE_ARGS },
  { "chapheading", cm_chapheading, NO_BRACE_ARGS },
  { "chapter", cm_chapter, LINE_ARGS },
  { "cindex", cm_cindex, LINE_ARGS },
  { "cite", cm_cite, BRACE_ARGS },
  { "clear", cm_clear, LINE_ARGS },
  { "code", cm_code, BRACE_ARGS },
  { "comment", cm_ignore_line, LINE_ARGS },
  { "contents", do_nothing, NO_BRACE_ARGS },
  { "copyright", cm_copyright, BRACE_ARGS },
  { "ctrl", cm_ctrl, BRACE_ARGS },
  { "defcodeindex", cm_defcodeindex, NO_BRACE_ARGS },
  { "defindex", cm_defindex, NO_BRACE_ARGS },
  { "dfn", cm_dfn, BRACE_ARGS },

/* The `def' commands. */
  { "deffn", cm_defun, LINE_ARGS },
  { "deffnx", cm_defun, LINE_ARGS },
  { "defun", cm_defun, LINE_ARGS },
  { "defunx", cm_defun, LINE_ARGS },
  { "defmac", cm_defun, LINE_ARGS },
  { "defmacx", cm_defun, LINE_ARGS },
  { "defspec", cm_defun, LINE_ARGS },
  { "defspecx", cm_defun, LINE_ARGS },
  { "defvr", cm_defun, LINE_ARGS },
  { "defvrx", cm_defun, LINE_ARGS },
  { "defvar", cm_defun, LINE_ARGS },
  { "defvarx", cm_defun, LINE_ARGS },
  { "defopt", cm_defun, LINE_ARGS },
  { "defoptx", cm_defun, LINE_ARGS },
  { "deftypefn", cm_defun, LINE_ARGS },
  { "deftypefnx", cm_defun, LINE_ARGS },
  { "deftypefun", cm_defun, LINE_ARGS },
  { "deftypefunx", cm_defun, LINE_ARGS },
  { "deftypevr", cm_defun, LINE_ARGS },
  { "deftypevrx", cm_defun, LINE_ARGS },
  { "deftypevar", cm_defun, LINE_ARGS },
  { "deftypevarx", cm_defun, LINE_ARGS },
  { "defcv", cm_defun, LINE_ARGS },
  { "defcvx", cm_defun, LINE_ARGS },
  { "defivar", cm_defun, LINE_ARGS },
  { "defivarx", cm_defun, LINE_ARGS },
  { "defop", cm_defun, LINE_ARGS },
  { "defopx", cm_defun, LINE_ARGS },
  { "defmethod", cm_defun, LINE_ARGS },
  { "defmethodx", cm_defun, LINE_ARGS },
  { "deftypemethod", cm_defun, LINE_ARGS },
  { "deftypemethodx", cm_defun, LINE_ARGS },
  { "deftp", cm_defun, LINE_ARGS },
  { "deftpx", cm_defun, LINE_ARGS },
/* The end of the `def' commands. */

  { "display", cm_display, NO_BRACE_ARGS },
  { "dots", cm_dots, BRACE_ARGS },
  { "dmn", do_nothing, BRACE_ARGS },
  { "emph", cm_emph, BRACE_ARGS },
  { "end", cm_end, WORD_ARGS },
  { "enumerate", cm_enumerate, LINE_ARGS },
  { "equiv", cm_equiv, BRACE_ARGS },
  { "error", cm_error, BRACE_ARGS },
  { "example", cm_example, LINE_ARGS },
  { "exdent", cm_exdent, NO_BRACE_ARGS },
  { "expansion", cm_expansion, BRACE_ARGS },
  { "file", cm_file, BRACE_ARGS },
  { "findex", cm_findex, LINE_ARGS },
  { "finalout", do_nothing, NO_BRACE_ARGS },
  { "flushleft", cm_flushleft, NO_BRACE_ARGS },
  { "flushright", cm_flushright, NO_BRACE_ARGS },
  { "format", cm_format, NO_BRACE_ARGS },
  { "ftable", cm_ftable, WORD_ARGS },
  { "group", cm_group, NO_BRACE_ARGS },
  { "heading", cm_heading, LINE_ARGS },
  { "headings", cm_ignore_line, LINE_ARGS },
  { "i", cm_italic, BRACE_ARGS },
  { "iappendix", cm_appendix, LINE_ARGS },
  { "iappendixsection", cm_appendixsec, LINE_ARGS },
  { "iappendixsec", cm_appendixsec, LINE_ARGS },
  { "iappendixsubsec", cm_appendixsubsec, LINE_ARGS },
  { "iappendixsubsubsec", cm_appendixsubsubsec, LINE_ARGS },
  { "ichapter", cm_chapter, LINE_ARGS },
  { "ifclear", cm_ifclear, LINE_ARGS },
  { "ifinfo", cm_ignore_cond, NO_BRACE_ARGS },
  { "ifset", cm_ifset, LINE_ARGS },
  { "iftex", cm_ignore_cond, NO_BRACE_ARGS },
  { "ignore", cm_ignore_cond, NO_BRACE_ARGS },
  { "include", cm_include, WORD_ARGS },
  { "inforef", cm_inforef, BRACE_ARGS },
  { "input", cm_include, NO_BRACE_ARGS },
  { "isection", cm_section, LINE_ARGS },
  { "isubsection", cm_subsection, LINE_ARGS },
  { "isubsubsection", cm_subsubsection, LINE_ARGS },
  { "italic", cm_italic, BRACE_ARGS },
  { "item", cm_item, LINE_ARGS },
  { "itemize", cm_itemize, WORD_ARGS },
  { "itemx", cm_itemx, LINE_ARGS },
  { "iunnumbered", cm_unnumbered, LINE_ARGS },
  { "iunnumberedsec", cm_unnumberedsec, LINE_ARGS },
  { "iunnumberedsubsec", cm_unnumberedsubsec, LINE_ARGS },
  { "iunnumberedsubsubsec", cm_unnumberedsubsubsec, LINE_ARGS },
  { "kbd", cm_kbd, BRACE_ARGS },
  { "key", cm_key, BRACE_ARGS },
  { "kindex", cm_kindex, LINE_ARGS },
  { "lowersections", cm_lowersections, NO_BRACE_ARGS },
  { "lisp", cm_lisp, NO_BRACE_ARGS },
  { "macro", cm_macro, NO_BRACE_ARGS },
  { "majorheading", cm_majorheading, LINE_ARGS },
  { "math", cm_math, BRACE_ARGS },
  { "menu", cm_menu, LINE_ARGS },
  { "minus", cm_minus, BRACE_ARGS },
  { "need", cm_ignore_line, WORD_ARGS },
  { "node", cm_node, LINE_ARGS },
  { "noindent", cm_noindent, NO_BRACE_ARGS },
  { "nwnode", cm_node, NO_BRACE_ARGS },
  { "overfullrule", cm_ignore_line, LINE_ARGS },
  { "page", cm_page, NO_BRACE_ARGS },
  { "pindex", cm_pindex, LINE_ARGS },
  { "point", cm_point, BRACE_ARGS },
  { "print", cm_print, BRACE_ARGS },
  { "printindex", cm_printindex, LINE_ARGS },
  { "pxref", cm_pxref, BRACE_ARGS },
  { "quotation", cm_quotation, NO_BRACE_ARGS },
  { "r", cm_roman, BRACE_ARGS },
  { "raisesections", cm_raisesections, NO_BRACE_ARGS },
  { "ref", cm_xref, BRACE_ARGS },
  { "refill", cm_refill, NO_BRACE_ARGS },
  { "result", cm_result, BRACE_ARGS },
  { "samp", cm_samp, BRACE_ARGS },
  { "sc", cm_sc, BRACE_ARGS },
  { "section", cm_section, LINE_ARGS },
  { "set", cm_set, LINE_ARGS },
  { "setchapternewpage", cm_ignore_line, WORD_ARGS },
  { "setchapterstyle", cm_ignore_line, WORD_ARGS },
  { "setfilename", cm_setfilename, WORD_ARGS },
  { "settitle", cm_ignore_line, LINE_ARGS },
  { "shortcontents", do_nothing, LINE_ARGS },
  { "shorttitlepage", command_name_condition, LINE_ARGS },
  { "smallbook", cm_ignore_line, LINE_ARGS },
  { "smallexample", cm_smallexample, NO_BRACE_ARGS },
  { "smalllisp", cm_smalllisp, NO_BRACE_ARGS },
  { "sp", cm_sp, WORD_ARGS },
  { "strong", cm_strong, BRACE_ARGS },
  { "subheading", cm_subheading, LINE_ARGS },
  { "subsection", cm_subsection, LINE_ARGS },
  { "subsubheading", cm_subsubheading, LINE_ARGS },
  { "subsubsection", cm_subsubsection, LINE_ARGS },
  { "subtitle", cm_subtitle, LINE_ARGS },
  { "summarycontents", do_nothing, NO_BRACE_ARGS },
  { "syncodeindex", cm_synindex, LINE_ARGS },
  { "synindex", cm_synindex, LINE_ARGS },
  { "t", cm_title, LINE_ARGS },
  { "table", cm_table, WORD_ARGS },
  { "tex", cm_ignore_cond, NO_BRACE_ARGS },
  { "tindex", cm_tindex, LINE_ARGS },
  { "title", cm_title, LINE_ARGS },
  { "titlefont", cm_titlefont, BRACE_ARGS },
  { "titlepage", cm_titlepage, NO_BRACE_ARGS },
  { "titlespec", command_name_condition, NO_BRACE_ARGS },
  { "today", cm_today, BRACE_ARGS },
  { "top", cm_top, NO_BRACE_ARGS  },
  { "unmacro", cm_unmacro, NO_BRACE_ARGS },
  { "unnumbered", cm_unnumbered, LINE_ARGS },
  { "unnumberedsec", cm_unnumberedsec, LINE_ARGS },
  { "unnumberedsubsec", cm_unnumberedsubsec, LINE_ARGS },
  { "unnumberedsubsubsec", cm_unnumberedsubsubsec, LINE_ARGS },
  { "value", cm_value, BRACE_WORD_ARGS },
  { "var", cm_var, BRACE_ARGS },
  { "vindex", cm_vindex, LINE_ARGS },
  { "vskip", cm_vskip, LINE_ARGS },
  { "vtable", cm_vtable, WORD_ARGS },
  { "w", cm_w, BRACE_ARGS },
  { "xref", cm_xref, BRACE_ARGS },
  { "{", insert_self, SELF_ARGS },
  { "}", insert_self, SELF_ARGS },

  /* Now @include does what this was supposed to. */
  { "infoinclude", cm_infoinclude, WORD_ARGS },
  { "footnote", cm_footnote, BRACE_ARGS},
  { "footnotestyle", cm_footnotestyle, WORD_ARGS },
  { "paragraphindent", cm_paragraphindent, WORD_ARGS },
    
  {(char *) NULL, (FUNCTION *) NULL, NO_BRACE_ARGS}
  };

void
skip_until_setfilename()
{
  static const char str[] = "@setfilename";
  int match = 0;
  while (1)
  {
    int ch = fileio_get();
    if (ch == EOF)
      break;
    if (ch == str[match])
      match++;
    else
      match = 0;
    if (str[match] == 0)
    {
      while (match)
	fileio_unget(str[--match]);
      return;
    }
  }
  fprintf(stderr, "error: no @setfilename\n");
  exit(1);
}

void
do_file(char *file_name)
{
  int ch, last_ch=-1;
  fill_mode = 1;
  fileio_include(file_name);
  skip_until_setfilename();
  bye = 0;
  while ((ch = fileio_get()) != EOF && !bye)
  {
    if (ch == '\n')
    {
      if (suspend_output)
	continue;
      word_emit();
      if (fill_mode)
      {
	if (last_ch == '\n')
	{
	  if (fill_mode && !eat_skips)
	    para_close();
	}
	else
	  word_ws();
      }
      else
      {
	if (last_ch == '\n')
	  line_skip();
	else
	  line_break();
      }
    }
    else if (ch == ' ' || ch == '\t')
    {
      if (suspend_output)
	continue;
      word_emit();
      word_ws();
    }
    else if (ch == '}')
    {
      if (!suspend_output)
	word_emit();
      forget_brace_args();
    }
    else if (ch == '@')
    {
      char *cmd_name;
      int cmd, argt, a _ATTRIBUTE(__unused__);
      word_emit();
      eat_skips = 0;
      bufp = 0;
      ch = fileio_get();
      buf[bufp++] = ch;
      if (isalnum(ch))
      {
	do {
	  ch = fileio_get();
	  buf[bufp++] = ch;
	} while (ch != '{' && ch > ' ');
	buf[--bufp] = 0;
      }
      else
	buf[bufp] = 0;
      cmd_name = strdup(buf);
      for (cmd=0; CommandTable[cmd].name; cmd++)
	if (strcmp(CommandTable[cmd].name, cmd_name) == 0)
	  break;
      argt = CommandTable[cmd].argument_in_braces;
      if (argt == SELF_ARGS)
      {
	if (!suspend_output)
	  word_add_string(cmd_name);
	continue;
      }
      if (ch == '{' && argt != BRACE_WORD_ARGS)
	argt = BRACE_ARGS;
      bufp = 0;
      switch (argt)
      {
      case NO_BRACE_ARGS:
	a = 'N';
	break;
      case BRACE_ARGS:
	a = 'B';
	remember_brace_args(CommandTable[cmd].proc, cmd_name);
	break;
      case BRACE_WORD_ARGS:
	a = 'R';
	ch = fileio_get();
	while (ch != '}')
	{
	  buf[bufp++] = ch;
	  ch = fileio_get();
	}
	break;
      case LINE_ARGS:
	a = 'L';
	while (ch == ' ' || ch == '\t')
	  ch = fileio_get();
	while (ch != '\n')
	{
	  buf[bufp++] = ch;
	  ch = fileio_get();
	}
	break;
      case WORD_ARGS:
	a = 'W';
	while (ch <= ' ')
	  ch = fileio_get();
	while (ch > ' ')
	{
	  buf[bufp++] = ch;
	  ch = fileio_get();
	}
	break;
      }
      buf[bufp] = 0;
/*      printf("\033[36m[---@%s(%c,\033[0m%s\033[1;36m)---]\033[0;1m", cmd_name, a, buf); */
      if (CommandTable[cmd].proc)
	CommandTable[cmd].proc(START, buf);
      free(cmd_name);
    }
    else
    {
      eat_skips = 0;
      if (!suspend_output)
      {
	if (capify)
	  word_add_char(toupper(ch));
	else
	  word_add_char(ch);
      }
    }
    last_ch = ch;
  }
}

static void
usage(void)
{
  printf("usage: texi2ps [-f size] [-I dir] [-Idir] [-v] [-m pts] [-Dname[=val]] [-Uname] [file...]\n");
  printf("  -f = fontsize (default = 10)\n");
  printf("  -I = include path\n");
  printf("  -v = verbose progress messages\n");
  printf("  -m = set margins (default 54 pts, 3/4\")\n");
  printf("  -D = set flag called `name\' to `val\' (default: empty value)\n");
  printf("  -U = clear (unset) flags called `name\'\n");
  printf("  -2 = print two columns per page\n");
  printf("  -menu = print menus\n");
  exit(0);
}

int
main(int argc, char **argv)
{
  int arg, margin=54;
  if (argc < 2)
    usage();
  for (arg=1; arg<argc && argv[arg][0] == '-'; arg++)
  {
    if (strcmp(argv[arg], "-h") == 0)
    {
      usage();
    }
    else if (strcmp(argv[arg], "-f") == 0)
    {
      int fsize = atoi(argv[++arg]);

      if (fsize > 0)
        ps_fontsize = fsize;
      else
      {
        fprintf(stderr, "illegal value for \"-f\" (ignored)\n");
        --arg;
      }
    }
    else if (strcmp(argv[arg], "-m") == 0)
    {
      int new_margin = atoi(argv[++arg]);

      if (new_margin > 0)
	margin = new_margin;
      else
      {
        fprintf(stderr, "illegal value for \"-m\" (ignored)\n");
        --arg;
      }
    }
    else if (strncmp(argv[arg], "-I", 2) == 0)
    {
      if (argv[arg][2])
	fileio_add_path(argv[arg]+2);
      else
	fileio_add_path(argv[++arg]);
    }
    else if (strcmp(argv[arg], "-v") == 0)
      screenio_enabled = 1;
    else if (strcmp(argv[arg], "-2") == 0)
      two_columns = 1;
    else if (strcmp(argv[arg], "-menu") == 0)
      show_menus = 1;
    else if (strncmp(argv[arg], "-D", 2) == 0 ||
             strncmp(argv[arg], "-U", 2) == 0)
    {
      int   set   = argv[arg][1] == 'D' ? 1 : 0;
      char *name  = argv[arg][2] ? argv[arg] + 2 : argv[++arg];
      char *value = strchr(name, '=');

      if (value)
        *value++ = '\0';
      else
        value = "";

      if (set)
        set_flag(name, value);
      else
        clear_flag(name);
    }
  }
  word_set_margins(margin);
  word_init();
  for (; arg < argc; arg++)
  {
    set_level = clear_level = 0;
    do_file(argv[arg]);
  }
  psdone();
  screenio_print("\n%d pages generated", current_page);
  return 0;
}
