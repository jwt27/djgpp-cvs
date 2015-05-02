/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <pc.h>
#include <debug/syms.h>
#include <debug/tss.h>

#define SC(r,c) (*(char *)(sc + (r)*ScreenCols() + (c)))
#define SW(r,c) (*(sc + (r)*ScreenCols() + (c)))

void *xmalloc (size_t);
void *xrealloc (void *, size_t);

TSS a_tss;
int main(int argc, char **argv)
{
  int r, c;
  short *sc;
  char *buf = NULL;
  char **sub_argv;
  size_t bufsize = 0;
  int i, lineno;
  unsigned v;
  unsigned long d;
  char *func, *file;
  FILE *ofile=0;
  FILE *ifile=0;

  if (argc < 2)
  {
    fprintf(stderr, "Usage: symify [-o <outfile>] [-i <corefile>] <program>\n");
    fprintf(stderr, "This program adds debug information to DJGPP program call frame tracebacks\n");
    return 1;
  }

  /* Try to run bfdsymify. */
  sub_argv = malloc(sizeof(char *)*(argc+1));
  if( sub_argv )
  {
    sub_argv[0] = "bfdsymify";
    for(i = 1; i < argc; i++)
    {
      sub_argv[i] = argv[i];
    }
    sub_argv[argc] = NULL;
    r = spawnvp(P_WAIT, "bfdsymify", sub_argv);
    if (! (r & 0xffffff00) )
    {
      free(sub_argv);
      return r;
    }
    free(sub_argv);
  }

  while (argv[1][0] == '-')
  {
    if ((strcmp(argv[1], "-o") == 0) && (argc > 3))
    {
      ofile = fopen(argv[2], "w");
      if (ofile == 0)
        fprintf(stderr, "Error: unable to open file %s\n", argv[2]);
      argc -= 2;
      argv += 2;
    }
    else if ((strcmp(argv[1], "-i") == 0) && (argc > 3))
    {
      ifile = fopen(argv[2], "r");
      if (ifile == 0)
        fprintf(stderr, "Error: unable to open file %s\n", argv[2]);
      argc -= 2;
      argv += 2;
    }
    else
    {
      fprintf(stderr, "Invalid option %s - type `symify' for help\n", argv[1]);
      exit(1);
    }
  }
  syms_init(argv[1]);

  if (ifile)
  {
    char line[1000];
    if (ofile == 0)
      ofile = stdout;
    while (fgets(line, 1000, ifile))
    {
      if (strncmp(line, "  0x", 4) == 0)
      {
        sscanf(line+4, "%x", &v);
        func = syms_val2name(v, &d);
        file = syms_val2line(v, &lineno, 0);
        fprintf(ofile, "  0x%08x", v);
        if (func)
        {
          fprintf(ofile, " %s", func);
          if (d)
            fprintf(ofile, "%+ld", d);
        }
        if (file)
        {
          if (func)
            fprintf(ofile, ", ");
          if (lineno)
            fprintf(ofile, "line %d of %s", lineno, file);
          else
            fprintf(ofile, "line ?? of %s", file);
        }
        fputc('\n', ofile);
      }
      else
        fputs(line, ofile);
    }
    return 0;
  }

  sc = (short *)xmalloc(ScreenRows() * ScreenCols() * 2);

  ScreenRetrieve(sc);

  bufsize = ScreenCols() + 10;
  buf = xmalloc (bufsize);

  for (r=0; r<ScreenRows(); r++)
  {
    if (SC(r,0) == ' ' && SC(r,1) == ' ' && SC(r,2) == '0' && SC(r,3) == 'x')
    {
      int l_left = bufsize - 1, l_func = 0, l_off = 0, l_file = 0;
      buf[8] = 0;
      for (i=0; i<8; i++)
        buf[i] = SC(r, i+4);
      sscanf(buf, "%x", &v);
      func = syms_val2name(v, &d);
      file = syms_val2line(v, &lineno, 0);
      if (ofile)
	fprintf (ofile, "  0x%08x", v);
      buf[0] = 0;
      if (func)
      {
	l_func = strlen (func);
	if (l_func > l_left - 10)
	{
	  bufsize += l_func + 10;
	  l_left += l_func + 10;
	  buf = xrealloc (buf, bufsize);
	}
	strcat(buf, func);
	l_left -= l_func;
	if (d)
	{
	  l_left -= sprintf(buf+l_func, "%+ld", d);
	  l_off = strlen(buf);
	}
      }
      if (file)
      {
	l_file = strlen(file);
	if (l_left < l_file + 25)
	{
	  bufsize += l_file + 25;
	  l_left += l_file + 25;
	  buf = xrealloc (buf, bufsize);
	}
        if (buf[0])
	{
          strcat(buf, ", ");
	  l_left -= 2;
	}
        if (lineno)
          sprintf(buf+strlen(buf), "line %d of %s", lineno, file);
        else
          sprintf(buf+strlen(buf), "line ?? of %s", file);
	l_file = strlen(buf);
      }
      if (buf[0])
      {
	int j;
	int max_func =
	  l_file > ScreenCols() - 13 && l_off > ScreenCols() / 2 - 6
	  ? ScreenCols() / 2 - 8 - (l_off - l_func)
	  : l_off;
	int in_func = 1, in_file = 0;

	/* 0xdeadbeef _FooBar_Func..+666, line 1234 of FooBarF..e.Ext  */
	/*            ^             ^   ^                            ^ */
	/*            0         l_func l_off                    l_file */
	for (i=0, j=0; buf[j]; i++, j++)
	{
	  if (i >= ScreenCols())
	    break;
	  else if (in_func && i >= max_func && j < l_off)
	  {
	    SW(r, 13+i) = 0x0700 + '.'; i++;
	    SW(r, 13+i) = 0x0700 + '.'; i++;
	    j = l_func;		/* truncate function name, jump to offset */
	    in_func = 0;
	    in_file = 1;
	  }
	  else if (in_file && 13+i >= ScreenCols() - 7 && j < l_file - 5)
	  {
	    SW(r, 13+i) = 0x0700 + '.'; i++;
	    SW(r, 13+i) = 0x0700 + '.'; i++;
	    j = l_file - 5;	/* jump to last char before extension */
	    in_file = 0;
	  }
	  SW(r, 13+i) = 0x0f00 + buf[j];
	}
	if (ofile)
	  fprintf (ofile, " %s\n", buf);
      }
    }
    else if (ofile)
    {
      c = 0;
      for (i=0; i<ScreenCols(); i++)
        if (SC(r, i) != ' ')
          c = i;
      for (i=0; i<=c; i++)
        fputc(SC(r,i), ofile);
      fputc('\n', ofile);
    }
  }

  if (ofile)
    fclose(ofile);
  else
    ScreenUpdate(sc);
  return 0;
}
