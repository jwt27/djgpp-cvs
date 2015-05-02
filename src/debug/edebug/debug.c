/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
** Copyright (C) 1993 DJ Delorie, 334 North Rd, Deerfield NH 03037-1110
**
** This file is distributed under the terms listed in the document
** "copying.dj", available from DJ Delorie at the address above.
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <stdio.h>
#include <setjmp.h>
#include <math.h>
#include <string.h>
#include <pc.h>		/* For getkey() */

#include "ed.h"
#include "debug.h"
#include "unassmbl.h"
#include <debug/syms.h>

static char char32spc[] = "xxxúxxxúxxxúxxxùxxxúxxxúxxxúxxx ";

static char flset[] = "VMRF  NT    OFDNIETFMIZR  AC  PE  CY";
static char floff[] = "              UPID  PLNZ      PO  NC";
static char fluse[] = {1,1,0,1,0,0,1,1,1,1,1,1,0,1,0,1,0,1};

static void tssprint(TSS *t)
{
  int i;
  printf("eax=%08lx  ebx=%08lx  ecx=%08lx  edx=%08lx\n",
    t->tss_eax, t->tss_ebx, t->tss_ecx, t->tss_edx);
  printf("esi=%08lx  edi=%08lx  ebp=%08lx ",
    t->tss_esi, t->tss_edi, t->tss_ebp);
  for (i=0; i<18; i++)
    if (fluse[i])
    {
      if (t->tss_eflags & (1<<(17-i)))
        printf(" %2.2s", flset+i*2);
      else
        printf(" %2.2s", floff+i*2);
    }
  printf("\nds=%04x es=%04x fs=%04x gs=%04x ss:esp=%04x:%08lx cs=%04x\n",
    t->tss_ds, t->tss_es, t->tss_fs, t->tss_gs, t->tss_ss, t->tss_esp, t->tss_cs);
}

static void my_getline(char *buf, const char *lasttoken)
{
  int idx, i, ch, erase_it=1;

  ansi(A_green);
  printf(">>");
  ansi(A_green | A_bold);
  printf(" %s", lasttoken);
  for (i=0; lasttoken[i]; i++)
    putchar(8);
  ansi(A_white);
  fflush(stdout);
  idx = 0;

  while (1)
  {
    ch = getkey();
    if (erase_it)
    {
      for (i=0; lasttoken[i]; i++)
        putchar(' ');
      for (i=0; lasttoken[i]; i++)
        putchar(8);
    }
    switch (ch)
    {
      case 10:
      case 13:
        buf[idx] = 0;
        if (!idx && lasttoken[0])
          printf("\r  \r");
        else
          putchar('\n');
        ansi(A_grey);
        fflush(stdout);
        return;
      case 27:
      case 21:
        while (idx)
        {
          printf("\b \b");
          idx--;
        }
        fflush(stdout);
        break;
      case 8:
        if (idx)
        {
          printf("\b \b");
          fflush(stdout);
          idx--;
        }
        break;
      default:
        putchar(ch);
        fflush(stdout);
        buf[idx++] = ch;
        break;
    }
  }
}

typedef enum { Zero, Unknown, CONT, STEP, NEXT, REGS, SET, HELP, LIST,
DUMP, PRINT, QUIT, BREAK, STATUS, WHERE, DUMP_A, DUMP_B, DUMP_W, WHEREIS, XNPX,
CLS } COMMAND_TYPES;

extern struct {
  char *name;
  int size;
  int ofs;
  } regs[];


typedef struct {
        const char *cp;
        int t;
        } item;

const item cmds[] = {
        {"g", CONT},
        {"go", CONT},
        {"cont", CONT},
        {"c", CONT},
        {"step", STEP},
        {"s", STEP},
        {"next", NEXT},
        {"n", NEXT},
        {"regs", REGS},
        {"r", REGS},
        {"set", SET},
        {"help", HELP},
        {"h", HELP},
        {"?", HELP},
        {"list", LIST},
        {"l", LIST},
        {"u", LIST},
        {"dump", DUMP},
        {"d", DUMP},
        {"da", DUMP_A},
        {"db", DUMP_B},
        {"dw", DUMP_W},
        {"dd", DUMP},
        {"p", PRINT},
        {"print", PRINT},
        {"quit", QUIT},
        {"q", QUIT},
        {"break", BREAK},
        {"b", BREAK},
        {"bl", STATUS},
        {"status", STATUS},
        {"where", WHERE},
        {"whereis", WHEREIS},
        {"npx", XNPX},
        {"cls", CLS},
        {0, 0}
        };

#define dr0 edi.dr[0]
#define dr1 edi.dr[1]
#define dr2 edi.dr[2]
#define dr3 edi.dr[3]
#define dr4 edi.dr[4]
#define dr5 edi.dr[5]
#define dr6 edi.dr[6]
#define dr7 edi.dr[7]

int can_longjmp = 0;
jmp_buf debugger_jmpbuf;

static int do_where(word32 vaddr)
{
  int i;
  word32 delta;
  char *name;
  printf("0x%08lx %s", vaddr, syms_val2name(vaddr, &delta));
  name = syms_val2line(vaddr, &i, 0);
  if (name)
    printf(", line %d in file %s", i, name);
  else if (delta)
    printf("%+ld", (int32)delta);
  putchar('\n');
  return (int32)delta;
}

static int print_reason(void)
{
  int n, i, rv=0;
  i = a_tss.tss_irqn;
  if ((i == 0x21) && ((a_tss.tss_eax & 0xff00) == 0x4c00))
  {
    ansi(A_green|A_bold);
    printf("Program terminated normally, exit code is %d\n", (word8)a_tss.tss_eax);
    a_tss.tss_eip -= 2; /* point to int 21h */
    return 1;
  }
  ansi(A_red | A_bold);
  if (i != 1)
  {
    tssprint(&a_tss);
    if (i == 0x79)
      printf("Keyboard interrupt\n");
    else if (i == 0x75)
    {
      printf("Numeric Exception (");
      if ((npx.status & 0x0241) == 0x0241)
        printf("stack overflow");
      else if ((npx.status & 0x0241) == 0x0041)
        printf("stack underflow");
      else if (npx.status & 1)
        printf("invalid operation");
      else if (npx.status & 2)
        printf("denormal operand");
      else if (npx.status & 4)
        printf("divide by zero");
      else if (npx.status & 8)
        printf("overflow");
      else if (npx.status & 16)
        printf("underflow");
      else if (npx.status & 32)
        printf("loss of precision");
      printf(") at eip=0x%08lx\n", npx.eip);
      unassemble(npx.eip, 0);
    }
    else
    {
      printf("exception %d (%#02x) occurred", i, i);
      if ((i == 8) || ((i>=10) && (i<=14)))
        printf(", error code=%#lx", a_tss.tss_error);
      putchar('\n');
      rv = 1;
    }
  }
  ansi(A_cyan | A_bold);
  for (n=0; n<3; n++)
    if ((dr6 & (1<<n)) && (dr7 & (3<<(n*2))))
    {
      printf("breakpoint %d hit\n", n);
      rv = 1;
    }
  return rv;
}

static int wildlines;

static void print_wildmatch(word32 addr, char type_c, char *name, char *name2, int lnum)
{
  int key;

  if(wildlines < 0)
    return;
  if(++wildlines > 20)
  {
    printf("--- More ---");
    fflush(stdout);
    key = getkey();
    printf("\r            \r");
    switch (key)
    {
      case ' ':
        wildlines = 0;
        break;
      case 13:
        wildlines--;
        break;
      case 'q':
      case 27:
        wildlines = -1;
        return;
    }
  }
  printf("0x%08lx %c %s", addr, type_c, name);
  if (name2)
    printf(", line %d of %s", lnum, name2);
  putchar('\n');
}

void debugger(void)
{
  char buf[140], token[10];
  char buf2[140], *name, lasttoken[140];
  int i, n, s, len, rem_cmd, cmd;
  word32 vaddr, v, rem_v, olddr7;
  word32 delta;

  syms_printwhy = 1;
  dr0 = dr1 = dr2 = 0;
  dr3 = syms_name2val("_main");
  if (undefined_symbol)
    dr3 = a_tss.tss_eip;
  can_longjmp = 1;
  setjmp(debugger_jmpbuf);
  rem_cmd = Zero;
  lasttoken[0] = 0;
  while (1)
  {
    int found;
    undefined_symbol = 0;
    my_getline(buf, lasttoken);
    token[0] = 0;
    if (sscanf(buf, "%s %[^\n]", token, buf) < 2)
      buf[0] = 0;
    if (token[0])
      strcpy(lasttoken, token);
    cmd = rem_cmd;
    found = 0;
    for (i=0; cmds[i].cp; i++)
      if (strcmp(cmds[i].cp, token) == 0)
      {
        cmd = cmds[i].t;
        found = 1;
      }
    if (!found && token[0])
      cmd = Unknown;
    if (rem_cmd != cmd)
      vaddr = a_tss.tss_eip;

    switch (cmd)
    {
      case HELP:
        printf("Commands:\n");
        printf("go <v>\tg\tgo, stop at <v>\n");
        printf("cont\tc\tcontinue execution\n");
        printf("step\ts\tstep through current instruction\n");
        printf("next\tn\tstep to next instruction\n");
        printf("list\tl u\tlist instructions (takes addr, count)\n");
        printf("dump\td\tdump memory (takes addr, count)\n");
        printf("print\tp\tprint value of expression (takes expr)\n");
        printf("break\tb\tset breakpoint (takes which, addr)\n");
        printf("status\t\tbreakpoint status\n");
        printf("regs\tr\tprint registers\n");
        printf("set\t\tset register/memory\n");
        printf("npx\t\tdisplay 80387 contents\n");
        printf("where\t\tdisplay list of active functions\n");
        printf("whereis\t\tfind a symbol/location (takes wildcard or value)\n");
        printf("cls\t\tclear screen\n");
        printf("help\th,?\tprint help\n");
        printf("quit\tq\tquit\n");
        break;

      case CONT:
        sscanf(buf, "%s", buf);
        if (buf[0])
        {
          v = syms_name2val(buf);
          if (undefined_symbol)
            break;
          dr3 = v;
          dr7 |= 0xc0;
        }
        else
          dr7 &= ~0xc0;
        olddr7 = dr7;
        dr7 = 0;
        a_tss.tss_eflags |= 0x0100;
        run_child();
        dr7 = olddr7;
        if (a_tss.tss_irqn == 1)
        {
          a_tss.tss_eflags &= ~0x0100;
          a_tss.tss_eflags |= 0x10000;
          run_child();
          if (a_tss.tss_irqn == 1)
            tssprint(&a_tss);
        }
        print_reason();
        dr3 = unassemble(a_tss.tss_eip, 1);
        break;

      case STEP:
        if (rem_cmd != cmd)
          n = 1;
        sscanf(buf, "%d", &n);
        a_tss.tss_eflags |= 0x0100;
        for (i=0; i<n; i++)
        {
          int q;
          olddr7 = dr7;
          dr7 = 0;
          run_child();
          dr7 = olddr7;
          q = print_reason();
          dr3 = unassemble(a_tss.tss_eip, 1);
          if ((a_tss.tss_irqn != 1) || q)
            break;
        }
        a_tss.tss_eflags &= ~0x0100;
        break;

      case NEXT:
        if (rem_cmd != cmd)
          n = 1;
        sscanf(buf, "%d", &n);
        for (i=0; i<n; i++)
        {
          olddr7 = dr7;
          dr7 &= ~0xc0;
          dr7 |= 0xc0;
          if (last_unassemble_unconditional ||
              last_unassemble_jump)
            a_tss.tss_eflags |= 0x0100; /* step */
          else
            a_tss.tss_eflags &= ~0x0100;
          run_child();
          dr7 = olddr7;
          print_reason();
          dr3 = unassemble(a_tss.tss_eip, 1);
          if (a_tss.tss_irqn != 1)
            break;
        }
        a_tss.tss_eflags &= ~0x0100;
        break;

      case WHERE:
        lasttoken[0] = 0;
        v = a_tss.tss_ebp;
        vaddr = a_tss.tss_eip;
        delta = do_where(vaddr);
        if (delta == 0) /* try to find out where we just came from */
        {
          read_child(a_tss.tss_esp, &rem_v, 4);
          if (rem_v)
            do_where(rem_v);
        }
        do {
          if (v == 0)
            break;
          if (read_child(v, &rem_v, 4))
            break;
          if (rem_v == 0)
            break;
          if (read_child(v+4, &vaddr, 4))
            break;
          do_where(vaddr);
          v = rem_v;
        } while ((v>=a_tss.tss_esp) && (v<0x80000000UL));
        break;

      case WHEREIS:
        lasttoken[0] = 0;
        sscanf(buf, "%s", buf2);
        if (strpbrk(buf2, "*?"))
        {
          wildlines = 0;
          syms_listwild(buf2, print_wildmatch);
          break;
        }
        if (buf2[0])
          vaddr = syms_name2val(buf2);
        if (undefined_symbol)
          break;
        name = syms_val2name(vaddr, &delta);
        printf("0x%08lx %s", vaddr, name);
        if (delta)
          printf("+%lx", delta);
        name = syms_val2line(vaddr, &i, 0);
        if (name)
          printf(", line %d in file %s", i, name);
        putchar('\n');
        break;

      case LIST:
        if (rem_cmd != cmd)
          n = 10;
        buf2[0] = 0;
        sscanf(buf, "%s %d", buf2, &n);
        if (buf2[0] && strcmp(buf2, "."))
          vaddr = syms_name2val(buf2);
        if (undefined_symbol)
          break;
        for (i=0; i<n; i++)
        {
          vaddr = unassemble(vaddr, 0);
          i += last_unassemble_extra_lines;
        }
        break;

      case DUMP_A:
        buf2[0] = 0;
        sscanf(buf, "%s %d", buf2, &n);
        if (buf2[0])
          vaddr = syms_name2val(buf2);
        if (undefined_symbol)
          break;
        while (1)
        {
          word8 ch;
          if (vaddr == 0)
          {
            printf("<bad address>\n");
            break;
          }
          if (read_child(vaddr, &ch, 1))
            break;
          if (ch == 0)
          {
            putchar('\n');
            break;
          }
          if (ch < ' ')
            printf("^%c", ch+'@');
          else if ((ch >= ' ') && (ch < 0x7f))
            putchar(ch);
          else if (ch == 0x7f)
            printf("^?");
          else if ((ch >= 0x80) && (ch < 0xa0))
            printf("M-^%c", ch-0x80+'@');
          else if (ch >= 0xa0)
            printf("M-%c", ch-0x80);
          vaddr++;
        }
        break;
      case DUMP:
      case DUMP_B:
      case DUMP_W:
        if (rem_cmd != cmd)
          n = 4;
        buf2[0] = 0;
        sscanf(buf, "%s %d", buf2, &n);
        if (buf2[0])
          vaddr = syms_name2val(buf2);
        if (undefined_symbol)
        {
          printf("undefined symbol\n");
          break;
        }
        s = 0;
        len = n + (~((vaddr&15)/4-1) & 3);
        for (i=-((vaddr&15)/4); i<len; i++)
        {
          if ((s&3) == 0)
            printf("0x%08lx:", vaddr+i*4);
          if ((i>=0) && (i<n))
          {
            word32 v1;
            if (read_child(vaddr+i*4, &v1, 4))
              break;
            printf(" 0x%08lx", v1);
          }
          else
            printf("           ");
          if ((s & 3) == 3)
          {
            int j, c;
            printf("  ");
            for (j=0; j<16; j++)
              if ((j+i*4-12>=0) && (j+i*4-12 < n*4))
              {
                if (read_child(vaddr+j+i*4-12, &c, 1))
                  break;
                if (c<' ')
                  putchar('.');
                else
                  putchar(c);
              }
              else
                putchar(' ');
            printf("\n");
          }
          s++;
        }
        if (s & 3)
          printf("\n");
        vaddr += n*4;
        break;

      case PRINT:
        lasttoken[0] = 0;
        sscanf(buf, "%s", buf2);
        if (buf2[0])
          vaddr = syms_name2val(buf2);
        if (undefined_symbol)
          break;
        name = syms_val2name(vaddr, &delta);
        printf("0x%08lx %ld", vaddr, (long)vaddr);
        for (i=31; i>=0; i--)
        {
          if (char32spc[i] != 'x')
            putchar(char32spc[i]);
          printf("%d", (int)((vaddr>>i)&1));
        }
        printf("\n");
        break;

      case BREAK:
        vaddr = n = 0;
        buf2[0] = 0;
        sscanf(buf, "%d %s", &n, buf2);
        if (buf2[0])
          vaddr = syms_name2val(buf2);
        if (undefined_symbol)
          break;
        edi.dr[n] = vaddr;
        if (vaddr == 0)
          dr7 &= ~(2 << (n*2));
        else
          dr7 |= 2 << (n*2);

      case STATUS:
        s = 0;
        for (n=0; n<4; n++)
        {
          s = 1;
          name = syms_val2name(edi.dr[n], &delta);
          printf("  dr%d  %s", n, name);
          if (delta)
            printf("+%#lx", delta);
          if (name[0] != '0')
            printf(" (0x%lx)", edi.dr[n]);
          if (!(dr7 & (3 << (n*2))))
            printf(" (disabled)");
          putchar('\n');
        }
        if (s == 0)
          printf("  No breakpoints set\n");
        break;

      case REGS:
        tssprint(&a_tss);
        unassemble(a_tss.tss_eip, 0);
        break;

      case SET:
        cmd = Zero;
        lasttoken[0] = 0;
        buf2[0] = 0;
        len = sscanf(buf, "%s %s", buf2, buf);
        if (buf2[0] == 0)
        {
          break;
        }
        if (len > 1)
        {
          v = syms_name2val(buf);
          if (undefined_symbol)
            break;
        }
        found = 0;
        for (i=0; regs[i].name; i++)
          if (strcmp(regs[i].name, buf2) == 0)
          {
            TSS *tss_ptr = &a_tss;
            found = 1;
            if (len > 1)
            {
              switch (regs[i].size)
              {
                case 1:
                  *(word8 *)((word8 *)tss_ptr + regs[i].ofs) = v;
                  break;
                case 2:
                  *(word16 *)((word8 *)tss_ptr + regs[i].ofs) = v;
                  break;
                case 4:
                  *(word32 *)((word8 *)tss_ptr + regs[i].ofs) = v;
                  break;
              }
            }
            else
            {
              switch (regs[i].size)
              {
                case 1:
                  printf("%02x ", *(word8 *)((word8 *)tss_ptr + regs[i].ofs));
                  my_getline(buf, "");
                  if (buf[0])
                  {
                    v = syms_name2val(buf);
                    if (undefined_symbol)
                      break;
                    *(word8 *)((word8 *)tss_ptr + regs[i].ofs) = v;
                  }
                  break;
                case 2:
                  printf("%04x ", *(word16 *)((word16 *)tss_ptr + regs[i].ofs));
                  my_getline(buf, "");
                  if (buf[0])
                  {
                    v = syms_name2val(buf);
                    if (undefined_symbol)
                      break;
                    *(word16 *)((word16 *)tss_ptr + regs[i].ofs) = v;
                  }
                  break;
                case 4:
                  printf("%08lx ", *(word32 *)((word32 *)tss_ptr + regs[i].ofs));
                  my_getline(buf, "");
                  if (buf[0])
                  {
                    v = syms_name2val(buf);
                    if (undefined_symbol)
                      break;
                    *(word32 *)((word32 *)tss_ptr + regs[i].ofs) = v;
                  }
                  break;
              }
            }
            break;
          }
        if (found)
          break;
        vaddr = syms_name2val(buf2);
        if (undefined_symbol)
          break;
        if (len < 2)
        {
          v = syms_name2val(buf);
          if (undefined_symbol)
            break;
          write_child(vaddr, &v, 4);
        }
        while (1)
        {
          word32 vv;
          if (read_child(vaddr, &vv,4))
            break;
          printf("0x%08lx 0x%08lx", vaddr, vv);
          my_getline(buf, "");
          if (buf[0])
          {
            if (strcmp(buf, ".") == 0)
              break;
            vv = syms_name2val(buf);
            if (write_child(vaddr, &vv, 4))
              break;
          }
          vaddr += 4;
        }
        break;
      case XNPX:
        printf("Control: 0x%04lx  Status: 0x%04lx  Tag: 0x%04lx\n",
               npx.control & 0xffff, npx.status & 0xffff, npx.tag & 0xffff);
        for (i=0; i<8; i++)
        {
          double d;
          int tag;
          int tos = (npx.status >> 11) & 7;
          printf("st(%d)  ", i);
          if (npx.reg[i].sign)
            putchar('-');
          else
            putchar('+');
          printf(" %04x %04x %04x %04x e %04x    ",
                 npx.reg[i].sig3,
                 npx.reg[i].sig2,
                 npx.reg[i].sig1,
                 npx.reg[i].sig0,
                 npx.reg[i].exponent);
          tag = (npx.tag >> (((i+tos)%8)*2)) & 3;
          switch (tag)
          {
            case 0:
              printf("Valid");
              if (((int)npx.reg[i].exponent-16382 < 1000) &&
                  ((int)npx.reg[i].exponent-16382 > -1000))
              {
                d = npx.reg[i].sig3/65536.0 + npx.reg[i].sig2/65536.0/65536.0
                  + npx.reg[i].sig1/65536.0/65536.0/65536.0;
                d = ldexp(d,(int)npx.reg[i].exponent-16382);
                if (npx.reg[i].sign)
                  d = -d;
                printf("  %.16g", d);
              }
              else
                printf("  (too big to display)");
              putchar('\n');
              break;
            case 1:
              printf("Zero\n");
              break;
            case 2:
              printf("Special\n");
              break;
            case 3:
              printf("Empty\n");
              break;
          }
        }
        break;

      case QUIT:
        return;

      case Zero:
        break;

      case CLS:
	asm volatile("pusha; movb $15,%ah; int $0x10; movb $0,%ah; int $0x10; popa");
	break;

      default:
        printf("Unknown command\n");
        lasttoken[0] = 0;
        cmd = Zero;
        break;
    }
    if (undefined_symbol)
    {
      lasttoken[0] = 0;
      cmd = Zero;
      undefined_symbol = 0;
    }
    rem_cmd = cmd;
  }
}
