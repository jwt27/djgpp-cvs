/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pc.h>
#include <debug/syms.h>
#include <debug/tss.h>

#define SC(r,c) (*(char *)(sc + (r)*ScreenCols() + (c)))
#define SW(r,c) (*(sc + (r)*ScreenCols() + (c)))

TSS a_tss;
int main(int argc, char **argv)
{
  int r, c;
  short *sc;
  char buf[90];
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
          fprintf(ofile, "line %d of %s", lineno, file);
        }
        fputc('\n', ofile);
      }
      else
        fputs(line, ofile);
    }
    return 0;
  }

  sc = (short *)malloc(ScreenRows() * ScreenCols() * 2);

  ScreenRetrieve(sc);

  for (r=0; r<ScreenRows(); r++)
  {
    if (SC(r,0) == ' ' && SC(r,1) == ' ' && SC(r,2) == '0' && SC(r,3) == 'x')
    {
      buf[8] = 0;
      for (i=0; i<8; i++)
        buf[i] = SC(r, i+4);
      sscanf(buf, "%x", &v);
      func = syms_val2name(v, &d);
      file = syms_val2line(v, &lineno, 0);
      buf[0] = 0;
      if (func)
      {
	strcpy(buf, func);
	if (d)
	  sprintf(buf+strlen(buf), "%+ld", d);
      }
      if (file)
      {
        if (buf[0])
          strcat(buf, ", ");
        sprintf(buf+strlen(buf), "line %d of %s", lineno, file);
      }
      if (buf[0])
        for (i=0; buf[i]; i++)
          SW(r, 15+i) = 0x0f00 + buf[i];
    }
  }

  if (ofile)
  {
    for (r=0; r<ScreenRows(); r++)
    {
      c = 0;
      for (i=0; i<ScreenCols(); i++)
        if (SC(r, i) != ' ')
          c = i;
      for (i=0; i<=c; i++)
        fputc(SC(r,i), ofile);
      fputc('\n', ofile);
    }
    fclose(ofile);
  }
  else
    ScreenUpdate(sc);
  return 0;
}
