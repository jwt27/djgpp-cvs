/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*

   Redir 1.0 Copyright (C) 1995 DJ Delorie (dj@delorie.com)

   Redir is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Redir is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

*/
   

#include <stdio.h>
#include <process.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <crt0.h>

int _crt0_startup_flags = _CRT0_FLAG_DISALLOW_RESPONSE_FILES;

char **
__crt0_glob_function(char *a)
{
  return 0;
}

int time_it=0, display_exit_code=0;
time_t startt, endt;
int std_err_fid;
FILE *std_err;
int rv;

static void
usage(void)
{
  /*               ----+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8 */
  fprintf(stderr, "Redir 1.0 Copyright (C) 1995 DJ Delorie (dj@delorie.com) - distribute freely\n");
  fprintf(stderr, "NO WARRANTEE.  This program is protected by the GNU General Public License.\n");
  fprintf(stderr, "Usage: redir [-i file] [-o file] [-oa file] [-e file] [-ea file]\n");
  fprintf(stderr, "                [-eo] [-oe] [-x] [-t] command [args . . .]\n\n");
  fprintf(stderr, "  -i file   redirect stdandard input from file\n");
  fprintf(stderr, "  -o file   redirect standard output to file\n");
  fprintf(stderr, "  -oa file  append standard output to file\n");
  fprintf(stderr, "  -e file   redirect standard error to file\n");
  fprintf(stderr, "  -ea file  append standard error to file\n");
  fprintf(stderr, "  -eo       redirect standard error to standard output\n");
  fprintf(stderr, "  -oe       redirect standard output to standard error\n");
  fprintf(stderr, "  -x        print exit code\n");
  fprintf(stderr, "  -t        print elapsed time\n");
  fprintf(stderr, "  command   the program you want to run, with arguments\n\n");
  fprintf(stderr, "Options are processed in the order they are encountered.\n\n");
  exit(1);
}

static void
fatal(const char *msg, const char *fn)
{
  fprintf(std_err, msg, fn);
  fprintf(std_err, "The error was: %s\n", strerror(errno));
  exit(1);
}

int
main(int argc, char **argv)
{

  if (argc < 2)
    usage();

  std_err_fid = dup(1);
  std_err = fdopen(std_err_fid, "w");

  time(&startt);

  while (argc > 1 && argv[1][0] == '-')
  {
    if (strcmp(argv[1], "-i")==0 && argc > 2)
    {
      close(0);
      if (open(argv[2], O_RDONLY, 0666) != 0)
	fatal("redir: attempt to redirect stdin from %s failed", argv[2]);
      argc--;
      argv++;
    }
    else if (strcmp(argv[1], "-o")==0 && argc > 2)
    {
      close(1);
      if (open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0666) != 1)
	fatal("redir: attempt to redirect stdout to %s failed", argv[2]);
      argc--;
      argv++;
    }
    else if (strcmp(argv[1], "-oa")==0 && argc > 2)
    {
      close(1);
      if (open(argv[2], O_WRONLY|O_APPEND|O_CREAT, 0666) != 1)
	fatal("redir: attempt to append stdout to %s failed", argv[2]);
      argc--;
      argv++;
    }
    else if (strcmp(argv[1], "-e")==0 && argc > 2)
    {
      close(2);
      if (open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0666) != 2)
	fatal("redir: attempt to redirect stderr to %s failed", argv[2]);
      argc--;
      argv++;
    }
    else if (strcmp(argv[1], "-ea")==0 && argc > 2)
    {
      close(2);
      if (open(argv[2], O_WRONLY|O_APPEND|O_CREAT, 0666) != 2)
	fatal("redir: attempt to append stderr to %s failed", argv[2]);
      argc--;
      argv++;
    }
    else if (strcmp(argv[1], "-eo")==0)
    {
      close(2);
      if (dup2(1,2) == -1)
	fatal("redir: attempt to redirect stderr to stdout failed", 0);
    }
    else if (strcmp(argv[1], "-oe")==0)
    {
      close(1);
      if (dup2(2,1) == -1)
	fatal("redir: attempt to redirect stdout to stderr failed", 0);
    }
    else if (strcmp(argv[1], "-x")==0)
    {
      display_exit_code = 1;
    }
    else if (strcmp(argv[1], "-t")==0)
    {
      time_it = 1;
    }
    else
      usage();
    argc--;
    argv++;
  }

  rv = spawnvp(P_WAIT, argv[1], argv+1);
  if (rv < 0)
    fatal("Error attempting to run program %s\n", argv[1]);

  if (display_exit_code)
  {
    if ((rv & 255) == rv)
      fprintf(std_err, "Exit code: %d\n", rv & 255);
    else
      fprintf(std_err, "Exit code: %d (0x%04x)\n", rv & 255, rv);
  }

  time(&endt);
  if (time_it)
  {
    time_t min, sec, hour, elapsedt;
    elapsedt = endt - startt;

    sec = elapsedt % 60;
    min = (elapsedt / 60) % 60;
    hour = elapsedt / 3600;
    if (elapsedt > 59)
      fprintf(std_err, "Elapsed time: %d seconds (%d:%02d:%02d)\n", elapsedt, hour, min, sec);
    else
      fprintf(std_err, "Elapsed time: %d seconds\n", elapsedt);
  }

  return rv;
}
