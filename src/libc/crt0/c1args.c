/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <io.h>
#include <unistd.h>
#include <stdlib.h>
#include <crt0.h>
#include <go32.h>
#include <fcntl.h>
#include <libc/farptrgs.h>
#include <ctype.h>
#include <string.h>
#include <stubinfo.h>

#define ds _my_ds()

static void *
c1xmalloc(size_t s)
{
  void *q = malloc(s);
  if (q == 0)
  {
#define err(x) _write(STDERR_FILENO, x, sizeof(x)-1)
    err("No memory to gather arguments\r\n");
    _exit(1);
  }
  return q;
}

static int
far_strlen(int selector, int linear_addr)
{
  int save=linear_addr;
  _farsetsel(selector);
  while (_farnspeekb(linear_addr))
    linear_addr++;
  return linear_addr - save;
}

static int
atohex(char *s)
{
  int rv = 0;
  while (*s)
  {
    int v = *s - '0';
    if (v > 9)
      v -= 7;
    v &= 15; /* in case it's lower case */
    rv = rv*16 + v;
    s++;
  }
  return rv;
}

typedef struct Arg {
  char *arg;
  char **arg_globbed;
  struct ArgList *arg_file;
  struct Arg *next;
  int was_quoted;
} Arg;

typedef struct ArgList {
  int argc;
  Arg **argv;
} ArgList;

static Arg *new_arg(void)
{
  Arg *a = (Arg *)c1xmalloc(sizeof(Arg));
  memset(a, 0, sizeof(Arg));
  return a;
}

static void delete_arglist(ArgList *al);

static void
delete_arg(Arg *a)
{
  if (a->arg) free(a->arg);
  if (a->arg_globbed)
  {
    int i;
    for (i=0; a->arg_globbed[i]; i++)
      free(a->arg_globbed[i]);
    free(a->arg_globbed);
  }
  if (a->arg_file)
    delete_arglist(a->arg_file);
  free(a);
}

static ArgList *
new_arglist(int count)
{
  ArgList *al = (ArgList *)c1xmalloc(sizeof(ArgList));
  al->argc = count;
  al->argv = (Arg **)c1xmalloc((count+1)*sizeof(Arg *));
  memset(al->argv, 0, (count+1)*sizeof(Arg *));
  return al;
}

static void
delete_arglist(ArgList *al)
{
  int i;
  for (i=0; i<al->argc; i++)
    delete_arg(al->argv[i]);
  free(al->argv);
  free(al);
}

static ArgList *
parse_bytes(char *bytes, int length)
{
  int largc, quote=0, i;
  Arg *a, **anext, *afirst;
  ArgList *al;
  char *bp=bytes, *ep, *epp, *last=bytes+length;

  anext = &afirst;
  largc = 0;
  while (bp<last)
  {
    while (isspace(*bp) && bp < last)
      bp++;
    if (bp == last)
      break;
    ep = epp = bp;
    *anext = a = new_arg();
    while ((quote || !isspace(*ep)) && ep < last)
    {
      if (quote && *ep == quote)
      {
        quote = 0;
        ep++;
      }
      else if (!quote && (*ep == '\'' || *ep == '"'))
      {
        quote = *ep;
        ep++;
        a->was_quoted = 1;
      }
      else if (*ep == '\\' && strchr("'\"", ep[1]) && ep < last-1)
      {
        ep++;
        *epp++ = *ep++;
        a->was_quoted = 1;
      }
      else
      {
        *epp++ = *ep++;
      }
    }
    anext = &(a->next);
    largc++;
    a->arg = (char *)c1xmalloc(epp-bp+1);
    memcpy(a->arg, bp, epp-bp);
    a->arg[epp-bp] = 0;
    bp = ep+1;
  }
  al = new_arglist(largc);
  for (i=0, a=afirst; i<largc; i++, a=a->next)
    al->argv[i] = a;
  return al;
}

static int
count_args(ArgList *al)
{
  int i, r=0;
  for (i=0; i<al->argc; i++)
  {
    int j;
    if (al->argv[i]->arg_globbed)
    {
      for (j=0; al->argv[i]->arg_globbed[j]; j++);
      r += j;
    }
    else if (al->argv[i]->arg_file)
    {
      r += count_args(al->argv[i]->arg_file);
    }
    else
    {
      r++;
    }
  }
  return r;
}

static char **
fill_args(char **largv, ArgList *al)
{
  int i;
  for (i=0; i<al->argc; i++)
  {
    int j;
    if (al->argv[i]->arg_globbed)
    {
      for (j=0; al->argv[i]->arg_globbed[j]; j++)
      {
        *largv++ = al->argv[i]->arg_globbed[j];
        al->argv[i]->arg_globbed[j] = 0;
      }
    }
    else if (al->argv[i]->arg_file)
    {
      largv = fill_args(largv, al->argv[i]->arg_file);
    }
    else
    {
      *largv++ = al->argv[i]->arg;
      al->argv[i]->arg = 0;
    }
  }
  return largv;
}

static void
expand_response_files(ArgList *al)
{
  int i, f;
  for (i=0; i<al->argc; i++)
  {
    if (! al->argv[i]->was_quoted && al->argv[i]->arg[0] == '@')
      if ((f = _open(al->argv[i]->arg+1, O_RDONLY)) >= 0)
      {
	char *bytes;
	int len, st_size;
	st_size = lseek(f, 0L, SEEK_END);
	lseek(f, 0L, SEEK_SET);
        bytes = (char *)alloca(st_size);
        len = _read(f, bytes, st_size);
        _close(f);
        al->argv[i]->arg_file = parse_bytes(bytes, len);
        expand_response_files(al->argv[i]->arg_file);
      }
  }
}

static void
expand_wildcards(ArgList *al)
{
  int i;
  for (i=0; i<al->argc; i++)
  {
    if (al->argv[i]->arg_file)
      expand_wildcards(al->argv[i]->arg_file);
    else if (!(al->argv[i]->was_quoted))
    {
      al->argv[i]->arg_globbed = __crt0_glob_function(al->argv[i]->arg);
    }
  }
}

void
__crt0_setup_arguments(void)
{
  ArgList *arglist;
  char *argv0;
  int prepend_argv0 = 1;

  /*
  ** first, figure out what to pass for argv[0]
  */
  {
    int i;
    char *ap, *ls, *fc;
/*    char newbase[14]; */
    
    if (_crt0_startup_flags & _CRT0_FLAG_DROP_DRIVE_SPECIFIER)
      if (__dos_argv0[1] == ':')
        __dos_argv0 += 2;

    ls = __dos_argv0;
    for (ap=__dos_argv0; *ap; ap++)
      if (*ap == ':' || *ap == '\\' || *ap == '/')
        ls = ap + 1;
    fc = ls;
#if 0
    /* We never do this!  Only the stub uses this field */
    if (_stubinfo->basename[0])
    {
      for (i=0; i<8 && _stubinfo->basename[i]; i++)
	newbase[i] = _stubinfo->basename[i];
      newbase[i++] = '.';
      newbase[i++] = 'E';
      newbase[i++] = 'X';
      newbase[i++] = 'E';
      newbase[i++] = 0;
      fc = newbase;
    }
#endif
    if (_stubinfo->argv0[0])
    {
      fc = _stubinfo->argv0;
    }
    argv0 = (char *)calloc(1, ls-__dos_argv0+strlen(fc)+1);
    if (ls == __dos_argv0)
      strncpy(argv0, fc, 16);
    else
    {
      strncpy(argv0, __dos_argv0, ls-__dos_argv0);
      strncat(argv0, fc, 16);
    }
    for (i=0; (fc == _stubinfo->argv0)?(i<ls-__dos_argv0):(argv0[i]); i++)
    {
      if (!(_crt0_startup_flags & _CRT0_FLAG_USE_DOS_SLASHES))
        if (argv0[i] == '\\')
          argv0[i] = '/';
      if (!(_crt0_startup_flags & _CRT0_FLAG_PRESERVE_UPPER_CASE))
        if (isupper(argv0[i]))
          argv0[i] = tolower(argv0[i]);
    }
    if (_crt0_startup_flags & _CRT0_FLAG_DROP_EXE_SUFFIX)
    {
      char *sp = argv0 + strlen(argv0) - 4;
      if (sp[0] == '.'
	  && (sp[1] == 'e' || sp[1] == 'E')
	  && (sp[2] == 'x' || sp[2] == 'X')
	  && (sp[3] == 'e' || sp[3] == 'E'))
        *sp = 0;
    }
  }
  
  /*
  ** Next, scan dos's command line.
  */
  {
    char doscmd[128];
    movedata(_stubinfo->psp_selector, 128, ds, (int)doscmd, 128);
    arglist = parse_bytes(doscmd+1, doscmd[0] & 0x7f);
  }
  
  /*
  ** Check for !proxy
  */
  if (arglist->argc > 3 && strcmp(arglist->argv[0]->arg, "!proxy") == 0)
  {
    int argv_seg, argv_ofs, i;
    unsigned short *rm_argv;
    __crt0_argc = atohex(arglist->argv[1]->arg);
    argv_seg = atohex(arglist->argv[2]->arg);
    argv_ofs = atohex(arglist->argv[3]->arg);
    delete_arglist(arglist);

    rm_argv = (unsigned short *)alloca(__crt0_argc*sizeof(unsigned short));
    movedata(_dos_ds, argv_seg*16+argv_ofs, ds, (int)rm_argv, __crt0_argc*sizeof(unsigned short));

    arglist = new_arglist(__crt0_argc);
    
    for (i=0; i<__crt0_argc; i++)
    {
      int al = far_strlen(_dos_ds, argv_seg*16 + rm_argv[i]);
      arglist->argv[i] = new_arg();
      arglist->argv[i]->arg = (char *)c1xmalloc(al+1);
      movedata(_dos_ds, argv_seg*16 + rm_argv[i], ds, (int)(arglist->argv[i]->arg), al+1);
    }
    prepend_argv0 = 0;
  }
  else if (arglist->argc > 3 && strcmp(arglist->argv[0]->arg, "!proxy2") == 0)
  {
    int argv_sel, argv_ofs, i;
    unsigned long *pm_argv;
    __crt0_argc = atohex(arglist->argv[1]->arg);
    argv_sel = atohex(arglist->argv[2]->arg);
    argv_ofs = atohex(arglist->argv[3]->arg);
    delete_arglist(arglist);

    pm_argv = (unsigned long *)alloca(__crt0_argc*sizeof(unsigned long));
    movedata(argv_sel, argv_ofs, ds, (int)pm_argv, __crt0_argc*sizeof(unsigned long));

    arglist = new_arglist(__crt0_argc);
    
    for (i=0; i<__crt0_argc; i++)
    {
      int al = far_strlen(argv_sel, pm_argv[i]);
      arglist->argv[i] = new_arg();
      arglist->argv[i]->arg = (char *)c1xmalloc(al+1);
      movedata(argv_sel, pm_argv[i], ds, (int)(arglist->argv[i]->arg), al+1);
    }
    prepend_argv0 = 0;
  }

  /*
  **  Now, expand response files
  */
  if (!(_crt0_startup_flags & _CRT0_FLAG_DISALLOW_RESPONSE_FILES))
    expand_response_files(arglist);

  /*
  **  Now, expand wildcards
  */
  expand_wildcards(arglist);

  __crt0_argc = prepend_argv0 + count_args(arglist);
  __crt0_argv = (char **)c1xmalloc((__crt0_argc+1) * sizeof(char *));
  if (prepend_argv0)
    __crt0_argv[0] = argv0;
  *fill_args(__crt0_argv+prepend_argv0, arglist) = 0;
}
