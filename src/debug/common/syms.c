/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
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

/* Modified by Charles Sandmann 1995 for DJGPP V2 (bug fixes) 
   incorporate changes by Morten Welinder, terra@diku.dk */

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <io.h>
#include <libc/unconst.h>

#include <debug/tss.h>
#include <coff.h>
#include <debug/syms.h>
#include <debug/stab.h>
#include <debug/wild.h>

#include <assert.h>

void *xmalloc (size_t);

int undefined_symbol=0;
int syms_printwhy=0;

#define Ofs(n) ((int)&(((TSS *)0)->n))

struct {
  const char *name;
  int size;
  int ofs;
  } regs[] = {
  {"eip", 4, Ofs(tss_eip)},
  {"eflags", 4, Ofs(tss_eflags)},
  {"eax", 4, Ofs(tss_eax)},
  {"ebx", 4, Ofs(tss_ebx)},
  {"ecx", 4, Ofs(tss_ecx)},
  {"edx", 4, Ofs(tss_edx)},
  {"esp", 4, Ofs(tss_esp)},
  {"ebp", 4, Ofs(tss_ebp)},
  {"esi", 4, Ofs(tss_esi)},
  {"edi", 4, Ofs(tss_edi)},
  {"ax", 2, Ofs(tss_eax)},
  {"bx", 2, Ofs(tss_ebx)},
  {"cx", 2, Ofs(tss_ecx)},
  {"dx", 2, Ofs(tss_edx)},
  {"ah", 1, Ofs(tss_eax)+1},
  {"bh", 1, Ofs(tss_ebx)+1},
  {"ch", 1, Ofs(tss_ecx)+1},
  {"dh", 1, Ofs(tss_edx)+1},
  {"al", 1, Ofs(tss_eax)},
  {"bl", 1, Ofs(tss_ebx)},
  {"cl", 1, Ofs(tss_ecx)},
  {"dl", 1, Ofs(tss_edx)},
  {0, 0, 0}
};

/* From the file */

typedef struct SYM_ENTRY {
  unsigned long string_off;
  unsigned char type;
  unsigned char other;
  unsigned short desc;
  unsigned long val;
} SYM_ENTRY;

static FILHDR f_fh;
static AOUTHDR f_ah;
static SCNHDR *f_sh;
static SYMENT *f_symtab;
static SYM_ENTRY *f_aoutsyms;
static AUXENT *f_aux;
static LINENO **f_lnno;
static char *f_string_table;
static char *f_types;

/* built internally */

typedef struct {
  char *filename;
  unsigned long first_address;
  unsigned long last_address;
  LINENO *lines;
  int num_lines;
} FileNode;

static FileNode *files;
static int num_files;

typedef struct SymNode {
  char *name;
  unsigned long address;
  char type_c;
} SymNode;

static SymNode *syms;
static SymNode *syms_byname;
static int num_syms;

static int syms_sort_bn(const void *a, const void *b)
{
  const SymNode *sa = (const SymNode *)a;
  const SymNode *sb = (const SymNode *)b;
  return strcmp(sa->name, sb->name);
}

static int syms_sort_bv(const void *a, const void *b)
{
  const SymNode *sa = (const SymNode *)a;
  const SymNode *sb = (const SymNode *)b;
  return sa->address - sb->address;
}

static char *symndup(char *s, int len)
{
  char *rv;
  rv = xmalloc(len+1);
  memcpy(rv,s,len);
  rv[len] = 0;
  return rv;
}

static int valid_symbol(int i)
{
  char *sn;
  if (f_symtab[i].e.e.e_zeroes)
    sn = f_symtab[i].e.e_name; 
  else
    sn = f_string_table + f_symtab[i].e.e.e_offset;
  if (sn[0] != '_')
    return 0;
  if (strncmp(sn, "___gnu_compiled", 15) == 0)
    return 0;
  if (strcmp(sn, "__DYNAMIC") == 0)
    return 0;
  return 1;
}

/* bail(): Print MSG to stderr and quit */
static void __attribute__((noreturn)) bail(const char *msg)
{
  assert(msg);

  if (errno)
     fprintf(stderr, "Symify error: %s: %s\n", msg, strerror(errno));
  else
     fprintf(stderr, "Symify error: %s\n", msg);
  exit(EXIT_FAILURE);
}

/* xfseek(): Move the file pointer for FILE according to MODE,
 * print an error message and bailout if the file operation fails.
 * Return: 0 (success) always.
 */
static int xfseek(FILE *stream, long offset, int mode)
{
  assert(stream);
  assert((mode==SEEK_SET) || (mode==SEEK_CUR) || (mode==SEEK_END));

  /* Note that fseek's past the end of file will not fail in dos. */

  if (fseek(stream, offset, mode))
    bail("fseek failed");

  return 0;
}

/* xfread(): Read NUMBER of objects, each of SIZE bytes, from FILE to BUFFER.
 * print an error message and bailout if the file operation fails, or if less
 * than NUMBER*SIZE bytes were read.
 * Return: NUMBER always.
 */
static size_t xfread(void *buffer, size_t size, size_t number, FILE *stream)
{
  assert(buffer);
  assert(stream);

  if (fread(buffer, size, number, stream) == number)
    return number;
  else if (feof(stream))
    bail("unexpected end of file");
  else if (ferror(stream))
    bail("error reading from file");
  else
    bail("fread failed");

  return 0; /* dummy return */
}

/* xmalloc_fread(): Allocate a memory buffer to store NUMBER of objects,
 * each of SIZE bytes, then fill the buffer by reading from FILE.
 * print an error message and bailout if not enough memory available to
 * allocate the buffer, or if a file error occurs, or if not enough bytes
 * are read to fill the buffer.
 * Return: a pointer to the newly allocated buffer.
 */
static void *xmalloc_fread(size_t size, size_t number, FILE *stream)
{
  void *buffer;

  assert(stream);

  buffer = xmalloc(size * number);

  xfread(buffer, size, number, stream);

  return buffer;
}

static void process_coff(FILE *fd, long ofs)
{
  unsigned int i;
  int f, s, f_pending;
  LINENO *l = NULL;		/* CWS note: uninitialized? */
  int l_pending;
  unsigned long strsize;
  char *name;
  int i2_max;

  xfseek(fd, ofs, 0);
  xfread(&f_fh, FILHSZ, 1, fd);
  xfread(&f_ah, AOUTSZ, 1, fd);
  f_sh = xmalloc_fread(SCNHSZ, f_fh.f_nscns, fd);
  f_types = (char *)xmalloc(f_fh.f_nscns);
  f_lnno = (LINENO **)xmalloc(f_fh.f_nscns * sizeof(LINENO *));

  for (i=0; i<f_fh.f_nscns; i++)
  {
    if (f_sh[i].s_flags & STYP_TEXT)
      f_types[i] = 'T';
    if (f_sh[i].s_flags & STYP_DATA)
      f_types[i] = 'D';
    if (f_sh[i].s_flags & STYP_BSS)
      f_types[i] = 'B';
    if (f_sh[i].s_nlnno)
    {
      xfseek(fd, ofs + f_sh[i].s_lnnoptr, 0L);
      f_lnno[i] = xmalloc_fread(LINESZ, f_sh[i].s_nlnno, fd);
    }
    else
      f_lnno[i] = 0;
  }

  xfseek(fd, ofs + f_fh.f_symptr + f_fh.f_nsyms * SYMESZ, 0);
  xfread(&strsize, 4, 1, fd);
  f_string_table = (char *)xmalloc(strsize);
  /* CWS note: must zero or pukes below.  Does not fill from file. */
  memset(f_string_table,0,strsize);
  /* Note: this is fread instead of xfread, because this fread fails
     on a stripped program.  If we use xfread here, SYMIFY will bail out
     with an error message, and worse, Edebug and FSDB will be unable to
     debug stripped programs.  We can afford using fread here because we
     have already zeroed out the entire string table, see above.  So
     reading a partial table will never do any real harm.  */
  fread(f_string_table+4, 1, strsize-4, fd);
  f_string_table[0] = 0;

  xfseek(fd, ofs+f_fh.f_symptr, 0);
  f_symtab = xmalloc_fread(SYMESZ, f_fh.f_nsyms, fd);
  f_aux = (AUXENT *)f_symtab;

  num_syms = num_files = 0;
  for (i=0; i<f_fh.f_nsyms; i++)
  {
    switch (f_symtab[i].e_sclass)
    {
      case C_FILE:
        num_files++;
        break;
      case C_EXT:
      case C_STAT:
        if (!valid_symbol(i))
          break;
        num_syms++;
        break;
    }
    i += f_symtab[i].e_numaux;
  }

  files = (FileNode *)xmalloc(num_files * sizeof(FileNode));

  syms = (SymNode *)xmalloc(num_syms * sizeof(SymNode));

  f = s = f_pending = l_pending = i2_max = 0;
  for (i=0; i<f_fh.f_nsyms; i++)
  {
    switch (f_symtab[i].e_sclass)
    {
      case C_FILE:
        if (f_aux[i+1].x_file.x_n.x_zeroes)
          files[f].filename = symndup(f_aux[i+1].x_file.x_fname, 16);
        else
          files[f].filename = f_string_table + f_aux[i+1].x_file.x_n.x_offset;
        files[f].lines = 0;
        f_pending = 1;
	f++;
        break;
      case C_EXT:
      case C_STAT:

        if (f_symtab[i].e.e.e_zeroes)
          name = f_symtab[i].e.e_name;
        else
          name = f_string_table + f_symtab[i].e.e.e_offset;

        if (f_pending && strcmp(name, ".text") == 0)
        {
          files[f-1].first_address = f_symtab[i].e_value;
          files[f-1].last_address = f_symtab[i].e_value + f_aux[i+1].x_scn.x_scnlen - 1;
          files[f-1].num_lines = f_aux[i+1].x_scn.x_nlinno;
          f_pending = 0;
        }

        if (ISFCN(f_symtab[i].e_type))
        {
          int scn = f_symtab[i].e_scnum - 1;

          /* For some weird reason, sometimes x_lnnoptr is less than
             s_lnnoptr.  We just punt for such cases, rather than
             crash.  */
          if (f_aux[i+1].x_sym.x_fcnary.x_fcn.x_lnnoptr >= f_sh[scn].s_lnnoptr)
          {
            size_t l_idx = (f_aux[i+1].x_sym.x_fcnary.x_fcn.x_lnnoptr
			    - f_sh[scn].s_lnnoptr) / LINESZ;

            /* No line number info can be at offset larger than 0xffff
               from f_lnno[scn], because COFF is limited to 64K
               line-number entries.  If they have more line entries
               than that, they had line number overflow at link
               time. */
            if (l_idx < 0xffffU)
              {
                l = f_lnno[scn] + l_idx;
                l_pending = 1;
                i2_max = f_sh[scn].s_nlnno - l_idx;
                l->l_addr.l_paddr = f_symtab[i].e_value;
              }
          }
        }

        if (!valid_symbol(i))
          break;

        syms[s].address = f_symtab[i].e_value;
        if (f_symtab[i].e.e.e_zeroes)
          syms[s].name = symndup(f_symtab[i].e.e_name, 8);
        else
          syms[s].name = f_string_table + f_symtab[i].e.e.e_offset;

        switch (f_symtab[i].e_scnum)
        {
          case 1 ... 10:
            syms[s].type_c = f_types[f_symtab[i].e_scnum-1];
            break;
          case N_UNDEF:
            syms[s].type_c = 'U';
            break;
          case N_ABS:
            syms[s].type_c = 'A';
            break;
          case N_DEBUG:
            syms[s].type_c = 'D';
            break;
        }
        if (f_symtab[i].e_sclass == C_STAT)
          syms[s].type_c += 'a' - 'A';

        s++;
        break;
      case C_FCN:
        if (f_pending && files[f-1].lines == 0)
        {
          files[f-1].lines = l;
        }
        if (l_pending && f_aux[i+1].x_sym.x_misc.x_lnsz.x_lnno)
        {
          int lbase = f_aux[i+1].x_sym.x_misc.x_lnsz.x_lnno - 1;
          int i2;
          l->l_lnno = lbase;
          l++;
          for (i2 = 0; i2 < i2_max && l[i2].l_lnno; i2++)
            l[i2].l_lnno += lbase;
          l_pending = 0;
        }
        break;
    }
    i += f_symtab[i].e_numaux;
  }
}

static void process_aout(FILE *fd, long ofs)
{
  GNU_AOUT header;
  unsigned long string_table_length;
  int nsyms, i, f, s, l;

  xfseek(fd, ofs, 0);
  xfread(&header, sizeof(header), 1, fd);

  xfseek(fd, ofs + sizeof(header) + header.tsize + header.dsize + header.txrel + header.dtrel, 0);
  nsyms = header.symsize / sizeof(SYM_ENTRY);
  f_aoutsyms = xmalloc_fread(header.symsize, 1, fd);

  xfread(&string_table_length, 4, 1, fd);
  f_string_table = (char *)xmalloc(string_table_length);
  xfread(f_string_table+4, 1, string_table_length-4, fd);
  f_string_table[0] = 0;

  num_files = num_syms = 0;
  for (i=0; i<nsyms; i++)
  {
    char *symn = f_string_table + f_aoutsyms[i].string_off;
    char *cp;
    switch (f_aoutsyms[i].type & ~N_EXT)
    {
      case N_SO:
        if (symn[strlen(symn)-1] == '/')
          break;
        num_files++;
        break;
      case N_TEXT:
        cp = symn + strlen(symn) - 2;
	if (strncmp(symn, "___gnu", 6) == 0 ||
	    strcmp(cp, "d.") == 0 /* as in gcc_compiled. */ ||
	    strcmp(cp, ".o") == 0)
	  break;
      case N_DATA:
      case N_ABS:
      case N_BSS:
      case N_FN:
      case N_SETV:
      case N_SETA:
      case N_SETT:
      case N_SETD:
      case N_SETB:
      case N_INDR:
        num_syms ++;
        break;
    }
  }
  
  syms = (SymNode *)xmalloc(num_syms * sizeof(SymNode));
  memset(syms, 0, num_syms * sizeof(SymNode));
  files = (FileNode *)xmalloc(num_files * sizeof(FileNode));
  memset(files, 0, num_files * sizeof(FileNode));

  f = s = 0;
  for (i=0; i<nsyms; i++)
  {
    char c, *cp;
    char *symn = f_string_table + f_aoutsyms[i].string_off;
    switch (f_aoutsyms[i].type & ~N_EXT)
    {
      case N_SO:
        if (symn[strlen(symn)-1] == '/')
          break;
        if (f && files[f-1].last_address == 0)
          files[f-1].last_address = f_aoutsyms[i].val - 1;
        files[f].filename = symn;
        files[f].first_address = f_aoutsyms[i].val;
        f ++;
        break;
      case N_SLINE:
        files[f-1].num_lines++;
        break;
      case N_TEXT:
        cp = symn + strlen(symn) - 2;
	if (strncmp(symn, "___gnu", 6) == 0 ||
	    strcmp(cp, "d.") == 0 /* as in gcc_compiled. */ ||
	    strcmp(cp, ".o") == 0)
        {
          if (f && files[f-1].last_address == 0)
            files[f-1].last_address = f_aoutsyms[i].val - 1;
          break;
        }
	c = 't';
	goto sym_c;
      case N_DATA:
	c = 'd';
	goto sym_c;
      case N_ABS:
	c = 'a';
	goto sym_c;
      case N_BSS:
	c = 'b';
	goto sym_c;
      case N_FN:
	c = 'f';
	goto sym_c;
      case N_SETV:
	c = 'v';
	goto sym_c;
      case N_SETA:
	c = 'v';
	goto sym_c;
      case N_SETT:
	c = 'v';
	goto sym_c;
      case N_SETD:
	c = 'v';
	goto sym_c;
      case N_SETB:
	c = 'v';
	goto sym_c;
      case N_INDR:
	c = 'i';
	sym_c:
        syms[s].name = symn;
        syms[s].address = f_aoutsyms[i].val;
        syms[s].type_c = (f_aoutsyms[i].type & N_EXT) ? (c+'A'-'a') : c;
        s ++;
        break;
    }
  }
  
  l = f = 0;
  for (i=0; i<nsyms; i++)
  {
    char *symn = f_string_table + f_aoutsyms[i].string_off;
    switch (f_aoutsyms[i].type & ~N_EXT)
    {
      case N_SO:
        if (symn[strlen(symn)-1] == '/')
          break;
        files[f].lines = (LINENO *)xmalloc(files[f].num_lines * sizeof(LINENO));
        f++;
        l = 0;
        break;
      case N_SLINE:
        files[f-1].lines[l].l_addr.l_paddr = f_aoutsyms[i].val;
        files[f-1].lines[l].l_lnno = f_aoutsyms[i].desc;
        l ++;
        break;
    }
  }
  
}

static void process_file(FILE *fd, long ofs)
{
  short s, exe[2];
  xfseek(fd, ofs, 0);
  xfread(&s, 2, 1, fd);
  switch (s)
  {
    case 0x5a4d:       /* .exe */
      xfread(exe, 2, 2, fd);
      ofs += (long)exe[1] * 512L;
      if (exe[0])
        ofs += (long)exe[0] - 512L;
      process_file(fd, ofs);
      break;
    case 0x014c:	/* .coff */
      process_coff(fd, ofs);
      break;
    case 0x010b:	/* a.out ZMAGIC */
    case 0x0107:	/* a.out object */
      process_aout(fd, ofs);
      break;
  }
}

void syms_init(char *fname)
{
  FILE *fd = fopen(fname, "rb");
  if (fd == 0)
  {
    bail(fname);
  }
  else
  {
    process_file(fd, 0);

    syms_byname = (SymNode *)xmalloc(num_syms * sizeof(SymNode));
    memcpy(syms_byname, syms, num_syms * sizeof(SymNode));
    qsort(syms_byname, num_syms, sizeof(SymNode), syms_sort_bn);
    qsort(syms, num_syms, sizeof(SymNode), syms_sort_bv);

    fclose(fd);
  }
}

static int lookup_sym_byname(const char *name, int idx, int ofs)
{
  int below, above;
  
  below = -1;
  above = num_syms;
  while (above - below > 1)
  {
    int mid = (above + below) / 2;
    int c = 0;
    if (ofs)
      c = '_' - syms_byname[mid].name[0];
    if (c == 0)
      c = strncmp(name, syms_byname[mid].name+ofs, idx);
    if (c == 0)
    {
      while (mid && strncmp(name, syms_byname[mid-1].name+ofs, idx) == 0)
	mid--;
      return mid;
    }
    if (c < 0)
      above = mid;
    else
      below = mid;
  }
  return -1;
}

unsigned long syms_name2val(const char *name)
{
  int idx, sign=1, i;
  unsigned long v;
  char *cp;

  undefined_symbol = 0;

  idx = 0;
  cp = unconst(name, char *);
  sscanf(cp, "%s", cp);

  if (name[0] == 0)
    return 0;

  if (name[0] == '-')
  {
    sign = -1;
    name++;
  }
  else if (name[0] == '+')
  {
    name++;
  }
  if (isdigit((unsigned char)name[0]))
  {
    if (sign == -1)
      return -strtoul(name, 0, 0);	/* MW change from strtol */
    return strtoul(name, 0, 0);
  }

  cp = strpbrk(name, "+-");
  if (cp)
    idx = cp-name;
  else
    idx = strlen(name);

  for (i=0; i<idx; i++)
    if (name[i] == '#')
    {
      int f;
      int lnum, l;
      sscanf(name+i+1, "%d", &lnum);
      for (f=0; f<num_files; f++)
      {
	if ((strncmp(name, files[f].filename, i) == 0) && (files[f].filename[i] == 0))
	{
	  for (l=0; l<files[f].num_lines; l++)
	  {
	    if (files[f].lines[l].l_lnno == lnum)
	      return files[f].lines[l].l_addr.l_paddr + syms_name2val(name+idx);
	  }
          if(syms_printwhy)
	    printf("undefined line number %.*s\n", idx, name);
	  undefined_symbol = 1;
	  return 0;
	}
      }
      if(syms_printwhy)
        printf("Undefined file name %.*s\n", i, name);
      undefined_symbol = 1;
      return 0;
    }

  i = lookup_sym_byname(name, idx, 0);
  if (i == -1)
    i = lookup_sym_byname(name, idx, 1);
  if (i != -1)
    return syms_byname[i].address * sign + syms_name2val(name+idx);

  {
    const char *p = name + (name[0] == '%');

    for (i = 0; regs[i].name; i++)
      if (strncmp (p, regs[i].name, idx) == 0)
      {
	switch (regs[i].size)
	{
	  case 1:
	    v = *(unsigned char *)((unsigned char *)(&a_tss) + regs[i].ofs);
	    break;
	  case 2:
	    v = *(unsigned short *)((unsigned char *)(&a_tss) + regs[i].ofs);
	    break;
	  case 4:
	    v = *(unsigned long *)((unsigned char *)(&a_tss) + regs[i].ofs);
	    break;
	  default:
	    v = 0;	/* Or complains about maybe uninit */
	}
	return v + syms_name2val(name+idx);
      }
  }

  if(syms_printwhy)
    printf("Undefined symbol %.*s\n", idx, name);
  undefined_symbol = 1;
  return 0;
}

static char noname_buf[11];

char *syms_val2name(unsigned long val, unsigned long *delta)
{
  int above, below, mid;

  if (delta)
    *delta = 0;

  if (num_syms <= 0)
    goto noname;  
  above = num_syms;
  below = -1;
  mid = 0;			/* Or complains about maybe uninit */
  while (above-below > 1)
  {
    mid = (above+below)/2;
    if (syms[mid].address == val)
      break;
    if (syms[mid].address > val)
      above = mid;
    else
      below = mid;
  }
  if (syms[mid].address > val)
  {
    if (mid == 0)
      goto noname;
    mid--; /* the last below was it */
  }
  if (mid < 0)
    goto noname;
  if (strcmp(syms[mid].name, "_end") == 0)
    goto noname;
  if (strcmp(syms[mid].name, "__end") == 0)
    goto noname;
  if (strcmp(syms[mid].name, "_etext") == 0)
    goto noname;
  if (delta)
    *delta = val - syms[mid].address;
  return syms[mid].name;
noname:
  sprintf(noname_buf, "%#lx", val);
  return noname_buf;
}

char *syms_val2line(unsigned long val, int *lineret, int exact)
{
  int f, l;
  for (f=0; f<num_files; f++)
  {
    if (val >= files[f].first_address && val <= files[f].last_address && files[f].lines)
    {
      for (l=files[f].num_lines-1; l >= 0 && files[f].lines[l].l_addr.l_paddr > val; l--);
      if ((files[f].lines[l].l_addr.l_paddr != val) && exact)
        return 0;
      *lineret = files[f].lines[l].l_lnno;
      return files[f].filename;
    }
  }
  return 0;
}

void syms_listwild(char *pattern,
  void (*handler)(unsigned long addr, char type_c, char *name, char *name2, int lnum))
{
  int lnum;
  char *name;
  int i;

  for (i=0; i<num_syms; i++)
    if (wild(pattern, syms_byname[i].name))
    {
      name = syms_val2line(syms_byname[i].address, &lnum, 0);
      handler (syms_byname[i].address,
	       syms_byname[i].type_c,
	       syms_byname[i].name,
	       name, lnum);
    }
}

/* Code added by MW, changed by CWS */
char *syms_module(int no)
{
  if ((no < 0) || (no >= num_files))
    return 0;
  else
    return files[no].filename;
}

unsigned long syms_line2val(char *filename, int lnum)
{
  int f,l;

  for (f = 0; f < num_files; f++)
    if (strcmp (filename, files[f].filename) == 0)
    {
      for (l = 0; l < files[f].num_lines; l++)
	if (files[f].lines[l].l_lnno == lnum)
	  return files[f].lines[l].l_addr.l_paddr;
      return 0;
    }
  return 0;
}
