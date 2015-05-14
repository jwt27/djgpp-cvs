/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* ------------------------------------------------------------------------- */
/*			    FULL SCREEN DEBUGGER			     */
/*									     */
/* Copyright 1994 by Morten Welinder, terra@diku.dk			     */
/* ------------------------------------------------------------------------- */
/*

This debugger is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This debugger is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with djgpp; see the file COPYING.	 If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */
/* ------------------------------------------------------------------------- */
#define FULLSCR_VERSION 1.00

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
#include <unistd.h>
#include "ed.h"
#include <debug/syms.h>
#include "unassmbl.h"
#include "screen.h"
/* ------------------------------------------------------------------------- */
/* Information about the initialization state of the debugger.	Actually
   I'm not sure that longjmp should ever be permitted.	*/
int can_longjmp = 0;
jmp_buf debugger_jmpbuf;

/* Information about panes.  */
#define PANECOUNT 16
enum {
  PANE_CODE = 0,
  PANE_REGISTER = 1,
  PANE_FLAG = 2,
  PANE_BREAKPOINT = 3,
  PANE_DATA = 4,
  PANE_NPX = 5,
  PANE_STACK = 6,
  PANE_INFO = 7,
  PANE_WHEREIS = 8,
  PANE_GDT = 9,
  PANE_IDT = 10,
  PANE_LDT = 11,
  PANE_HELP = 12,
  PANE_MODULE = 13,
  PANE_SOURCE = 14,
  PANE_WATCH = 15};

static int pane, ulpane, dlpane, pane_positions[PANECOUNT], pane_pos;
static word32 code_dump_origin, code_dump_last;
static word32 *code_pane_pos;
static int register_pane_origin;
static int flag_pane_origin;
static word32 data_dump_origin, data_dump_last, data_dump_size;
static int npx_pane_origin;
static int stack_dump_origin, stack_dump_last, stack_dump_more;
static word32 *stack_dump_pos;
static int breakpoint_origin;
static char **whereis_pane_text;
static int whereis_text_count, whereis_pane_origin;
static int gdt_pane_origin;
static int idt_pane_origin;
static int ldt_pane_origin;
static int help_pane_origin;
static int module_text_count, module_pane_origin;
static char **module_pane_text;
static int *module_line_numbers;
static int source_text_count, source_pane_origin;
static char **source_pane_text, *source_pane_module;
static int watch_text_count, watch_pane_origin;
static char **watch_pane_text;

static int code_pane_active;
static int register_pane_active;
static int flag_pane_active;
static int breakpoint_pane_active;
static int data_pane_active;
static int npx_pane_active;
static int stack_pane_active;
static int info_pane_active;
static int whereis_pane_active;
static int gdt_pane_active;
static int idt_pane_active;
static int ldt_pane_active;
static int help_pane_active;
static int module_pane_active;
static int source_pane_active;
static int watch_pane_active;

/* Odds and ends.  */
#define MAXINSTLEN 16
static int first_step;
static word32 main_entry;
static void redraw (int);
static const char hexchars[] = "0123456789abcdef";
static int dpmi;
static int terminated;
char *setupfilename;
extern int evaluate (char *, long *, char **);

/* Pseudo-keys.  */
#define PK_Redraw 0x2ff
#define PK_Control_I 0x2fe

#define MIN(x,y) ((x) <= (y) ? (x) : (y))
#define MAX(x,y) ((x) >= (y) ? (x) : (y))
/* ------------------------------------------------------------------------- */
/* Description of which types of selectors that are valid in which contexts.
   Type #i is valid if the i-th bit is set.  */

static word32 allowed_descriptors[] = {
  0xffffdafeL,   /* GDT */
  0x0000c0e0L,   /* IDT */
  0xffffdafaL};  /* LDT */
/* ------------------------------------------------------------------------- */
static char *helptext[] = {
  "Welcome to Sally Full Screen Debugger!",
  "",
  "This program is copyright 1994-1996 by Morten Welinder.",
  "For further copyright information, including other parties'",
  "claims, please read the manual.",
  "",
  "",
  "The following keys are globally defined:",
  "",
  "Alt-C      : Select code (disassembly) pane.",
  "Alt-E      : Evaluate expression.",
  "Alt-G      : Select GDT pane.",
  "Alt-H      : Select help pane.",
  "Alt-I      : Select IDT pane.",
  "Alt-L      : Select LDT pane.",
  "Alt-M      : Select module pane.",
  "Alt-N      : Select numeric processor (fpu) pane.",
  "Alt-S      : Select stack pane.",
  "Alt-W      : Select where-is (symbol list) pane.",
  "Alt-X      : Exit right away.",
  "Tab        : Select next pane.",
  "BackTab    : Select previous pane.",
  "F1         : Select help pane (alias for Alt-H).",
  "C-F4       : Evaluate expression (alias for Alt-E).",
  "Alt-F5     : Show debugged program's screen.",
  "F7         : Execute single instruction.",
  "C-F7       : Add watch.",
  "F8         : Execute till next instruction.",
  "F9         : Run program.",
  "F10        : Select pane via menu.",
  "Alt-F10    : Misc. functions.",
  "\030,\031,\033,\032    : Move focus up/down/left/right one step.",
  "Alt-\030,Alt-\031: Move horizontal split up/down.",
  "PgUp,PgDn  : Move focus up/down one page.",
  "Home,End   : Move focus to beginning/end of pane.",
  "",
  "",
  "",
  "Expressions can use",
  "",
  "* numbers     : Decimal, hex, and octal is supported.",
  "* operators   : More or less the full C set (not counting",
  "                the assignment operators) is supported.",
  "* parentheses : For grouping.",
  "* symbols     : The value is the address of the symbol.  An",
  "                initial underscore may be omitted if that",
  "                name is not used for another symbol.",
  "* registers   : 8-, 16-, and 32-bit registers are supported.",
  "                If there is a symbol with the same name as a",
  "                register then a `%' must be prepended to the",
  "                name.",
  "* `expr:size' : Reduces the operand to `size' bytes.",
  "* `[expr]'    : Evaluates to the 32-bit value at that",
  "                address.",
  "",
  "All evaluation is done with 32-bit signed values, except",
  "shifts which use unsigned values.",
  "",
  "",
  "",
  "Code pane keys:",
  "",
  "F2         : Set breakpoint.",
  "F4         : Run to instruction at focus.",
  "Return     : Go to source.",
  "C-\033,C-\032    : Start disassembly one byte earlier/later.",
  "C-g        : Go to specified address.",
  "C-n        : Set eip to instruction at cursor.",
  "C-o        : Go to origin, i.e., current eip.",
  "",
  "",
  "",
  "Register pane keys:",
  "",
  "C-d        : Decrement register.",
  "C-i        : Increment register.",
  "C-n        : Negate register.",
  "C-z        : Zero register.",
  "=          : Enter new expression for register.",
  "",
  "",
  "",
  "Flag pane keys:",
  "",
  "Space      : Toggle flag.",
  "1,s        : Set flag.",
  "0,r        : Reset flag.",
  "",
  "Only the user flags are listed.  These are:",
  "",
  "Flag   Name         0 means    1 means",
  "-----------------------------------------",
  "c      Carry        none       carry",
  "p      Parity       odd        even",
  "a      Auxiliary    clear      half-carry",
  "z      Zero         nonzero    zero",
  "s      Sign         positive   negative",
  "i      Interrupt    disabled   enabled",
  "d      Direction    down       up",
  "o      Overflow     none       overflow",
  "",
  "",
  "",
  "Breakpoint pane keys:",
  "",
  "Delete     : Delete breakpoint.",
  "Enter      : Edit condition and trigger count.",
  "C-g        : Go to code affected by breakpoint.",
  "",
  "For code breakpoints you can enter a condition and a",
  "count controling when the breakpoint is triggered.",
  "",
  "",
  "",
  "Data pane keys:",
  "",
  "C-\033,C-\032    : Start data display one byte earlier/later.",
  "C-b        : Display data as bytes.",
  "C-c        : Go to the the current instruction.",
  "C-g        : Go to specified address.",
  "C-l        : Display data as 32-bit words (`longs').",
  "C-s        : Go to the stack.",
  "C-w        : Display data as 16-bit words.",
  "F2         : Set data write breakpoint at focus.",
  "Alt-F2     : Set data read/write breakpoint at focus.",
  "=          : Enter new value or string.",
  "",
  "",
  "",
  "Numeric processor pane keys:",
  "",
  "C-e        : Empty register.",
  "C-z        : Zero register.",
  "C-n        : Negate register.",
  "=          : Enter new value for register.",
  "",
  "",
  "",
  "Stack pane keys:",
  "",
  "Enter      : Go to code.",
  "",
  "",
  "",
  "Where-is pane keys:",
  "",
  "Enter      : Go to code/data.",
  "=          : Enter new search expression.",
  "",
  "",
  "",
  "Module pane keys:",
  "",
  "Enter      : Show source for module.",
  "",
  "",
  "",
  "Source pane keys:",
  "",
  "F2         : Set breakpoint.",
  "F4         : Run to instruction at focus.",
  "F8         : Execute till next line or function return.",
  "Enter      : Show code for line.",
  "",
  "",
  "",
  "Watch pane keys:",
  "",
  "Delete     : Delete watch.",
  "=          : Add watch."
};
/* ------------------------------------------------------------------------- */
/* This information guards the display of descriptors.  */

struct {
  word16 limit __attribute__ ((packed));
  word32 base __attribute__ ((packed));
} gdtlinear, idtlinear, ldtlinear;

#ifndef V2DBG
static char *gdtnames[g_num] = {
  "NULL", "gdt", "idt", "rcode",
  "rdata", "pcode", "pdata", "core",
  "bios data", "arena data", "ctss", "active tss",
  "page tss", "int tss", "rc32", "grdr",
  "vcpi code", "(vcpi res. 0)", "(vcpi res. 1)", "vect 0x74",
  "vect 0x78", "vect 0x79", "altc", "debug cs",
  "debug ds", "debug tss", "rtss", "arena code",
  "vesa funcions" };
#endif
/* ------------------------------------------------------------------------- */
/* The presentation order of registers in the register pane.  The three
   tables must, of course, match.  */

static char *regs_names[] = {
  "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp",
  "cs",	 "ds",	"es",  "fs",  "gs",  "ss",
  "eip", "flg" };

static word32 *regs_addr[] = {
  &a_tss.tss_eax, &a_tss.tss_ebx, &a_tss.tss_ecx, &a_tss.tss_edx,
  &a_tss.tss_esi, &a_tss.tss_edi, &a_tss.tss_ebp, &a_tss.tss_esp,
  (word32 *)&a_tss.tss_cs, (word32 *)&a_tss.tss_ds,
  (word32 *)&a_tss.tss_es, (word32 *)&a_tss.tss_fs,
  (word32 *)&a_tss.tss_gs, (word32 *)&a_tss.tss_ss,
  &a_tss.tss_eip, &a_tss.tss_eflags };

/* g: general, !: special, s: segment, f: flags, \0: end-of-table */
static char regs_type[] = "ggggggg!ssssss!f";
/* ------------------------------------------------------------------------- */
/* The presentation order of flags in the flags pane.  The two tables must,
   of course, match.  */

static char flag_names[] = "cpaznido";

static unsigned flag_bits[] =
{ 0x0001, 0x0004, 0x0010, 0x0040, 0x0080, 0x0200, 0x0400, 0x0800 };
/* ------------------------------------------------------------------------- */
/* Breakpoint data.  When breakpoints are actually put into the cpu debug
   registers, data breakpoints have first priority.  Any code breakpoints
   left over are set by patching the code to contain "Int 3".  */

typedef enum { BP_Code = 0, BP_Write = 1, BP_Read = 3 } BP_TYPE;

typedef struct
{
  word32 addr;                /* Linear address of breakpoint.  */
  int count;                  /* #hits left before code bp triggers.  */
  char *condition;            /* Condition for code breakpoints.  */
  BP_TYPE type;               /* What kind of breakpoint?  */
  unsigned char length;	      /* 1, 2, or 4 bytes.  */
  unsigned char saved;        /* Non-zero for code bps using int 0x03.  */
  unsigned char savedbyte;    /* Patched-away byte.  */
  unsigned char disabled;     /* Non-zero means don't put into cpu regs.  */
  unsigned char temporary;    /* A temporary like those used for F4.  */
} BP_ENTRY;

static int breakpoint_count;
static BP_ENTRY *breakpoint_table;
static unsigned char int03 = 0xcc;
/* ------------------------------------------------------------------------- */
#ifndef xmalloc
extern void *xmalloc (size_t);
extern void *xrealloc (void *, size_t);

char *
strdup (const char *_s)
{
  char *res = xmalloc (strlen (_s) + 1);
  strcpy (res, _s);
  return res;
}
#endif
/* ------------------------------------------------------------------------- */
/* Evaluate EXPR and return the result.  Set OKP to 1 if and if only the
   expression was sucessfully evaluated.  Display an error message if not.  */

static int
eval (char *expr, int *okp)
{
  int32 sym;
  int err;
  char *errtxt;

  if ((err = evaluate (expr, &sym, &errtxt)))
    message (CL_Error, "%s", errtxt);
  *okp = !err;
  return (int) sym;
}
/* ------------------------------------------------------------------------- */
/* Read an expression and evaluate it.  */

static int
read_eval (int *okp, char *starttext)
{
  *okp = !read_string (starttext);
  if (*okp && read_buffer[0] != '\0')
    return eval (read_buffer, okp);
  else
    return *okp = 0;
}
/* ------------------------------------------------------------------------- */
/* Try to read memory using dpmi tricks.  Return 1 if ok.  */

static int
dpmi_readmem (word32 physaddr, void *buf, unsigned len)
{
  union REGS regs;
  int ok = 0;

  regs.w.ax = 0x0000;
  regs.w.cx = 1;
  int86 (0x31, &regs, &regs);
  if (!regs.w.cflag)
    {
      word16 sel = regs.w.ax;
      regs.w.ax = 0x0007;
      regs.w.bx = sel;
      regs.w.cx = physaddr >> 16;
      regs.w.dx = (word16)physaddr;
      int86 (0x31, &regs, &regs);
      if (!regs.w.cflag)
	{
	  regs.w.ax = 0x0008;
	  regs.w.bx = sel;
	  regs.w.cx = len >> 16;
	  regs.w.dx = (word16)len;
	  int86 (0x31, &regs, &regs);
	  if (!regs.w.cflag)
	    {
	      movedata (sel, 0, _go32_my_ds (), (unsigned)buf, len);
	      ok = 1;
	    }
	}
      regs.w.ax = 0x0001;
      regs.w.bx = sel;
      int86 (0x31, &regs, &regs);
    }
  return ok;
}
/* ------------------------------------------------------------------------- */
/* Get a descriptor from TABLE, entry NO, to DESCR.  Return its type, or
   -1 in case of failure.  */

static int
getdescriptor (word32 tablebase, int no, void *descr)
{
  word32 phys;

  phys = tablebase + no * 8;
  if (phys + sizeof (GDT_S) <= 0x100000l)
    dosmemget (phys, 8, descr);
  else if (dpmi && dpmi_readmem (phys, descr, 8))
    /* Nothing.  */;
  else
    return -1;
  return ((GDT_S *)descr)->stype & 0x1f;
}
/* ------------------------------------------------------------------------- */
/* At BUF place the textual representation of DESCR.  */

static void
describedescriptor (char *buf, void *descr, int table)
{
  IDT *idtp = (IDT *)descr;
  GDT_S *gdtp = (GDT_S *)descr;
  int typ = gdtp->stype & 0x1f;
  word32 limit = ((gdtp->lim1 & 0x0f) << 16) | gdtp->lim0;

  if (gdtp->lim1 & 0x80)
    limit = (limit << 12) | 0xfff;

  if ((gdtp->stype & 0x80) == 0)
    sprintf (buf, "Not present.");
  else if (((allowed_descriptors[table] >> typ) & 1) == 0)
    sprintf (buf, "Invalid in context (%02x)", typ);
  else
    switch (typ)
      {
      case 0x01:
      case 0x03:
	sprintf (buf, "16-bit TSS  %s",
		 typ & 2 ? " (busy)" : "");
	break;
      case 0x02:
	sprintf (buf, "LDT %02x%02x%04x+%08lx",
		 gdtp->base2, gdtp->base1, gdtp->base0, limit);
	break;
      case 0x04:
	sprintf (buf, "16-bit Call Gate.");
	break;
      case 0x05:
	sprintf (buf, "Task Gate, TSS=%04x",
		 (unsigned) idtp->selector);
	break;
      case 0x06:
	sprintf (buf, "16-bit Interrupt Gate");
	break;
      case 0x07:
	sprintf (buf, "16-bit Trap Gate");
	break;
      case 0x09:
      case 0x0b:
	sprintf (buf, "32-bit TSS  %02x%02x%04x+%08lx%s",
		 (unsigned) gdtp->base2,
		 (unsigned) gdtp->base1,
		 (unsigned) gdtp->base0,
		 limit,
		 typ & 2 ? " (busy)" : "");
	break;
      case 0x0c:
	sprintf (buf, "32-bit Call Gate, %04x:%04x%04x (%d)",
		 (unsigned) idtp->selector,
		 (unsigned) idtp->offset1, (unsigned) idtp->offset0,
		 gdtp->base1 & 0x0f);
	break;
      case 0x0e:
	sprintf (buf, "32-bit Interrupt Gate, %04x:%04x%04x",
		 (unsigned) idtp->selector,
		 (unsigned) idtp->offset1, (unsigned) idtp->offset0);
	break;
      case 0x0f:
	sprintf (buf, "32-bit Trap Gate, %04x:%04x%04x",
		 (unsigned) idtp->selector,
		 (unsigned) idtp->offset1, (unsigned) idtp->offset0);
	break;
      case 0x10 ... 0x17:
	sprintf (buf,
		 "%s-bit data %02x%02x%04x+%08lx %s %s %s",
		 gdtp->lim1 & 0x40 ? "32" : "16",
		 gdtp->base2, gdtp->base1, gdtp->base0,
		 limit,
		 typ & 0x04 ? "E\031" : "E\030",
		 typ & 0x02 ? "RW" : "RO",
		 typ & 0x01 ? "A+" : "A-");
	break;
      case 0x18 ... 0x1f:
	sprintf (buf,
		 "%s-bit code %02x%02x%04x+%08lx %s %s %s",
		 gdtp->lim1 & 0x40 ? "32" : "16",
		 gdtp->base2, gdtp->base1, gdtp->base0,
		 limit,
		 typ & 0x04 ? "C+" : "C-",
		 typ & 0x02 ? "R+" : "R-",
		 typ & 0x01 ? "A+" : "A-");
	break;
      case 0x00:
      case 0x08:
      case 0x0a:
      case 0x0d:
	sprintf (buf, "Invalid (%02x)", typ);
	break;
      default:
	sprintf (buf, "Invalid (%02x)", typ);
      }
}
/* ------------------------------------------------------------------------- */
static void
show_tss (void *descr)
{
  GDT_S *gdtp = (GDT_S *)descr;
  word32 base = (gdtp->base2 << 24) | (gdtp->base1 << 16) | (gdtp->base0);
  word32 limit = ((gdtp->lim1 & 0x0f) << 16) | gdtp->lim0;
  int y = 1, width = cols - 19;
  char buf[80];
  TSS tss;

  if (gdtp->lim1 & 0x80)
    limit = (limit << 12) | 0xfff;

  sprintf (buf, "Tss at %08lx+%08lx", base, limit);
  putl (1, y++, width, buf);
  putl (1, y++, width, "");
  if (limit < 0x67)
    putl (1, y++, width, "Invalid length");
  else
    {
      int ok;

      if (base + sizeof (tss) <= 0x100000l)
	{
	  dosmemget (base, sizeof (tss), &tss);
	  ok = 1;
	}
      else if (dpmi && dpmi_readmem (base, &tss, sizeof (tss)))
	ok = 1;
      else
	{
	  ok = 0;
	  putl (1, y++, width, "Read failure.");
	}

      if (ok)
	{
	  sprintf (buf, "s0=%04x:%08lx   s1=%04x:%08lx   s1=%04x:%08lx",
		   tss.tss_ss0, tss.tss_esp0,
		   tss.tss_ss1, tss.tss_esp1,
		   tss.tss_ss2, tss.tss_esp2);
	  putl (1, y++, width, buf);
	  sprintf (buf, "ldt=%04x   link=%04x   trap=%s   io=%04x",
		   tss.tss_ldt,
		   tss.tss_back_link,
		   tss.tss_trap ? "Yes" : "No",
		   tss.tss_iomap);
	  putl (1, y++, width, buf);
	  putl (1, y++, width, "");
	  sprintf (buf, "ds=%04x   es=%04x   fs=%04x   gs=%04x",
		   tss.tss_ds, tss.tss_es, tss.tss_fs, tss.tss_gs);
	  putl (1, y++, width, buf);
	  putl (1, y++, width, "");
	  sprintf (buf, "eax=%08lx   ebx=%08lx   ecx=%08lx   edx=%08lx",
		   tss.tss_eax, tss.tss_ebx, tss.tss_ecx, tss.tss_edx);
	  putl (1, y++, width, buf);
	  sprintf (buf, "esi=%08lx   edi=%08lx   ebp=%08lx   flg=%08lx",
		   tss.tss_esi, tss.tss_edi, tss.tss_ebp, tss.tss_eflags);
	  putl (1, y++, width, buf);
	  sprintf (buf, "cs:eip=%04x:%08lx   ss:esp=%04x:%08lx",
		   tss.tss_cs, tss.tss_eip, tss.tss_ss, tss.tss_esp);
	  putl (1, y++, width, buf);
	  putl (1, y++, width, "");
#ifndef V2DBG
	  if (!dpmi)
	    {
	      /* From here on it is go32 specific information.  */
	      sprintf (buf, "cr2=%08lx   error=%08lx   irqn=%02x",
		       tss.tss_cr2, tss.tss_error, tss.tss_irqn);
	      putl (1, y++, width, buf);
	    }
#endif
	}
    }

  while (y < toplines)
    putl (1, y++, width, "");
  putl (1, y++, width, "Press Enter");
  put_screen (debug_screen_save);
  (void) getxkey ();
}
/* ------------------------------------------------------------------------- */
/* Check that it is valid to access at ADDR, LENGTH bytes.  Two adjacent
   areas will not be considered as one area (this is a bug).  */

int
valid_addr (word32 vaddr, int len)
{
#ifdef V2DBG
  if (vaddr < 0x1000) return 0;
  if (vaddr >= 0xffffffffl - len) return 0;
  if (vaddr + len > __dpmi_get_segment_limit (_my_ds ())) return 0;
  return 1;
#else
  int a;

  if (vaddr >= 0xffffffffl - len)
    return 0;
  len--;
  for (a = 0; a < MAX_AREA; a++)
    if ((vaddr + len <= areas[a].last_addr) && (vaddr >= areas[a].first_addr))
      return 1;
  if (!dpmi && vaddr >= 0xe0000000l && vaddr + len <= 0xe0100000l)
    return 1; /* Remapped core.  */
  return 0;
#endif
}
/* ------------------------------------------------------------------------- */
/* Check that it is valid to access child address ADDR as an instruction.  */

inline static int
valid_instaddr (word32 vaddr)
{
  return valid_addr (vaddr, MAXINSTLEN);
}
/* ------------------------------------------------------------------------- */
/* Set physical breakpoint registers from virtual ones.	 Only the first
   four data breakpoints are honoured.  When the debug registers are
   full, extra code breakpoints are set by patching "Int 3" into the
   code.  */

static void
activate_breakpoints (void)
{
  int b, no;
  BP_ENTRY *bep;

  no = 0;
  edi.dr[7] = 0;
  /* First handle data breakpoints.  */
  for (b = 0, bep = breakpoint_table; b < breakpoint_count; b++, bep++)
    if (!bep->disabled && no <= 3 && bep->type != BP_Code)
      {
	bep->saved = 0;
	edi.dr[7] |= ((bep->type + ((bep->length - 1) << 2)) << (16 + 4 * no)
		      | (2 << (2 * no)));
	edi.dr[no] = bep->addr;
	no++;
      }

  /* Now handle code breakpoint.  */
  for (b = 0, bep = breakpoint_table; b < breakpoint_count; b++, bep++)
    if (!bep->disabled && bep->type == BP_Code)
    {
      if (no <= 3)
	{
	  bep->saved = 0;
	  edi.dr[7] |= ((BP_Code << (16 + 4 * no)) | (2 << (2 * no)));
	  edi.dr[no] = bep->addr;
	  no++;
	  edi.dr[7] |= 0x00000300L;  /* For 386s we set GE & LE bits.  */
	}
      else
	{
	  bep->saved = valid_addr (bep->addr, 1);
	  if (bep->saved)
	    {
	      read_child (bep->addr, &bep->savedbyte, 1);
	      write_child (bep->addr, &int03, 1);
	    }
	}
    }
}
/* ------------------------------------------------------------------------- */
/* Un-patch code.  This means restore the instruction under any "Int 3"
   that was patched-in by previous function.  */

static void
deactivate_breakpoints (void)
{
  int b;
  BP_ENTRY *bep;

  for (b = 0, bep = breakpoint_table; b < breakpoint_count; b++, bep++)
    if (!bep->disabled && bep->saved)
      write_child (bep->addr, &bep->savedbyte, 1);
}
/* ------------------------------------------------------------------------- */
/* Get the serial number of a breakpoint of TYPE and LENGTH at ADDR.  Return
   -1 if no such breakpoint is set.  */

static int
get_breakpoint (BP_TYPE type, int length, word32 addr)
{
  int b;

  for (b = 0; b < breakpoint_count; b++)
    if (breakpoint_table[b].type == type
	&& breakpoint_table[b].length == length
	&& breakpoint_table[b].addr == addr)
      return b;
  return -1;
}
/* ------------------------------------------------------------------------- */
/* Delete breakpoint number B.  */

static void
reset_breakpoint (int b)
{
  if (breakpoint_table[b].condition)
    free (breakpoint_table[b].condition);
  breakpoint_table[b] = breakpoint_table[--breakpoint_count];
  breakpoint_table
    = xrealloc (breakpoint_table, breakpoint_count * sizeof (BP_ENTRY));
}
/* ------------------------------------------------------------------------- */
/* Set a breakpoint of TYPE and LENGTH at ADDR and return the serial number
   of the new breakpoint.  */

static int
set_breakpoint (BP_TYPE type, int length, word32 addr)
{
  int b;

  b = breakpoint_count;
  breakpoint_table
    = xrealloc (breakpoint_table, ++breakpoint_count * sizeof (BP_ENTRY));
  bzero (breakpoint_table + b, sizeof (BP_ENTRY));
  breakpoint_table[b].addr = addr;
  breakpoint_table[b].type = type;
  breakpoint_table[b].length = length;
  return b;
}
/* ------------------------------------------------------------------------- */
static void
select_source_file (char *file)
{
  if (source_pane_module == 0 || strcmp (file, source_pane_module))
    {
      int i, no;

      while (source_text_count)
	free (source_pane_text[--source_text_count]);

      for (no = 0;
	   no < module_text_count && strcmp (file, module_pane_text[no]);
	   no++)
	/* Nothing */;

      if (no < module_text_count)
	{
	  char buf[1000];
	  int lineno;

	  source_text_count = file_line_count (file) + 1;
	  source_pane_text
	    = xrealloc (source_pane_text,
			source_text_count * sizeof (char *));
	  source_pane_module = file;
	  source_pane_text[0] = (char *)xmalloc (20 + strlen (file));
	  sprintf (source_pane_text[0],
		   "Module %s:", source_pane_module);
	  for (lineno = 1; lineno < source_text_count; lineno++)
	    {
	      buf[0] = 0;
	      put_source_line (2, source_pane_module, lineno, buf);
	      source_pane_text[lineno] = strdup (buf);
	    }
	  i = toplines - 3;
	  source_pane_origin = module_line_numbers [no] - i;
	  if (source_pane_origin < 0)
	    i += source_pane_origin,
	    source_pane_origin = 0;
	}
      else
	{
	  source_pane_text = xrealloc (source_pane_text, 0);
	  source_pane_module = 0;
	  source_text_count = source_pane_origin = i = 0;
	}
      pane_positions[PANE_SOURCE] = i;
      if (pane == PANE_SOURCE) pane_pos = i;
    }
}
/* ------------------------------------------------------------------------- */
/* From ORIGIN skip COUNT instructions forward (positive count) or
   backwards (negative count).	Backwards skipping cannot be assumed to
   be working 100% perfectly, but in the presense of debug information
   it is probably very, very close.

   This function works poorly close to the extremes of segments.  */

static int
code_skip (int origin, int count)
{
  int len, *next, i, j, k, instcount, done, leave;
  char *state, *source;

  if (count >= 0)
    {
      while (count-- > 0)
	if (valid_instaddr (origin))
	  {
	    (void) unassemble_proper (origin, &len);
	    origin += len;
	  }
	else
	  origin ++;
      return origin;
    }
  else
    {
      count = -count;
      instcount = MAXINSTLEN * (count + 16) + 1;
      next = alloca (instcount * sizeof (int));
      memset (next, 0, instcount * sizeof (int));
      state = alloca (instcount * sizeof (char));
      memset (state, 0, instcount * sizeof (char));

      done = 0;
      do
	{
	  for (i = 0; i < 2 * count; i++)
	    {
	      done++;
	      j = origin - done;
	      if (valid_instaddr (j))
		{
		  unassemble_proper (j, &len);
		  source = unassemble_source (j);
		  next[done] = j + len;
		  if (source)
		    {
		      leave = 0;
		      k = done;
		      while (j < origin && state[k] == 0 && !leave)
			{
			  state[j++,k--] = 2;
			  while (len-- > 1)
			    state[j++,k--] = 3;
			  /* Since code and data is "never" mixed in 32 bit
			     code we don't need this.  */
#if 0
			  leave = (strncmp (inst, "jmp", 3) == 0
				   || strncmp (inst, "ret", 3) == 0
				   || strncmp (inst, "iret", 4) == 0);
#endif
			  if (!leave)
			    unassemble_proper (j, &len);
			}
		    }
		}
	      else
		{
		  state[done] = 2;
		  next[done] = j + 1;
		}
	    }
	  j = 1;
	  k = count;
	  while (k > 0 && j <= done && state[j] >= 2)
	    if (state[j++] == 2)
	      k--;
	}
      while (k > 0 && done + 2 * count <= instcount)
	;
      if (k == 0)
	return origin - j + 1;
      else
	{
	  i = origin;
	  k = 0;
	  while (count > 0 && k <= done)
	    {
	      leave = 0;
	      j = MAXINSTLEN;
	      while (!leave && j > 1)
		if (j + k <= done && next[j + k] == i)
		  leave = 1;
		else
		  j--;
	      if (!leave)
		i--, k++;
	      else
		i -= j, k += j;
	      count--;
	    }
	  return i;
	}
    }
}
/* ------------------------------------------------------------------------- */
/* Make the disassembly in the code pane include the instruction starting
   at ADDR.  The display is scrolled if necessary.  Do the same for the
   source pane, if possible.  */

static void
code_pane_goto (word32 v)
{
  int i, line;
  char *file;

  if (v >= code_dump_origin && v <= code_dump_last)
    {
      i = 0;
      while (code_pane_pos[i] < v)
	i++;
      if (code_pane_pos [i] == code_pane_pos[i+1])
	i++;
    }
  else
    {
      code_dump_origin = v;
      i = (valid_instaddr (v) && unassemble_source (v) != NULL);
    }
  pane_positions[PANE_CODE] = i;
  if (pane == PANE_CODE) pane_pos = i;

  file = syms_val2line (v, &line, 0);
  if (file)
    {
      select_source_file (file);
      if (source_pane_origin <= line && line < source_pane_origin + toplines)
	i = line - source_pane_origin;
      else
	source_pane_origin = line, i = 0;
      pane_positions[PANE_SOURCE] = i;
      if (pane == PANE_SOURCE) pane_pos = i;
    }
}
/* ------------------------------------------------------------------------- */
static word32
find_source_line (int line, int offset)
{
  word32 addr;

  if (source_pane_module)
    for (; offset; offset--, line++)
      if ((addr = syms_line2val (source_pane_module, line)))
	return addr;
  return 0;
}
/* ------------------------------------------------------------------------- */
/* Let the child process run freely (or rather as freely as debug registers
   and breakpoints decide.  */

inline static void
go (int bp)
{
  if (bp) activate_breakpoints ();
  run_child ();
  if (bp) deactivate_breakpoints ();
}
/* ------------------------------------------------------------------------- */
/* Let the child process loose in the specified way.  Try to optimize for
   reduced swapping of screens.  */

typedef enum { R_Step, R_Step1, R_Over, R_Run, R_RunMain } KIND_TYPE;

static void
step (KIND_TYPE kind)
{
  int i, b = -1, no, len, int03hit;
  char *inst = NULL;
  int tracing = (kind == R_Step1);
  word32 final = -1;

  switch (kind)
    {
    case R_Step1:
      kind = R_Step;
      /* Fall through.  */
    case R_Step:
      inst = unassemble_proper (a_tss.tss_eip, &len);
      if (strcmp (inst, "popf") == 0 || strcmp (inst, "pushf") == 0)
	{
	  kind = R_Over;  /* Push the right value of flags (no trace flag).  */
	  final = a_tss.tss_eip + len;
	}
      break;
    case R_Over:
      if (first_step)
	kind = R_RunMain;
      else
	{
	  inst = unassemble_proper (a_tss.tss_eip, &len);
	  if (strncmp (inst, "loop", 4)
	      && strncmp (inst, "call", 4)
	      && strncmp (inst, "int", 3) )
	    kind = R_Step;
	  else
	    final = a_tss.tss_eip + len;
	}
      break;
    default:;
      /* Nothing.  */
    }

 retry:
  /* A free run at a non-disabled breakpoint is bad.  */
  if (kind != R_Step)
    {
      int again = 1;

      while (again && (b = get_breakpoint (BP_Code, 0, a_tss.tss_eip)) != -1)
	{
	  breakpoint_table[b].disabled++;
	  step (R_Step1);
	  breakpoint_table[b].disabled--;
	  again = !terminated && a_tss.tss_irqn == 1 && final != a_tss.tss_eip;
	}
    }

  switch (kind)
    {
    case R_Step1: /* Can't happen, but keeps gcc happy.  */
    case R_Step:
      /* If we are referencing memory we should swap-in the user screen.  */
      if (strchr (inst, '['))
      {
	/* Assume that all access to code and stack segments are safe.
	   This should hold unless you do something extra-ordinarily dirty.  */
	if (((a_tss.tss_ds == a_tss.tss_cs) || (a_tss.tss_ds == a_tss.tss_ss))
	    &&
	    ((a_tss.tss_es == a_tss.tss_cs) || (a_tss.tss_es == a_tss.tss_ss))
	    &&
	    ((a_tss.tss_fs == a_tss.tss_cs) || (a_tss.tss_fs == a_tss.tss_ss)
	     || (strstr (inst, "fs:") == 0))
	    &&
	    ((a_tss.tss_gs == a_tss.tss_cs) || (a_tss.tss_gs == a_tss.tss_ss)
	     || (strstr (inst, "gs:") == 0)))
	  /* Nothing.  */;
	else
	  user_screen ();
      }
      a_tss.tss_eflags |= 0x0100;
      edi.dr[7] = 0;
      go (0);
      a_tss.tss_eflags &= ~0x0100;
      break;
    case R_Over:
      user_screen ();
      b = set_breakpoint (BP_Code, 0, final);
      go (1);
      break;
    case R_Run:
      user_screen ();
      go (1);
      break;
    case R_RunMain:
      b = set_breakpoint (BP_Code, 0, main_entry);
      user_screen ();
      go (1);
      reset_breakpoint (b);
      break;
    }
  first_step = 0;
  i = a_tss.tss_irqn;
  if ((terminated = (i == 0x21) && (a_tss.tss_eax & 0xff00) == 0x4c00))
    a_tss.tss_eip -= 2;	 /* point back to Int 21h */
  if ((int03hit = (i == 0x03)
       && get_breakpoint (BP_Code, 0, a_tss.tss_eip - 1) != -1))
    a_tss.tss_eip--;  /* point back to Int 3 */
  if (kind == R_Over && b >= 0)
    reset_breakpoint (b);  /* reset only after get_breakpoint did its thing */
  if (tracing) return;

  /* Find out whether a breakpoint stopped us.  */
  no = -1;
  if (i == 1 || int03hit)
    {
      BP_ENTRY *bep;

      for (b = 0; b <= 3; b++)
	if ((edi.dr[6] & (1 << b)) && (edi.dr[7] & (3 << (b * 2))))
	  {
	    no = get_breakpoint (((edi.dr[7] >> (16 + 4 * b)) & 3),
				 ((edi.dr[7] >> (18 + 4 * b)) & 3) + 1,
				 edi.dr[b]);
	    break;
	  }
      if (no == -1)
	no = get_breakpoint (BP_Code, 0, a_tss.tss_eip);

      bep = breakpoint_table + no;
      if (no != -1 && bep->type == BP_Code)
	{
	  if (bep->condition)
	    {
	      char *errtxt;
	      int32 val;

	      if (evaluate (bep->condition, &val, &errtxt))
		{
		  message (CL_Error, "Hit breakpoint with invalid condition");
		  goto error;
		}
	      if (val == 0)
		/* We hit a breakpoint, but its condition is not satisfied.*/
		goto retry;
	    }
	  if (--(bep->count) > 0)
	    /* We hit a breakpoint but it is not yet mature.  */
	    goto retry;
	  bep->count = 0;
	}
    error:;
    }

  code_pane_goto (a_tss.tss_eip);
  debug_screen ();
  redraw (0);
  while (kbhit ())
    (void) getxkey ();

  if (terminated)
    message (CL_Msg, "Program terminated normally, exit code is %d",
	     (word8) a_tss.tss_eax);
  else if (i == 1 || int03hit)
    {
      if (no != -1)
	switch (breakpoint_table[no].type)
	  {
	  case BP_Code:
	    if (!breakpoint_table[no].temporary)
	    {
	      if (breakpoint_table[no].condition)
		message (CL_Msg, "Condition \"%s\" satisfied",
			 breakpoint_table[no].condition);
	      else
		message (CL_Info, "Code breakpoint hit");
	    }
	    break;
	  case BP_Write:
	    message (CL_Msg, "Data write breakpoint at 0x%08lx triggered by previous instruction",
		     breakpoint_table[no].addr);
	    break;
	  case BP_Read:
	    message (CL_Msg, "Data read/write breakpoint at 0x%08lx triggered by previous instruction",
		     breakpoint_table[no].addr);
	    break;
	  }
      else if (i == 1 && (edi.dr[6] & (1 << 13)))
	message (CL_Error, "User program used debug registers");
      else if (i == 1 && (edi.dr[6] & (1 << 15)))
	message (CL_Error, "Task switch caused debug exception");
    }
  else if (i == 3 && !int03hit)
    message (CL_Error, "Unexpected Int 3 hit");
  else if (i == 0x79 || i == 0x09)
    message (CL_Info, "Keyboard interrupt");
  else if (i == 0x7a)
    message (CL_Info, "QUIT key pressed");
  else if (i == 0x75)
    {
      char *reason;

      if ((npx.status & 0x0241) == 0x0241)
	reason = "stack overflow";
      else if ((npx.status & 0x0241) == 0x0041)
	reason = "stack underflow";
      else if (npx.status & 1)
	reason = "invalid operation";
      else if (npx.status & 2)
	reason = "denormal operand";
      else if (npx.status & 4)
	reason = "divide by zero";
      else if (npx.status & 8)
	reason = "overflow";
      else if (npx.status & 16)
	reason = "underflow";
      else if (npx.status & 32)
	reason = "loss of precision";
      else
	reason = "?";
      message (CL_Error, "Numeric Exception (%s) at eip=0x%08lx",
	       reason, npx.eip);
    }
  else if (i == 8 || (i >= 10 && i <= 14) || i == 17)
    message (CL_Error, "Exception %d (0x%02x) occurred, error code=%#lx",
	     i, i, a_tss.tss_error);
  else
    message (CL_Error, "Exception %d (0x%02x) occurred", i, i);
}
/* ------------------------------------------------------------------------- */
/* Assuming EIP, ESP, and EBP as values for the obvious registers, track
   back one stack from.  Return non-zero on success.  */

static int
stack_trace (word32 *eip, word32 *esp, word32 *ebp)
{
  /* We look directly at the bit pattern instead of using disassembly;
     the bit patterns are those generated by gcc.  In general we
     cannot expect to be able to follow a non-gcc stack anyway.  */
  unsigned char eipcode[4];

  if (!valid_instaddr (*eip))
    return 0;

  read_child (*eip, eipcode, 3);
  if ((eipcode[0] == 0x55 && eipcode[1] == 0x89 && eipcode[2] == 0xe5)
      || eipcode[0] == 0xc3)
    {
      /* In this case where we are looking at `Push Ebp//Mov Ebp,Esp'
	 or `Ret', only the return address is on the stack; I believe
	 this only to happen in the innermost activation record.  */
      if (!valid_addr (*esp, 4))
	return 0;
      read_child (*esp, eip, 4);
      *esp += 4;
      return 1;
    }
  else
    {
      if (! (eipcode[0] == 0x89 && eipcode[1] == 0xe5))
      {
	 /* We are *not* looking at `Mov Ebp,Esp'.  */
	if (*ebp < 0x80000000UL && *ebp >= *esp && valid_addr (*ebp, 8))
	  *esp = *ebp;
	else
	  return 0;
      }
      else
	if (!valid_addr (*esp, 8))
	  return 0;
      read_child (*esp, ebp, 4);
      *esp += 4;
      read_child (*esp, eip, 4);
      *esp += 4;
      return 1;
    }
}
/* ------------------------------------------------------------------------- */
/* Trace execution path until a source point is reached, execution jumps,
   or LEVEL call levels is processed.  */

static word32
trace_till_source (word32 eip, int level, int mustmove)
{
  char *inst;
  unsigned char eipcode[5];
  int len;
  int line;

  while (valid_instaddr (eip) && (mustmove || !syms_val2line (eip, &line, 1)))
    {
      signed long offset;
      word32 try;

      mustmove = 0;
      read_child (eip, eipcode, 5);
      switch (eipcode[0])
	{
	case 0xea:
	  offset = *((signed char *)(eipcode + 1));
	  try = eip + 2 + offset;
	  if (offset >= 0)
	    {
	      eip = try;
	      continue;
	    }
	  break;
	case 0xe8:
	  offset = *((signed long *)(eipcode + 1));
	  try = eip + 5 + offset;
	  if (offset >= 0)
	    {
	      eip = try;
	      continue;
	    }
	  break;
	case 0xe9:
	  offset = *((signed long *)(eipcode + 1));
	  try = eip + 5 + offset;
	  break;
	default:
	  try = 0;
	}

      if (try && level && syms_val2line (try, &line, 0))
	{
	  try = trace_till_source (try, level - 1, 0);
	  if (try) return try;
	}

      inst = unassemble_proper (eip, &len);
      if (strncmp (inst, "ret", 3)
	  && strncmp (inst, "call", 4)
	  && strncmp (inst, "int", 3)
	  && strncmp (inst, "iret", 4)
	  && strncmp (inst, "loop", 4)
	  && inst[0] != 'j')
	eip += len;
      else
	break;
    }
  if (syms_val2line (eip, &line, 1))
    return eip;
  else
    return 0;
}
/* ------------------------------------------------------------------------- */
/* Redraw all active panes.  If FIRST then also redraw the frames.  */

static void
redraw (int first)
{
  char *buf = alloca (8192);

  debug_screen ();
  screen_attr = ScreenAttrib = screen_attr_normal;

  if (first)
    {
      int y = toplines + 1, x1 = cols - 18, x2 = cols - 5, x3 = 46;

      frame (0, 0, cols - 1, rows - 1);
      frame (x2, 0, cols - 1, y);
      frame (x1, 0, x2, y);
      frame (x3, y, cols - 1, rows - 1);
      frame (0, y, x3, rows - 1);

      put (x2, 0, "Â");
      put (x2, y, "Á");
      put (0, y, "Ã");
      put (cols - 1, y, "´");
      put (x1, 0, "Â");
      put (x1, y, "Á");
      put (x3, y, "Â");
      put (x3, rows - 1, "Á");
    }

  {
    /* Show register status */
    int reg, x = cols - 17, y = 1, width = 12;

    for (reg = register_pane_origin; y <= toplines && regs_type[reg]; reg++)
      {
	if (regs_type[reg] == 's')
	  sprintf (buf, " %s %04x",
		   regs_names[reg],
		   *(unsigned short *)(regs_addr[reg]));
	else
	  sprintf (buf, "%s %08lx",
		   regs_names[reg],
		   *(regs_addr[reg]));
	putl (x, y++, width, buf);
      }
    while (y <= toplines)
      putl (x, y++, width, "");
  }

  {
    /* Show flags status */
    int f, x = cols - 4, y = 1, width = 3;

    for (f = flag_pane_origin; y <= toplines && flag_names[f]; f++)
      {
	sprintf (buf, "%c=%d",
		 flag_names[f],
		 (a_tss.tss_eflags & flag_bits[f]) != 0);
	put (x, y++, buf);
      }
    while (y <= toplines)
      putl (x, y++, width, "");
  }

  {
    /* Show breakpoints */
    int b, x = 47, y = toplines + 2, width = cols - 48;
    char *name;
    word32 delta;

    for (b = breakpoint_origin;
	 b < breakpoint_origin + bottomlines / 2 && b < breakpoint_count;
	 b++)
      {
	switch (breakpoint_table[b].type)
	  {
	  case BP_Code:
	    sprintf (buf, "Execute: %08lx",
		     breakpoint_table[b].addr);
	    break;
	  case BP_Write:
	    sprintf (buf, "Data write: %08lx, %d",
		     breakpoint_table[b].addr, breakpoint_table[b].length);
	    break;
	  case BP_Read:
	    sprintf (buf, "Data read: %08lx, %d",
		     breakpoint_table[b].addr, breakpoint_table[b].length);
	    break;
	  }
	putl (x, y++, width, buf);

	name = syms_val2name (breakpoint_table[b].addr, &delta);
	if (name[0] != '0')
	{
	  if (delta && (int)strlen (name) < width)
	    sprintf (buf, " %s+%#lx", name, delta);
	  else
	    sprintf (buf, " %-*s", width, name);
	}
	else
	  buf[0] = '\0';
	putl (x, y++, width , buf);
      }
    if (breakpoint_count == 0)
      putl (x, y++, width, "No breakpoints");
    while (y < rows - 1)
      putl (x, y++, width, "");
  }

  if (data_pane_active)
    {
      /* Show data dump */
      word32 p = data_dump_origin;
      int b, x, y = toplines + 2, ok, width = 45;
      unsigned char data[8], xpos[8];

      for (x = 0; x < 8; x++)
	xpos[x] = 10
	  + ((data_dump_size == 1) ? (x * 3) :
	     ((data_dump_size == 2) ? ((x / 2) * 5 + (~x & 1) * 2) :
	      ((x / 4) * 9 + (~x & 3) * 2)));

      while (y < rows - 1)
	{
	  if ((ok = valid_addr (p, 8)))
	    read_child (p, data, 8);

	  sprintf (buf, "%08lx: %35s", p, "");
	  for (x = 0; x < 8; x++)
	    if (ok || valid_addr (p + x, 1))
	      {
		if (!ok) read_child (p + x, data + x, 1);
		buf[xpos[x]] = hexchars[data[x] >> 4];
		buf[xpos[x] + 1] = hexchars[data[x] & 0xf];
		buf[37 + x] = (data[x] ? data[x] : '.');
	      }
	    else
	      buf[xpos[x]] = buf[xpos[x] + 1] = buf[37 + x] = '?';
	  putl (1, y, width, buf);

	  screen_attr = screen_attr_break;
	  for (x = 0; x < 8; x++)
	    for (b = 0; b < breakpoint_count; b++)
	      if (breakpoint_table[b].type != BP_Code
		  && p + x >= breakpoint_table[b].addr
		  && p + x < (breakpoint_table[b].addr
			      + breakpoint_table[b].length))
		{
		  highlight (xpos[x] + 1, y, 2);
		  highlight (38 + x, y, 1);
		}
	  screen_attr = screen_attr_normal;
	  p += 8;
	  y++;
	}
      data_dump_last = p - 1;
    }

  if (code_pane_active)
  {
    /* Show code dump */
    word32 p = code_dump_origin, width = cols - 19;
    int y = 1, len, source = 0;
    char *txt;

    while (y <= toplines)
      {
	source = !source;
	code_pane_pos[y - 1] = p;
	if (source)
	  {
	    txt = unassemble_source (p);
	    if (txt)
	      {
		screen_attr = screen_attr_source;
		putl (1, y++, width, txt);
	      }
	  }
	else
	  {
	    if (valid_instaddr (p))
	      txt = unassemble_proper (p, &len);
	    else
	      txt = "?", len = 1;
	    sprintf (buf, "%08lx%c%s",
		     p,
		     p == a_tss.tss_eip ? 16 : ' ',
		     txt);
	    screen_attr =
	      (get_breakpoint (BP_Code, 0, p) == -1
	       ? screen_attr_normal : screen_attr_break);
	    putl (1, y++, width, buf);
	    p += len;
	  }
      }
    code_pane_pos[y - 1] = p + 1;
    code_dump_last = p - 1;
    screen_attr = screen_attr_normal;
  }

  if (npx_pane_active)
  {
    /* Show npx dump */
    int i, y = 1, width = cols - 19;
    static char *rtype[] = { "Near", "-Inf", "+Inf", "Zero" };

    sprintf (buf,
	     "Control: %04lx PR=%s UN=%s OV=%s ZD=%s DN=%s IV=%s Rnd=%s",
	     npx.control & 0xffff,
	     (npx.control & (1 << 5)) ? "N" : "Y",
	     (npx.control & (1 << 4)) ? "N" : "Y",
	     (npx.control & (1 << 3)) ? "N" : "Y",
	     (npx.control & (1 << 2)) ? "N" : "Y",
	     (npx.control & (1 << 1)) ? "N" : "Y",
	     (npx.control & (1 << 0)) ? "N" : "Y",
	     rtype[(npx.control >> 10) & 3]);
    putl (1, y++, width, buf);
    sprintf (buf,
	     "Status:  %04lx PR=%s UN=%s OV=%s ZD=%s DN=%s IV=%s ST=%s",
	     npx.status & 0xffff,
	     (npx.status & (1 << 5)) ? "Y" : "N",
	     (npx.status & (1 << 4)) ? "Y" : "N",
	     (npx.status & (1 << 3)) ? "Y" : "N",
	     (npx.status & (1 << 2)) ? "Y" : "N",
	     (npx.status & (1 << 1)) ? "Y" : "N",
	     (npx.status & (1 << 0)) ? "Y" : "N",
	     (npx.status & (1 << 6)) ? "Y" : "N");
    putl (1, y++, width, buf);
    sprintf (buf, "%19sC3=%d C2=%d C1=%d C0=%d",
	     "",
	     (npx.status & (1 << 14)) != 0,
	     (npx.status & (1 << 10)) != 0,
	     (npx.status & (1 <<  9)) != 0,
	     (npx.status & (1 <<  8)) != 0);
    if (y <= toplines) putl (1, y++, width, buf);

    for (i = 0; y <= toplines && i < 8; i++)
      {
	/* We assume that `long double' is the same type as npx.reg[i].
	   It would be sensible to check that the sizes match, but they
	   don't!  For alignment reasons, `sizeof (long double)' is 12.	 */
	int tag;
	int tos = (npx.status >> 11) & 7;
	int exp = (int)npx.reg[i].exponent - 16382;
	char *dstr = alloca (30);

	dstr[0] = (npx.reg[i].sign) ? '-' : '+';
	dstr[1] = '\0';
	tag = (npx.tag >> (((i + tos) & 7) * 2)) & 3;
	switch (tag)
	  {
	  case 0:
	    if (abs (exp) < 1000)
	      {
		/*  Fix -Wstrict-aliasing.  */
		union {
		  long double d_value;
		  NPXREG r_value;
		} npx_reg_union;
		npx_reg_union.r_value = npx.reg[i];
		sprintf(dstr,"%+.19Lg", npx_reg_union.d_value);
	      }
	    else
	      sprintf (dstr, "Valid, %s, and %s",
		       npx.reg[i].sign ? "negative" : "positive",
		       exp > 0 ? "huge" : "tiny");
	    break;
	  case 1:
	    strcat (dstr, "Zero");
	    break;
	  case 2:
	    if (npx.reg[i].exponent == 0x7fff)
	    {
	      if (npx.reg[i].sig3 == 0x8000
		  && npx.reg[i].sig2 == 0x0000
		  && npx.reg[i].sig1 == 0x0000
		  && npx.reg[i].sig0 == 0x0000)
		strcat (dstr, "Inf");
	      else
		strcat (dstr, "NaN");
	    }
	    else
	      dstr = "Special";
	    break;
	  case 3:
	    dstr = "Empty";
	    break;
	  }
	sprintf (buf, "st(%d): %d %04x %04x%04x%04x%04x %s",
		 i,
		 npx.reg[i].sign, npx.reg[i].exponent,
		 npx.reg[i].sig3, npx.reg[i].sig2,
		 npx.reg[i].sig1, npx.reg[i].sig0,
		 dstr);
	putl (1, y++, width, buf);
      }
    while (y <= toplines)
      putl (1, y++, width, "");
  }

  if (stack_pane_active)
  {
    /* Show stack dump */
    int y = 1, no = -stack_dump_origin, width = cols - 19;
    word32 delta;
    word32 ebp = a_tss.tss_ebp;
    word32 esp = a_tss.tss_esp;
    word32 eip = a_tss.tss_eip;

    stack_dump_more = 0;
    while (valid_instaddr (eip))
      {
	if (no++ >= 0)
	{
	  if (y > toplines)
	    {
	      stack_dump_more = 1;
	      break;
	    }
	  else
	    {
	      char *symaddr, *sourceaddr, *sourcefile;
	      int line, len;

	      symaddr = syms_val2name (eip, &delta);
	      sourcefile = syms_val2line (eip, &line, 0);
	      if (sourcefile)
		{
		  sourceaddr = alloca (cols + strlen (sourcefile));
		  sprintf (sourceaddr, ", line %d in file %s",
			   line, sourcefile);
		}
	      else if (delta)
		{
		  sourceaddr = alloca (cols);
		  sprintf (sourceaddr, "%+ld", (long) delta);
		}
	      else
		sourceaddr = "";
	      len = 15 + strlen (symaddr) + strlen (sourceaddr);
	      if (len >= cols)
		buf = alloca (len);
	      sprintf (buf, "%08lx: %s%s", eip, symaddr, sourceaddr);
	      stack_dump_pos[stack_dump_last = y - 1] = eip;
	      putl (1, y++, width, buf);
	    }
	}

	if (!stack_trace (&eip, &esp, &ebp))
	  break;
      }
    while (y <= toplines)
      putl (1, y++, width, "");
  }

  if (info_pane_active)
  {
    /* Show info */
    int y = 1, width = cols - 19;
    long ul;
    char *s;
    _go32_dpmi_meminfo info;
    _go32_dpmi_registers regs;

#define OUT(s) if (y <= toplines) putl (1, y++, width, (s));

    sprintf (buf, "Debugger version ............: %.2f %s",
	     FULLSCR_VERSION,
	     FULLSCR_VERSION < 1.00 ? "beta" : "");
    OUT (buf);
    switch (_go32_info_block.run_mode)
      {
      case _GO32_RUN_MODE_RAW:
	s = "Raw";
	break;
      case _GO32_RUN_MODE_XMS:
	s = "Xms";
	break;
      case _GO32_RUN_MODE_VCPI:
	s = "Vcpi";
	break;
      case _GO32_RUN_MODE_DPMI:
	s = alloca (20);
	sprintf (s,
#ifdef V2DBG
		 "Dpmi %d.%02d",
#else
		 "Dpmi %d.%02x",
#endif
		 _go32_info_block.run_mode_info >> 8,
		 _go32_info_block.run_mode_info & 0xff);
	break;
      default:
	s = "Unknown";
      }
    sprintf (buf, "Running mode ................: %s", s);
    OUT (buf);
    sprintf (buf, "Protection ring .............: %d (in %s)",
	     a_tss.tss_cs & 3,
	     (a_tss.tss_cs & 4) ? "LDT" : "GDT");
    OUT (buf);
    asm volatile ("pushfl ; popl %0" : "=g" (ul));
    sprintf (buf, "I/O protected ...............: %s",
	     (a_tss.tss_cs & 3) > ((ul >> 12) & 3)
	     ? "Yes" : "No");
    OUT (buf);
    sprintf (buf, "Ctrl-C checking .............: %s",
	     getcbrk () ? "On" : "Off");
    OUT (buf);
    OUT ("");

    _go32_dpmi_get_free_memory_information (&info);
    ul = info.total_physical_pages;
    if (ul > 0)
      {
	ul <<= 12;
	sprintf (buf, "Total physical memory .......: %lu KB", ul >> 10);
	OUT (buf);
      }
    ul = info.available_physical_pages
      ? info.available_physical_pages << 12
	: info.available_memory;
    sprintf (buf, "Remaining physical memory ...: %lu KB", ul >> 10);
    OUT (buf);
    ul = info.available_memory;
    sprintf (buf, "Remaining virtual memory ....: %lu KB", ul >> 10);
    OUT (buf);
    ul = info.free_linear_space << 12;
    if (ul >= 0)
      {
	sprintf (buf, "Free linear space ...........: %ld KB", ul >> 10);
	OUT (buf);
      }
    /* Recall that Dos memory is only made available of direct request;
       using 0xffff does not count as a request.  */
    regs.h.ah = 0x48;
    regs.x.bx = 0xffff;
    regs.x.ss = regs.x.sp = 0;
    _go32_dpmi_simulate_int (0x21, &regs);
    ul = regs.x.bx << 4;
    sprintf (buf, "Free dos memory .............: %ld %s",
	     ul > 8192 ? ul >> 10 : ul,
	     ul > 8192 ? "KB"	  : "Bytes");
    OUT (buf);
    OUT ("");

#ifndef V2DBG
    sprintf (buf, "Program text ................: %08lx - %08lx",
	     areas[A_text].first_addr, areas[A_text].last_addr);
    OUT (buf);
    sprintf (buf, "Program data ................: %08lx - %08lx",
	     areas[A_data].first_addr, areas[A_data].last_addr);
    OUT (buf);
    sprintf (buf, "Program bss .................: %08lx - %08lx",
	     areas[A_bss].first_addr, areas[A_bss].last_addr);
    OUT (buf);
    sprintf (buf, "Program stack ...............: %08lx - %08lx",
	     areas[A_stack].first_addr, areas[A_stack].last_addr);
    OUT (buf);
#endif
#undef OUT

    while (y <= toplines)
      putl (1, y++, width, "");
  }

  if (whereis_pane_active)
  {
    /* Show result of last whereis command */
    int y = 1, width = cols - 19, i = whereis_pane_origin;

    if (whereis_text_count == 0)
      putl (1, y++, width, "(No symbols matched on last search)");
    while (y <= toplines)
      if (i < whereis_text_count)
	putl (1, y++, width, whereis_pane_text[i++]);
      else
	putl (1, y++, width, "");
  }

  if (gdt_pane_active || idt_pane_active || ldt_pane_active)
  {
    /* Show descriptor info.  */
    int y = 1, width = cols - 19, i;
    char *buf1;
    GDT_S descr;
    int typ, count;
    word32 base;

    if (gdt_pane_active)
      {
	typ = 0;
	count = (gdtlinear.limit + 1) / 8;
	base = gdtlinear.base;
	i = gdt_pane_origin;
      }
    else if (idt_pane_active)
      {
	typ = 1;
	count = MIN((idtlinear.limit + 1) / 8, 0x100);
	base = idtlinear.base;
	i = idt_pane_origin;
      }
    else if (ldt_pane_active)
      {
	int ldt;
	word32 limit;

	asm volatile
	  ("xorl  %%eax,%%eax	\n\
	    sldt  %%ax		\n\
	    movl  %%eax,%0"
	   : "=r" ((int)(ldt))
	   : /* no inputs.  */
	   : "%eax");

	if (ldt / 8 == 0)
	  {
	    ldtlinear.base = ldtlinear.limit = 0;
	    putl (1, y++, width, "There is no local descriptor table.");
	  }
	else
	  {
	    /* We can always get the limit with the `lsl' instruction, but
	       there seems to be no other way of getting the base.  */
	    if (getdescriptor (gdtlinear.base, ldt / 8, &descr) != 2)
	      putl (1, y++, width, "LDT is present, but unreadable.");
	    else
	      {
		base = ldtlinear.base = descr.base0
		  | ((word32)descr.base1 << 16)
		    | ((word32)descr.base2 << 24);
		limit = ((descr.lim1 & 0x0f) << 16) | descr.lim0;
		if (descr.lim1 & 0x80)
		  limit = (limit << 12) | 0xfff;
		ldtlinear.limit = (limit > 0xffff) ? 0xffff : limit;
	      }
	  }
	typ = 2;
	base = ldtlinear.base;
	count = (ldtlinear.limit + 1) / 8;
	i = ldt_pane_origin;
      }
    else
      abort ();

    while (y <= toplines)
      if (i < count)
	{
	  if (i == 0 && typ == 0)
	    buf1 = "(null entry)";
	  else
	    {
	      if (getdescriptor (base, i, &descr) < 0)
		buf1 = "Read failure.";
	      else
		{
		  buf1 = alloca (80);
		  describedescriptor (buf1, &descr, typ);
		}
	    }
	  if (typ == 1)
	    sprintf (buf, "%02x: %s", i, buf1);
	  else if (dpmi || typ == 2)
	    sprintf (buf, "%03x: %s",
		     i * 8 | (typ ? ((a_tss.tss_cs & 3) | 4) : 0),
		     buf1);
#ifndef V2DBG
	  else
	    sprintf (buf, "%02x: %-13s %s",
		     i * 8,
		     i < g_num ? gdtnames[i] : "?",
		     buf1);
#endif
	  putl (1, y++, width, buf);
	  i++;
	}
      else
	putl (1, y++, width, "");
  }

  if (help_pane_active)
  {
    /* Show help info.  */
    int y = 1, width = cols - 19, i = help_pane_origin;
    int count = sizeof (helptext) / sizeof (char *);

    while (y <= toplines)
      if (i < count)
	putl (1, y++, width, helptext[i++]);
      else
	putl (1, y++, width, "");
  }

  if (module_pane_active)
  {
    int y = 1, width = cols - 19, i = module_pane_origin;

    while (y <= toplines)
      if (i < module_text_count)
	putl (1, y++, width, module_pane_text[i++]);
      else
	putl (1, y++, width, "");
  }

  if (source_pane_active)
  {
    int y = 1, width = cols - 19, i = source_pane_origin;

    if (source_text_count == 0)
      putl (1, y++, width, "(No module selected)");
    while (y <= toplines)
      {
	screen_attr = screen_attr_source;
	if (i < source_text_count)
	  {
	    word32 addr = find_source_line (i, 1);
	    char *buf;

	    buf = source_pane_text[i++];
	    if (addr)
	      {
		if (get_breakpoint (BP_Code, 0, addr) != -1)
		  screen_attr = screen_attr_break;
		if (addr == a_tss.tss_eip && strlen (buf) > 6)
		  {
		    char *newbuf = alloca (strlen (buf) + 1);
		    buf = strcpy (newbuf, buf);
		    buf[5] = 16;
		  }
	      }
	    putl (1, y++, width, buf);
	  }
	else
	  putl (1, y++, width, "");
      }
    screen_attr = screen_attr_normal;
  }

  if (watch_pane_active)
  {
    int y = toplines + 2, width = 45, i = watch_pane_origin;

    if (watch_text_count == 0)
      putl (1, y++, width, "(No watches specified)");
    while (y - (toplines + 1) <= bottomlines)
      if (i < watch_text_count)
	{
	  int32 val;
	  char *errtxt, *valtxt, *exprtxt;
	  int l;

	  if (evaluate (watch_pane_text[i], &val, &errtxt))
	    {
	      valtxt = errtxt;
	      if ((int)strlen (errtxt) > width - 5)
		{
		  valtxt = alloca (width - 4);
		  strncpy (valtxt, errtxt, width - 4);
		  valtxt[width - 5] = 0;
		}
	    }
	  else
	    {
	      valtxt = alloca (30);
	      sprintf (valtxt, "%ld (%08lx)", val, val);
	    }
	  l = width - strlen (valtxt) - 3;
	  exprtxt = alloca (l + 1);
	  strncpy (exprtxt, watch_pane_text[i], l + 1);
	  if (exprtxt[l])
	    exprtxt[l - 3] = exprtxt[l - 2] = exprtxt[l - 1] = '.',
	    exprtxt[l] = 0;
	  sprintf (buf, "%s \032 %s", exprtxt, valtxt);
	  putl (1, y++, width, buf);
	  i++;
	}
      else
	putl (1, y++, width, "");
  }

  {
    /* Highlight focus */
    screen_attr = screen_attr_focus;

    switch (pane)
      {
      case PANE_STACK:
	pane_pos = MIN (pane_pos, stack_dump_last);
	/* Fall through */
      case PANE_CODE:
      case PANE_NPX:
      case PANE_GDT:
      case PANE_IDT:
      case PANE_LDT:
      case PANE_HELP:
      case PANE_SOURCE:
      case PANE_MODULE:
      case PANE_WHEREIS:
	pane_pos = MIN (pane_pos, toplines - 1);
	highlight (1, pane_pos + 1, cols - 19);
	break;
      case PANE_REGISTER:
	pane_pos = MIN (pane_pos, toplines - 1);
	highlight (cols - 17, pane_pos + 1, 12);
	break;
      case PANE_FLAG:
	pane_pos = MIN (pane_pos, toplines - 1);
	highlight (cols - 4, pane_pos + 1, 3);
	break;
      case PANE_BREAKPOINT:
	{
	  int y;
	  if (breakpoint_origin >= breakpoint_count)
	    breakpoint_origin = breakpoint_count ? breakpoint_count - 1 : 0;
	  if (breakpoint_origin + pane_pos >= breakpoint_count)
	    pane_pos = breakpoint_count - breakpoint_origin;
	  y = 2 * pane_pos;
	  if (y + 1 < bottomlines)
	    {
	      highlight (47, toplines + 2 + y++, cols - 48);
	      highlight (47, toplines + 2 + y++, cols - 48);
	    }
	}
	break;
      case PANE_DATA:
	{
	  int x = pane_pos & 7, y = pane_pos >> 3;
	  int width1 = 2 * data_dump_size;
	  int shift = data_dump_size >> 1;  /* log2 really.  */

	  if (y < bottomlines)
	    {
	      highlight (38 + x, toplines + 2 + y, data_dump_size);
	      highlight (11 + (width1 + 1) * (x >> shift),
			 toplines + 2 + y, width1);
	    }
	}
	break;
      case PANE_WATCH:
	pane_pos = MIN (pane_pos, bottomlines - 1);
	highlight (1, toplines + 2 + pane_pos, 45);
	break;
      }
    screen_attr = screen_attr_normal;
  }

  put_screen (debug_screen_save);
}
/* ------------------------------------------------------------------------- */
/* Initialize the display structures.  Unless FIRSTTIME free the old
   dynamically allocated structures.  */

void
initdisplay (int firsttime)
{
  init_screen ();
  if (firsttime)
    debug_screen_p = 0;
  else
    {
      free (read_buffer);
      free (code_pane_pos);
      free (stack_dump_pos);
      free (debug_screen_save);
    }

  read_buffer = xmalloc (cols + 10);
  code_pane_pos = xmalloc ((toplines + 2) * sizeof (word32));
  stack_dump_pos = xmalloc ((toplines + 2) * sizeof (word32));
  debug_screen_save = get_screen ();
  redraw (1);
  if (!dual_monitor_p)
    {
      /* Patch cursor pos.  */
      debug_screen_save[1] = debug_screen_save[2] = 0;
    }
}
/* ------------------------------------------------------------------------- */
static void
setup_save (int dummy)
{
  FILE *f;
  int i;

  f = fopen (setupfilename, "wt");
  if (f)
    {
      fprintf (f, "# Sally Full Screen Debugger version %.2f\n",
	       FULLSCR_VERSION);
      fprintf (f, "\n[Colours]\n");
      fprintf (f, "%d %d %d %d %d %d %d %d %d %d %d %d\n",
	       screen_attr_normal,
	       screen_attr_source,
	       screen_attr_focus,
	       screen_attr_break,
	       screen_attr_message,
	       screen_attr_error,
	       screen_attr_menu,
	       screen_attr_menufocus,
	       screen_attr_editframe,
	       screen_attr_edittxt,
	       screen_attr_editfield,
	       screen_attr_editfocus);
      if (breakpoint_count > 0)
	{
	  BP_ENTRY *bep = breakpoint_table;
	  fprintf (f, "\n[Breakpoints]\n%d\n", breakpoint_count);
	  for (i = 0; i < breakpoint_count; i++, bep++)
	    {
	      char *name, *buf;
	      word32 delta;

	      name = syms_val2name (bep->addr, &delta);
	      if (delta)
		{
		  buf = alloca (strlen (name) + 20);
		  sprintf (buf, "%s%+ld", name, delta);
		}
	      else
		buf = name;
	      fprintf (f, "%d %s %d %d %d %s\n",
		       bep->type,
		       buf,
		       bep->count,
		       bep->length,
		       bep->disabled,
		       bep->condition ? : "");
	    }
	}
      fclose (f);
    }
  else
    message (CL_Error, "Cannot write to file \"%s\"", setupfilename);
}
/* ------------------------------------------------------------------------- */
static void
setup_restore (int booting)
{
  FILE *f = NULL;
  char *err = "Corrupted setup file";

  f = fopen (setupfilename, "rt");
  if (f)
    {
      char version[5], myversion[5], section[21];

      if (fscanf (f, "# Sally Full Screen Debugger version %4s", version) != 1)
	goto error;
      sprintf (myversion, "%.2f", FULLSCR_VERSION);
      if (strcmp (version, myversion))
	{
	  err = "Setup file is from a different debugger version";
	  goto error;
	}

      while (fscanf (f, "\n[%20s\n", section) == 1)
	{
	  if (stricmp (section, "Colours]") == 0)
	    {
	      int a[12];
	      if (fscanf (f, "%d %d %d %d %d %d %d %d %d %d %d %d",
			  a + 0, a + 1, a + 2, a + 3, a + 4, a + 5,
			  a + 6, a + 7, a + 8, a + 9, a + 10, a + 11) != 12)
		goto error;
	      screen_attr_normal = a[0];
	      screen_attr_source = a[1];
	      screen_attr_focus = a[2];
	      screen_attr_break = a[3];
	      screen_attr_message = a[4];
	      screen_attr_error = a[5];
	      screen_attr_menu = a[6];
	      screen_attr_menufocus = a[7];
	      screen_attr_editframe = a[8];
	      screen_attr_edittxt = a[9];
	      screen_attr_editfield = a[10];
	      screen_attr_editfocus = a[11];
	    }
	  else if (stricmp (section, "Breakpoints]") == 0)
	    {
	      int b, count;
	      char addr[1000], cond[1000];
	      int32 res;
	      char *errtxt;

	      if (fscanf (f, "%d", &count) != 1)
		goto error;
	      while (count-- > 0)
		{
		  int a[4];
		  if (fscanf (f, "%d %999s %d %d %d",
			      a + 0, addr, a + 1, a + 2, a + 3) != 5)
		    goto error;
		  fgets (cond, sizeof (cond), f);
		  if (evaluate (addr, &res, &errtxt))
		  {
		    if (booting)
		      fprintf (stderr,
			       "Ignoring out-of-date breakpoint at %s.\n",
			       addr);
		    else
		      message (CL_Error,
			       "Ignoring out-of-date breakpoint at %s",
			       addr);
		  }
		  else
		    {
		      b = set_breakpoint (a[0], a[2], res);
		      breakpoint_table[b].count = a[1];
		      breakpoint_table[b].disabled = a[3];
		      breakpoint_table[b].condition
			= cond[0] ? strdup (cond) : 0;
		    }
		}
	    }
	  else
	    goto error;
	}
      fclose (f);
      return;
    }
  else
    err = "Cannot read setup file";

 error:
  if (f) fclose (f);
  if (booting)
    fprintf (stderr, "%s.\n", err);
  else
    message (CL_Error, err);
}
/* ------------------------------------------------------------------------- */
static void
initialize_module ()
{
  char *s;
  int i;

  module_text_count = 1;
  module_pane_text = xmalloc (module_text_count * sizeof (char *));
  module_line_numbers = xmalloc (module_text_count * sizeof (int));
  i = 0;
  s = syms_module (i++);
  while (s)
  {
    if (strcmp ("fake", s) && cache_fopen (s) != NULL)
      {
	module_pane_text
	  = xrealloc (module_pane_text, ++module_text_count * sizeof (char *));
	module_line_numbers
	  = xrealloc (module_line_numbers, module_text_count * sizeof (int));
	module_pane_text[module_text_count - 1] = strdup (s);
	module_line_numbers[module_text_count - 1] = 1;
      }
    s = syms_module (i++);
  }
  if (module_text_count == 1)
    module_pane_text[0] = strdup ("No modules with debug information.");
  else
    {
      module_pane_text[0] = strdup ("Modules listing:");
      pane_positions[PANE_MODULE] = 1;
    }
  module_line_numbers[0] = 1;
  source_pane_module = 0;
}
/* ---------------------------------------------------------------------- */
/* Select a pane, i.e., make it active.  */
static void
select_pane (int p)
{
  pane_positions[pane] = pane_pos;

  register_pane_active   = 1 || (p == PANE_REGISTER);
  flag_pane_active       = 1 || (p == PANE_FLAG);
  breakpoint_pane_active = 1 || (p == PANE_BREAKPOINT);
  switch (p)
    {
    case PANE_REGISTER:
    case PANE_FLAG:
    case PANE_BREAKPOINT:
      break;
    case PANE_DATA:
    case PANE_WATCH:
      dlpane = p;
      data_pane_active       = (p == PANE_DATA);
      watch_pane_active      = (p == PANE_WATCH);
      break;
    default:
      ulpane = p;
      code_pane_active       = (p == PANE_CODE);
      npx_pane_active        = (p == PANE_NPX);
      stack_pane_active      = (p == PANE_STACK);
      info_pane_active       = (p == PANE_INFO);
      whereis_pane_active    = (p == PANE_WHEREIS);
      gdt_pane_active        = (p == PANE_GDT);
      idt_pane_active        = (p == PANE_IDT);
      ldt_pane_active        = (p == PANE_LDT);
      help_pane_active       = (p == PANE_HELP);
      module_pane_active     = (p == PANE_MODULE);
      source_pane_active     = (p == PANE_SOURCE);
      break;
    }
  pane = p;
  pane_pos = pane_positions[pane];
}
/* ------------------------------------------------------------------------- */
/* Initialize all data structures.  */

static void
initialize ()
{
  int i;

  terminated = 0;
  dpmi = (_go32_info_block.run_mode == _GO32_RUN_MODE_DPMI);
  asm volatile ("sgdt (_gdtlinear)");
  asm volatile ("sidt (_idtlinear)");

  pane = PANE_CODE;
  pane_pos = 0;
  for (i = 0; i < PANECOUNT; i++) pane_positions[i] = 0;
  select_pane (PANE_DATA);
  select_pane (PANE_CODE);

  data_dump_origin = areas[A_data].first_addr;
  data_dump_size = 1;
  code_dump_origin = a_tss.tss_eip;
  register_pane_origin = 0;
  flag_pane_origin = 0;
  npx_pane_origin = 0;
  stack_dump_origin = 0;
  breakpoint_origin = 0;
  breakpoint_count = 0;
  breakpoint_table = xmalloc (breakpoint_count * sizeof (BP_ENTRY));
  whereis_pane_origin = 0;
  whereis_text_count = 0;
  whereis_pane_text = xmalloc (whereis_text_count * sizeof (char *));
  gdt_pane_origin = 0;
  ldt_pane_origin = 0;
  idt_pane_origin = 0;
  help_pane_origin = 0;
  module_pane_origin = 0;
  initialize_module ();
  source_pane_origin = 0;
  source_text_count = 0;
  source_pane_text = xmalloc (source_text_count * sizeof (char *));
  watch_pane_origin = 0;
  watch_text_count = 0;
  watch_pane_text = xmalloc (watch_text_count * sizeof (char *));

  init_colours ();
  if (access (setupfilename, R_OK) == 0)
    setup_restore (1);
  initdisplay (1);
  message (CL_Info, "Sally Full Screen Debugger");
}
/* ------------------------------------------------------------------------- */
static void
shell (int dummy)
{
  ScreenAttrib = 7;
  ScreenClear ();
  system ("");
}
/* ------------------------------------------------------------------------- */
/* Standard movement commands, a common way to handle a number of keys:
   up, down, left right, home, end, pgup, pgdn.  */

static int
standardmovement (int key, int count, int lines, int *origin)
{
  int zero = 0;
  int res = 1, no;

  if (origin == 0) origin = &zero;
  no = pane_pos + *origin;
  switch (key)
    {
    case K_Left:
    case K_ELeft:
    case K_Up:
    case K_EUp:
      if (pane_pos > 0)
	pane_pos--;
      else if (*origin > 0)
	(*origin)--;
      break;
    case K_Right:
    case K_ERight:
    case K_Down:
    case K_EDown:
      if (no < count - 1)
      {
	if (pane_pos == lines - 1)
	  (*origin)++;
	else
	  pane_pos++;
      }
      break;
    case K_Home:
    case K_EHome:
      pane_pos = (*origin) = 0;
      break;
    case K_End:
    case K_EEnd:
      if (count > lines)
	pane_pos = lines - 1, *origin = count - lines;
      else
	pane_pos = count ? count - 1 : 0, *origin = 0;
      break;
    case K_PageUp:
    case K_EPageUp:
      if (*origin == 0)
	pane_pos = 0;
      else
	if (*origin >= lines)
	  *origin -= lines;
	else
	  pane_pos -= (lines - *origin), *origin = 0;
      break;
    case K_PageDown:
    case K_EPageDown:
      if (*origin + lines >= count)
	pane_pos = count ? count - 1 - *origin : 0;
      else
	*origin += lines;
      break;
    case PK_Redraw:
      break;
    default:
      res = 0;
    }
  pane_pos = MAX (pane_pos, 0);
  no = pane_pos + *origin;
  if (pane_pos >= lines)
    {
      *origin = no - lines + 1;
      pane_pos = lines - 1;
    }
  if (no >= count)
    {
      if (*origin >= count)
	*origin = pane_pos = 0;
      else
	pane_pos -= (no + 1 - count);
    }
  return res;
}
/* ------------------------------------------------------------------------- */
/* Handle commands local to the code pane.  */

static void
code_pane_command (int key)
{
  int b;

  switch (key)
    {
    case PK_Redraw:
      if (code_pane_pos[pane_pos] <= code_dump_last)
	break;
      /* Fall through.  */
    case K_Up:
    case K_EUp:
      if (pane_pos > 0)
	pane_pos--;
      else
	code_pane_goto (code_skip (code_dump_origin, -1));
      break;
    case K_Down:
    case K_EDown:
      if (pane_pos < toplines - 1)
	pane_pos++;
      else
	code_dump_origin =
	  code_pane_pos[0] == code_pane_pos[1]
	    ? code_pane_pos[2] : code_pane_pos[1];
      break;
    case K_PageUp:
    case K_EPageUp:
      code_dump_origin = code_skip (code_dump_origin, -toplines);
      break;
    case K_PageDown:
    case K_EPageDown:
      code_dump_origin = code_dump_last + 1;
      break;
    case K_Control_Left:
    case K_Control_ELeft:
      code_dump_origin--;
      break;
    case K_Control_Right:
    case K_Control_ERight:
      code_dump_origin++;
      break;
    case K_F2:
      b = get_breakpoint (BP_Code, 0, code_pane_pos[pane_pos]);
      if (b != -1)
	reset_breakpoint (b);
      else
	set_breakpoint (BP_Code, 0, code_pane_pos[pane_pos]);
      break;
    case K_F4:
      b = set_breakpoint (BP_Code, 0, code_pane_pos[pane_pos]);
      breakpoint_table[b].temporary = 1;
      step (R_Run);
      reset_breakpoint (b);
      break;
    case K_Control_O:
      code_pane_goto (a_tss.tss_eip);
      break;
    case K_Control_N:
      a_tss.tss_eip = code_pane_pos[pane_pos];
      break;
    case K_Control_G:
      {
	int ok, res;
	res = read_eval (&ok, "");
	if (ok)
	  code_pane_goto (res);
      }
      break;
    case K_Return:
      {
	code_pane_goto (code_pane_pos[pane_pos]);
	if (source_pane_module)
	  select_pane (PANE_SOURCE);
      }
      break;
    }
  redraw (0);
}
/* ------------------------------------------------------------------------- */
static word32
register_set (int reg, int rel, word32 val)
{
  switch (regs_type[reg])
    {
    case 's':
      if (rel) val += *(unsigned short *)(regs_addr[reg]);
      return *(unsigned short *)(regs_addr[reg]) = (unsigned short)val;
    case 'f':
      if (rel) val += *(regs_addr[reg]);
      return a_tss.tss_eflags = (a_tss.tss_eflags & ~0xed5) | (val & 0xed5);
    default:
      if (rel) val += *(regs_addr[reg]);
      return *regs_addr[reg] = val;
    }
}

/* Handle commands local to the register pane.  */

static void
register_pane_command (int key)
{
  int no = register_pane_origin + pane_pos;

  if (!standardmovement (key, sizeof (regs_type) - 1,
			 toplines, &register_pane_origin))
    switch (key)
      {
      case K_Control_D:
	(void) register_set (no, 1, -1);
	break;
      case PK_Control_I:
	(void) register_set (no, 1, +1);
	break;
      case K_Control_N:
	(void) register_set (no, 0, -register_set (no, 1, 0));
	break;
      case K_Control_Z:
	(void) register_set (no, 0, 0);
	break;
      case ' ' ... '~':
	{
	  int res, ok;
	  char s[2];

	  s[0] = key; s[1] = '\0';
	  res = read_eval (&ok, key == '=' ? "" : s);
	  if (ok)
	    (void) register_set (no, 0, res);
	}
      }
  redraw (0);
}
/* ------------------------------------------------------------------------- */
/* Handle commands local to the flag pane.  */

static void
flag_pane_command (int key)
{
  int no = flag_pane_origin + pane_pos;

  if (!standardmovement (key, 8, toplines, &flag_pane_origin))
    switch (key)
      {
      case K_Space:
      case '+':
      case '-':
      case 't':
      case 'T':
	a_tss.tss_eflags ^= flag_bits[no];
	break;
      case '1':
      case 's':
      case 'S':
	a_tss.tss_eflags |= flag_bits[no];
	break;
      case '0':
      case 'r':
      case 'R':
	a_tss.tss_eflags &= ~flag_bits[no];
	break;
      }
  redraw (0);
}
/* ------------------------------------------------------------------------- */
/* Handle commands local to the breakpoint pane.  */

static void
breakpoint_pane_command (int key)
{
  int b = breakpoint_count ? pane_pos + breakpoint_origin : -1;
  int last = breakpoint_count && (b == breakpoint_count - 1);
  BP_ENTRY *bep = breakpoint_table + b;

  switch (key)
    {
    case K_Home:
    case K_EHome:
    case PK_Redraw:
      breakpoint_origin = pane_pos = 0;
      break;
    case K_Delete:
    case K_EDelete:
    case K_Control_C:
      if (b != -1)
	reset_breakpoint (b);
      if (!last)
	break;
      /* else fall through */
    case K_Left:
    case K_ELeft:
    case K_Up:
    case K_EUp:
      if (pane_pos > 0)
	pane_pos--;
      else
	if (breakpoint_origin > 0)
	  breakpoint_origin--;
      break;
    case K_Right:
    case K_ERight:
    case K_Down:
    case K_EDown:
      if (!last)
      {
	if (pane_pos < bottomlines / 2 - 1)
	  pane_pos++;
	else
	  breakpoint_origin++;
      }
      break;
    case K_Control_G:
      if (b != -1)
	if (bep->type == BP_Code)
	  {
	    code_pane_goto (bep->addr);
	    if (pane != PANE_CODE && pane != PANE_SOURCE)
	      select_pane (source_pane_module ? PANE_SOURCE : PANE_CODE);
	  }
      break;
    case K_Return:
      if (b != -1 && bep->type == BP_Code)
	{
	  int i;
	  unsigned long delta;
	  char *name;

	  static EDIT_ITEM bpedit[] = {
	    {"Address ....: ", 0},
	    {"Condition ..: ", 0},
	    {"Count ......: ", 0},
	    {0, 0}};

	  for (i = 0; bpedit[i].txt; i++) bpedit[i].data = alloca (cols);

	  name = syms_val2name (bep->addr, &delta);
	  if (delta)
	    sprintf (bpedit[0].data, "%s%+ld", name, delta);
	  else
	    strcpy (bpedit[0].data, name);
	  strcpy (bpedit[1].data, bep->condition ? : "");
	  if (bep->count)
	    sprintf (bpedit[2].data, "%d", bep->count);
	  else
	    bpedit[2].data[0] = 0;

	  if (edit ("Edit Breakpoint Data", bpedit, 1))
	    {
	      char *errtxt;
	      long newcount, newaddr;
	      int bad = 0;

	      /* Remove leading space on items.  */
	      for (i = 0; bpedit[i].txt; i++)
		while (isspace (bpedit[i].data[0])) bpedit[i].data++;

	      if (evaluate (bpedit[0].data, &newaddr, &errtxt))
		{
		  message (CL_Error, "Address not understood.");
		  bad = 1;
		}
	      if (bpedit[2].data[0] == 0)
		newcount = 0;
	      else if (evaluate (bpedit[2].data, &newcount, &errtxt))
		{
		  message (CL_Error, "Count not understood.");
		  bad = 1;
		}

	      if (!bad)
		{
		  bep->addr = newaddr;
		  free (bep->condition);
		  bep->condition
		    = bpedit[1].data[0] ? strdup (bpedit[1].data) : 0;
		  bep->count = newcount;
		}
	    }
	}
      break;
    }
  redraw (0);
}
/* ------------------------------------------------------------------------- */
/* Handle commands local to the data pane.  */

static void
data_pane_command (int key)
{
  word32 v = data_dump_origin + pane_pos;
  BP_TYPE t;
  int b;

  switch (key)
    {
    case PK_Redraw:
      while (v > data_dump_last)
	{
	  v -= 8;
	  pane_pos -= 8;
	}
      break;
    case K_Up:
    case K_EUp:
      if (pane_pos >= 8)
	pane_pos -= 8;
      else
	data_dump_origin -= 8;
      break;
    case K_Down:
    case K_EDown:
      if (v + 8 <= data_dump_last)
	pane_pos += 8;
      else
	data_dump_origin += 8;
      break;
    case K_Left:
    case K_ELeft:
      if (pane_pos > 0)
	pane_pos -= data_dump_size;
      else
	data_dump_origin -= 8, pane_pos = 8 - data_dump_size;
      break;
    case K_Right:
    case K_ERight:
      if (v + data_dump_size <= data_dump_last)
	pane_pos += data_dump_size;
      else
	data_dump_origin += 8, pane_pos -= (8 - data_dump_size);
      break;
    case K_PageUp:
    case K_EPageUp:
      data_dump_origin -= (data_dump_last + 1 - data_dump_origin);
      break;
    case K_PageDown:
    case K_EPageDown:
      data_dump_origin = data_dump_last + 1;
      break;
    case K_Control_Left:
    case K_Control_ELeft:
      data_dump_origin--;
      break;
    case K_Control_Right:
    case K_Control_ERight:
      data_dump_origin++;
      break;
    case K_Control_S:
      data_dump_origin = a_tss.tss_esp;
      pane_pos = 0;
      break;
    case K_Control_C:
      data_dump_origin = a_tss.tss_eip;
      data_dump_size = 1;
      pane_pos = 0;
      break;
    case K_Control_B:
      data_dump_size = 1;
      break;
    case K_Control_W:
      data_dump_size = 2;
      break;
    case K_Control_L:
      data_dump_size = 4;
      break;
    case K_Alt_F2:
    case K_F2:
      t = key == K_F2 ? BP_Write : BP_Read;
      b = get_breakpoint (t, data_dump_size, v);
      if (b == -1)
	set_breakpoint (t, data_dump_size, v);
      else
	reset_breakpoint (b);
      break;
    case K_Control_G:
      {
	int ok, res;
	res = read_eval (&ok, "");
	if (ok)
	  {
	    data_dump_origin = res;
	    pane_pos = 0;
	  }
	break;
      }
    case ' ' ... '~':
      {
	int res, ok, bad = 0;
	char s[2], *p, *p0, q;

	s[0] = key; s[1] = '\0';
	if (!read_string (key == '=' ? "" : s))
	  {
	    p = read_buffer;
	    do
	      {
		while (*p == ' ') p++;
		switch (*p)
		  {
		  case '\0':
		    break;
		  case '\'':
		  case '"':
		    if (data_dump_size != 1)
		      {
			*p = 0;
			message (CL_Error,
				 "Strings must be entered in byte mode");
			break;
		      }
		    q = *p++;
		    p0 = p;
		    while (*p != q && *p) p++;
		    if (*p)
		      {
			while (p0 != p)
			  {
			    if (valid_addr (v, 1))
			      write_child (v, p0, 1);
			    else
			      bad = 1;
			    p0++, v++;
			  }
			if (q == '"')
			  {
			    if (valid_addr (v, 1))
			      write_child (v, s + 1, 1);
			    else
			      bad = 1;
			    v++;
			  }
			p++;
		      }
		    else
		      message (CL_Error, "String constant not terminated");
		    break;
		  default:
		    p0 = p;
		    while (*p != ',' && *p) p++;
		    q = *p;
		    *p = 0;
		    res = eval (p0, &ok);
		    if (ok)
		      {
			*p = q;
			if (valid_addr (v, data_dump_size))
			  write_child (v, &res, data_dump_size);
			else
			  bad = 1;
			v += data_dump_size;
		      }
		  }
		if (*p == ',') p++;
	      }
	    while (*p);
	    if (bad)
	      message (CL_Error, "Part of the data could not be written");
	  }
      }
    }
  pane_pos &= ~(data_dump_size - 1);
  redraw (0);
}
/* ------------------------------------------------------------------------- */
/* Handle commands local to the npx pane.  */

static void
npx_pane_command (int key)
{
  int base = 3;
  int no = npx_pane_origin + pane_pos;
  int reg = no - base, rotreg, tag;
  int regp = (reg >= 0);

  rotreg = (reg + (npx.status >> 11)) & 7;
  tag = (npx.tag >> (rotreg * 2)) & 3;

  if (!standardmovement (key, base + 8, toplines, &npx_pane_origin))
    switch (key)
      {
      case K_Control_C:
      case K_Control_E:
      case K_Delete:
      case K_EDelete:
	if (regp)
	  {
	    tag = 3;
	    memset (&npx.reg[reg], 0, sizeof (NPXREG));
	  }
	break;
      case K_Control_Z:
	if (regp)
	  {
	    tag = 1;
	    memset (&npx.reg[reg], 0, sizeof (NPXREG));
	  }
	break;
      case K_Control_N:
	if (regp)
	  npx.reg[reg].sign = !npx.reg[reg].sign;
	break;
      case ' ' ... '~':
	if (regp)
	  {
	    char s[2], *endp, *p;
	    long double d;

	    s[0] = key; s[1] = '\0';
	    if (!read_string (key == '=' ? "" : s) && read_buffer[0] != '\0')
	      {
		p = read_buffer;
		while (*p == ' ')
		  p++;
		if (*p == '\0')
		  break;
		strlwr (p);
		if (strcmp (p, "+inf") == 0
		    || strcmp (p, "inf") == 0
		    || strcmp (p, "-inf") == 0)
		  {
		    tag = 2;
		    npx.reg[reg].exponent = 0x7fff;
		    npx.reg[reg].sig3 = 0x8000;
		    npx.reg[reg].sig2
		      = npx.reg[reg].sig1
			= npx.reg[reg].sig0 = 0;
		    npx.reg[reg].sign = (*p == '-');
		  }
		else
		  {
		    d = _strtold (p, &endp);
		    if (*p != '\0' && *endp)
		      message (CL_Error, "Expression not understood");
		    else
		      {
			/*  Fix -Wstrict-aliasing.  */
			union {
			  long double d_value;
			  NPXREG r_value;
			} npx_reg_union;
			npx_reg_union.d_value = d;
			npx.reg[reg] = npx_reg_union.r_value;
			tag = (d == 0.0);
			npx.reg[reg].sign = (*p == '-'); /* for -Zero */
		      }
		  }
	      }
	  }
      }
  if (regp)
    npx.tag = (npx.tag & ~(3 << (rotreg * 2))) | (tag << (rotreg * 2));
  redraw (0);
}
/* ------------------------------------------------------------------------- */
/* Handle commands local to the stack pane.  */

static void
stack_pane_command (int key)
{
  switch (key)
    {
    case K_PageDown:
    case K_EPageDown:
    case K_End:
    case K_EEnd:
      {
	int i;

	if (key == K_End || key == K_EEnd)
	  i = 10000;  /* As good as we get.  */
	else
	  i = toplines;
	while (i-- > 0 && (pane_pos < stack_dump_last || stack_dump_more))
	  if (pane_pos < stack_dump_last)
	    pane_pos++;
	  else
	    {
	      (void) standardmovement (K_Down,
				       pane_pos + stack_dump_origin + 2,
				       toplines, &stack_dump_origin);
	      redraw (0);
	    }
	break;
      }
    case K_Return:
      code_pane_goto (stack_dump_pos[pane_pos]);
      select_pane (PANE_CODE);
      break;
    default:
      {
	int more = pane_pos < stack_dump_last || stack_dump_more;
	(void) standardmovement (key, pane_pos + stack_dump_origin + 1 + more,
				 toplines, &stack_dump_origin);
	break;
      }
    }
  redraw (0);
}
/* ------------------------------------------------------------------------- */
/* Handle commands local to the info pane.  */

static void
info_pane_command (int key)
{
  /* No keys recognized.  */
  redraw (0);
}
/* ------------------------------------------------------------------------- */
/* Handle the insertion of a new symbol.  */

static void
fullscr_listwild_handler (word32 addr, char typ, char *name,
			  char *filename, int linenum)
{
  char *s;

  s = alloca (cols + strlen (name) + (filename ? strlen (filename) : 0));
  sprintf (s, "0x%08lx %c %s", addr, typ, name);
  if (filename)
    sprintf (s + strlen (s), ", line %d of %s", linenum, filename);
  whereis_pane_text = xrealloc (whereis_pane_text,
				(++whereis_text_count) * sizeof (char *));
  whereis_pane_text[whereis_text_count - 1] = strdup (s);
}

/* Handle commands local to the whereis pane.  */

static void
whereis_pane_command (int key)
{
  int no = whereis_pane_origin + pane_pos;

  if (!standardmovement (key, whereis_text_count,
			 toplines, &whereis_pane_origin))
    switch (key)
      {
      case K_Return:
	if (whereis_text_count)
	  {
	    char *endp, typ;
	    unsigned long ul;

	    typ = whereis_pane_text[no][11];
	    ul = strtoul (whereis_pane_text[no], &endp, 16);
	    switch (toupper (typ))
	      {
	      case 'T':
		code_pane_goto (ul);
		select_pane (PANE_CODE);
		break;
	      case 'B':
	      case 'D':
		data_dump_origin = ul;
		pane_positions[PANE_DATA] = 0;
		break;
	      }
	  }
	break;
      case ' ' ... '~':
	{
	  char s[2], *p;

	  s[0] = key; s[1] = '\0';
	  if (!read_string (key == '=' ? "" : s) && read_buffer[0] != '\0')
	    {
	      while (whereis_text_count > 0)
		free (whereis_pane_text[--whereis_text_count]);
	      whereis_pane_text = xrealloc (whereis_pane_text, 0);

	      p = read_buffer;
	      while (*p == ' ') p++;
	      syms_listwild (p, &fullscr_listwild_handler);
	      pane_pos = whereis_pane_origin = 0;
	      if (whereis_text_count >= 10)
		message (CL_Info, "There were %d symbols that matched",
			 whereis_text_count);
	    }
	}
      }
  redraw (0);
}
/* ------------------------------------------------------------------------- */
/* Handle commands local to the GDT/IDT/LDT panes.  */

static void
gildt_pane_command (int key)
{
  int i, typ, no, count, *originp;
  GDT_S descr;
  word32 base;

  if (gdt_pane_active)
    typ = 0,
    count = (gdtlinear.limit + 1) / 8,
    base = gdtlinear.base,
    originp = &gdt_pane_origin;
  else if (idt_pane_active)
    typ = 1,
    count = MIN ((idtlinear.limit + 1) / 8, 0x100),
    base = idtlinear.base,
    originp = &idt_pane_origin;
  else if (ldt_pane_active)
    typ = 2,
    count = (ldtlinear.limit + 1) / 8,
    base = ldtlinear.base,
    originp = &ldt_pane_origin;
  else
    abort ();

  no = pane_pos + *originp;
  if (!standardmovement (key, count, toplines, originp))
    switch (key)
      {
      case K_Return:
	i = getdescriptor (base, no, &descr);
	if (i >= 0 && ((allowed_descriptors[typ] >> i) & 1))
	  switch (i)
	    {
	    case 0x09:
	    case 0x0b:
	      show_tss (&descr);
	      break;
	    default:;
	    }
	break;
      }
  redraw (0);
}
/* ------------------------------------------------------------------------- */
/* Handle commands local to the help pane.  */

static void
help_pane_command (int key)
{
  (void) standardmovement (key, sizeof (helptext) / sizeof (char *),
			   toplines, &help_pane_origin);
  redraw (0);
}
/* ------------------------------------------------------------------------- */
/* Handle commands local to the module pane.  */

static void
module_pane_command (int key)
{
  int no = module_pane_origin + pane_pos;

  if (!standardmovement (key, module_text_count,
			 toplines, &module_pane_origin))
    switch (key)
      {
      case K_Return:
	if (no)
	  {
	    select_source_file (module_pane_text[no]);
	    select_pane (PANE_SOURCE);
	  }
	break;
      }
  redraw (0);
}
/* ------------------------------------------------------------------------- */
/* Handle commands local to the source pane.  */

static void
source_pane_command (int key)
{
  word32 addr;
  int b;
  int no = source_pane_origin + pane_pos;

  if (!standardmovement (key, source_text_count,
			 toplines, &source_pane_origin))
    switch (key)
      {
      case K_F2:
	addr = find_source_line (no, 4);
	if (addr)
	  {
	    b = get_breakpoint (BP_Code, 0, addr);
	    if (b != -1)
	      reset_breakpoint (b);
	    else
	      set_breakpoint (BP_Code, 0, addr);
	  }
	break;
      case K_F4:
	addr = find_source_line (no, 4);
	if (addr)
	  {
	    b = set_breakpoint (BP_Code, 0, addr);
	    breakpoint_table[b].temporary = 1;
	    step (R_Run);
	    reset_breakpoint (b);
	  }
	break;
      case K_F7:
      case K_F8:
	if (valid_instaddr (a_tss.tss_eip))
	  {
	    int line, b2, b3;
	    word32 ebp = a_tss.tss_ebp;
	    word32 esp = a_tss.tss_esp;
	    word32 eip = a_tss.tss_eip;

	    code_pane_goto (eip);
	    no = source_pane_origin + pane_pos;
	    b3 = b2 = b = -1;

	    /* Statically next source line.  */
	    addr = find_source_line (no + 1, 10);
	    if (!addr)
	      addr = trace_till_source (eip, 0, 1);
	    if (addr)
	      {
		b2 = set_breakpoint (BP_Code, 0, addr);
		breakpoint_table[b2].temporary++;
	      }

	    /* Dynamically next source line.  */
	    if (key == K_F7 && (addr = trace_till_source (eip, 10, 1)))
	      {
		b3 = set_breakpoint (BP_Code, 0, addr);
		breakpoint_table[b3].temporary++;
	      }

	    /* Caller.  */
	    if (stack_trace (&eip, &esp, &ebp))
	      {
		if ((addr = trace_till_source (eip, 10, 0)))
		  eip = addr;
		b = set_breakpoint (BP_Code, 0, eip);
		breakpoint_table[b].temporary++;
	      }
	    step (R_Run);
	    if (b != -1) reset_breakpoint (b);
	    if (b2 != -1) reset_breakpoint (b2);
	    if (b3 != -1) reset_breakpoint (b3);
	    if (!syms_val2line (a_tss.tss_eip, &line, 1))
	      select_pane (PANE_CODE);
	  }
	break;
      case K_Return:
	if (no)
	  {
	    word32 addr;

	    /* Not all lines have line numbers so we scan a range.  */
	    addr = find_source_line (no, 10);
	    if (addr)
	      {
		code_pane_goto (addr);
		select_pane (PANE_CODE);
		break;
	      }
	    else
	      message (CL_Msg,
		       "There is no line number information for this line.");
	  }
	break;
      }
  module_line_numbers [module_pane_origin + pane_positions[PANE_MODULE]]
    = pane_pos;
  redraw (0);
}
/* ------------------------------------------------------------------------- */
/* Handle commands local to the watch pane.  */

static void
watch_pane_command (int key)
{
  int no = source_pane_origin + pane_pos;

  if (!standardmovement (key, watch_text_count,
			 toplines, &watch_pane_origin))
    switch (key)
      {
      case K_Delete:
      case K_EDelete:
	free (watch_pane_text [no]);
	watch_pane_text[no] = watch_pane_text[--watch_text_count];
	watch_pane_text
	  = xrealloc (watch_pane_text, watch_text_count * sizeof (char *));
	if (no == watch_text_count)
	  (void) standardmovement (K_Up, watch_text_count,
				   toplines, &watch_pane_origin);
	break;
      case K_Insert:
      case K_EInsert:
	key = '=';
	/* Fall through.  */
      case ' ' ... '~':
	{
	  char s[2];

	  s[0] = key; s[1] = '\0';
	  if (!read_string (key == '=' ? "" : s) && read_buffer[0] != '\0')
	    {
	      watch_pane_text
		= xrealloc (watch_pane_text,
			    ++watch_text_count * sizeof (char *));
	      watch_pane_text[watch_text_count - 1] = strdup (read_buffer);
	    }
	}
      }
  redraw (0);
}
/* ------------------------------------------------------------------------- */
/* This is the entry point of the debugger.  */

void
debugger(void)
{
  int oldpane;
  static void (*keyhandlers[PANECOUNT])(int) =
    {
      &code_pane_command,	   /* 0 */
      &register_pane_command,	   /* 1 */
      &flag_pane_command,	   /* 2 */
      &breakpoint_pane_command,	   /* 3 */
      &data_pane_command,	   /* 4 */
      &npx_pane_command,	   /* 5 */
      &stack_pane_command,	   /* 6 */
      &info_pane_command,	   /* 7 */
      &whereis_pane_command,	   /* 8 */
      &gildt_pane_command,	   /* 9 */
      &gildt_pane_command,	   /* 10 */
      &gildt_pane_command,	   /* 11 */
      &help_pane_command,	   /* 12 */
      &module_pane_command,	   /* 13 */
      &source_pane_command,	   /* 14 */
      &watch_pane_command	   /* 15 */
    };

  main_entry = syms_name2val ("_main");
  first_step = !undefined_symbol;
  initialize ();
  can_longjmp = 1;
  if (setjmp (debugger_jmpbuf))
    {
      static MENU_ITEM badmemmenu[] = {
	{"Abort -- exit debugger immediately.", 0, 0},
	{"Resume -- this may re-trigger the fault.", 0, 0},
	{0, 0, 0}};
      static int focus = 0;

      if (menu ("Debugger was denied access to some memory",
		badmemmenu, &focus) == 1)
	abort ();
    }
  oldpane = -1;

  while (1)
    {
      int key;

      if (pane == oldpane)
	pane_positions[pane] = pane_pos;
      else
	{
	  pane_pos = pane_positions[pane];
	  oldpane = pane;
	  redraw (0);
	}
      key = getxkey ();
      if (key == K_Tab && (bioskey (2) & 4))
	key = PK_Control_I;  /* Control pressed.  */
      switch (key)
	{
	case K_Tab:
	  switch (pane)
	    {
	    case PANE_DATA:
	    case PANE_WATCH:
	      pane = ulpane; break;
	    case PANE_BREAKPOINT:
	      pane = dlpane; break;
	    case PANE_REGISTER:
	    case PANE_FLAG:
	      pane++; break;
	    default:
	      pane = PANE_REGISTER;
	    }
	  break;
	case K_BackTab:
	  switch (pane)
	    {
	    case PANE_REGISTER:
	      pane = ulpane; break;
	    case PANE_FLAG:
	    case PANE_BREAKPOINT:
	      pane--; break;
	    case PANE_DATA:
	    case PANE_WATCH:
	      pane = PANE_BREAKPOINT; break;
	    default:
	      pane = dlpane;
	    }
	  break;
	case K_Alt_C:
	  select_pane (PANE_CODE);
	  break;
	case K_Alt_G:
	  select_pane (PANE_GDT);
	  break;
	case K_Alt_H:
	case K_F1:
	  select_pane (PANE_HELP);
	  break;
	case K_Alt_I:
	  select_pane (PANE_IDT);
	  break;
	case K_Alt_L:
	  select_pane (PANE_LDT);
	  break;
	case K_Alt_M:
	  select_pane (PANE_MODULE);
	  break;
	case K_Alt_N:
	  select_pane (PANE_NPX);
	  break;
	case K_Alt_S:
	  select_pane (PANE_STACK);
	  break;
	case K_Alt_W:
	  select_pane (PANE_WHEREIS);
	  break;
	case K_Alt_X:
	  user_screen ();
	  can_longjmp = 0;
	  return;
	case K_Alt_F5:
	  user_screen ();
	  (void) getxkey ();
	  debug_screen ();
	  break;
	case K_Alt_EUp:
	  if (toplines > 2)
	    {
	      toplines--, bottomlines++;
	      redraw (1);
	      keyhandlers[pane] (PK_Redraw);
	    }
	  break;
	case K_Alt_EDown:
	  if (bottomlines > 2)
	    {
	      toplines++, bottomlines--;
	      redraw (1);
	      keyhandlers[pane] (PK_Redraw);
	    }
	  break;
	case K_Alt_E:
	case K_Control_F4:
	  {
	    int res, ok;

	    res = read_eval (&ok, "");
	    if (ok)
	      message (CL_Msg, "\"%s\" -> %d (0x%08lx)",
		       read_buffer, res, (word32) res);
	    break;
	  }
	case K_Control_F7:
	  select_pane (PANE_WATCH);
	  watch_pane_command (K_Insert);
	  break;
	case K_F7:
	  if (pane == PANE_SOURCE)
	    source_pane_command (key);
	  else
	    step (R_Step);
	  break;
	case K_F8:
	  if (!first_step && pane == PANE_SOURCE)
	    source_pane_command (key);
	  else
	    step (R_Over);
	  break;
	case K_F9:
	  step (R_Run);
	  break;
	case K_F10:
	  {
	    static int focus = 0;
	    static MENU_ITEM panemenu[] = {
	      {"Help",                        &select_pane, PANE_HELP},
	      {"Disassembly",                 &select_pane, PANE_CODE},
	      {"Source Code",                 &select_pane, PANE_SOURCE},
	      {"Modules",                     &select_pane, PANE_MODULE},
	      {"C Stack",                     &select_pane, PANE_STACK},
	      {"Symbol List",                 &select_pane, PANE_WHEREIS},
	      {"Numeric Processor",           &select_pane, PANE_NPX},
	      {"Watches",                     &select_pane, PANE_WATCH},
	      {"System Info",                 &select_pane, PANE_INFO},
	      {"Data Dump",                   &select_pane, PANE_DATA},
	      {"Register",                    &select_pane, PANE_REGISTER},
	      {"Flags",                       &select_pane, PANE_FLAG},
	      {"Breakpoints",                 &select_pane, PANE_BREAKPOINT},
	      {"Global Descriptor Table",     &select_pane, PANE_GDT},
	      {"Interrupt Descriptor Table",  &select_pane, PANE_IDT},
	      {"Local Descriptor Table",      &select_pane, PANE_LDT},
	      {0, 0, 0}};

	    (void) menu ("Select Pane", panemenu, &focus);
	  }
	  break;
	case K_Alt_F10:
	  {
	    static int focus = 0;
	    static MENU_ITEM funcmenu[] = {
	      {"25 Lines",                    &screen_mode, 25},
	      {"28 Lines",                    &screen_mode, 28},
	      {"50 Lines",                    &screen_mode, 50},
	      {"Silence Speaker",             &sound, 0},
	      {"Shell",                       &shell, 0},
	      {"Edit Colours",                &edit_colours, 0},
	      {"Save Setup",                  &setup_save, 0},
	      {"Restore Setup",               &setup_restore, 0},
	      {0, 0, 0}};

	    switch (menu ("Select Function", funcmenu, &focus))
	      {
	      case 0:
	      case 1:
	      case 3:
		initdisplay (0);
		break;
	      case -1:;
		/* Nothing.  */
	      }
	    redraw (0);
	  }
	  break;
	default:
	  keyhandlers[pane] (key);
	}
    }
}
/* -------------------------------------------------------------------------
   It's a mystery to me --- the game commences
   for the usual fee --- plus expenses
   confidential information --- it's in a diary
   this is my investigation --- it's not a public inquiry

   -- Mark Knopfler, "Private Investigations"
------------------------------------------------------------------------- */
