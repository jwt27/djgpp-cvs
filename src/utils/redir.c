/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*

   Redir 2.1 Copyright (C) 1995-1999 DJ Delorie (dj@delorie.com)

   Redir is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Redir is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

*/
   
#include <ctype.h>
#include <float.h>
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
#include <stubinfo.h>
#include <sys/segments.h>
#include <sys/exceptn.h>

/* Here's the deal.  We need to pass the command-line arguments to the
   child program *exactly* as we got them.  This means we cannot allow
   any wildcard expansion, we need to retain any quote characters, and
   we need to disable response files processing.  That's why
   _crt0_startup_flags are defined as they are, below, and that's why
   we define an empty __crt0_glob_function.

   In addition, we need to invoke the child program in the same way we
   were invoked: if they called us via `system', that's how the child
   should be invoked, and if they used `spawnXX', so should we.  This is
   so the child will process quoted arguments and wildcards exactly as
   the caller wanted.  */

int _crt0_startup_flags =
  (_CRT0_FLAG_DISALLOW_RESPONSE_FILES | _CRT0_FLAG_KEEP_QUOTES);

char **
__crt0_glob_function(char *a)
{
  return 0;
}

extern void *xmalloc(size_t), *xrealloc(void *, size_t);

int time_it=0, display_exit_code=0;
struct timeval startt, endt;
int std_err_fid;
FILE *std_err;
int rv;

static void
usage(void)
{
  /*               ----+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8 */
  fprintf(stderr, "Redir 2.1 Copyright (C) 1995 - 1999 DJ Delorie (dj@delorie.com)\n");
  fprintf(stderr, "Distribute freely.  There is NO WARRANTY.\n");
  fprintf(stderr, "This program is protected by the GNU General Public License.\n\n");
  fprintf(stderr, "Usage: redir [-i file] [-o file] [-oa file] [-e file] [-ea file]\n");
  fprintf(stderr, "                [-eo] [-oe] [-x] [-t] command [args . . .]\n\n");
  fprintf(stderr, "  -i file   redirect standard input from file\n");
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
  fprintf(std_err, "\nThe error was: %s\n", strerror(errno));
  exit(1);
}

static void
unquote(const char *src, char *dst)
{
  int quote=0;

  while ((quote || !isspace((unsigned char)*src)) && *src)
  {
    if (quote && *src == quote)
    {
      quote = 0;
      src++;
    }
    else if (!quote && (*src == '\'' || *src == '"'))
    {
      quote = *src;
      src++;
    }
    else if (*src == '\\' && strchr("'\"", src[1]) && src[1])
    {
      src++;
      *dst++ = *src++;
    }
    else
    {
      *dst++ = *src++;
    }
  }
  *dst = '\0';
}

static char *
unquoted_argv(int argc, char *argv[], char *reuse)
{
  char *new_arg;

  if (reuse)
    new_arg = (char *)xrealloc(reuse, strlen(argv[argc]) + 1);
  else
    new_arg = (char *)xmalloc(strlen(argv[argc]) + 1);
  unquote(argv[argc], new_arg);
  return new_arg;
}

extern char __PROXY[];	/* defined on crt1.c */
extern size_t __PROXY_LEN;

static int
run_program(int argc, char *argv[], int skip)
{
  char doscmd[128];
  char *tail = doscmd + 1, *tp = tail;
  size_t tail_len;
  int i = 0;

  /* Decide whether to invoke them with `spawn' or `system'.  */
  if (!getenv(__PROXY))
  {
    movedata(_stubinfo->psp_selector, 128, _my_ds(), (int)doscmd, 128);
    tail_len = doscmd[0] & 0x7f;
    tail[tail_len] = '\0';

    /* If the DOS command tail is "!proxy XXXX YYYY", then
       invoke via `spawn'.  */
    if (strstr(tail, __PROXY+1))
    {
      while (isspace((unsigned char)*tp))
	tp++;
      if (strncmp(tp, __PROXY+1, __PROXY_LEN-1)==0)
      {
	char *endarg;

	/* Paranoia: do we have at least three hex numbers after !proxy?  */
	endarg = tp + __PROXY_LEN - 1;
	do {
	  tp = endarg;
	  errno = 0;
	  strtoul(tp, &endarg, 16);
	  if (errno || endarg == tp)
	    break;
	  if (++i == 3)
	  {
	    gettimeofday(&startt, NULL);
	    return spawnvp(P_WAIT, argv[skip], argv+skip);
	  }
	} while (*endarg);
      }
    }
    /* The DOS command tail is the actual command line.
       Get past our own options we've already parsed,
       and pass the rest to the child via `system'.
       SKIP says how many argv[] elements to skip.  */
    for (tp = tail; skip--; argv++)
    {
      tp = strstr(tp, argv[1]);
      /* If, at some point, we don't find the next argv[] element,
	 it's probably some disaster, because they all should be
	 there.  Instead of screaming bloody murder, we fall back
	 on using argv[] from our `main', as the last resort.  */
      if (!tp)
	break;
      if (skip == 0)
      {
	/* We've come all the way to the child command line, invoke it.  */
	gettimeofday(&startt, NULL);
	return system(tp);
      }
      tp += strlen(argv[1]);	/* get past this arg */
    }
  }
  /* We need to recreate the original command line as a single string,
     from its breakdown in argv[].  */
  for (tail_len = 0, i = skip; i < argc; i++)
    tail_len += strlen(argv[i]) + 1;	/* +1 for the blank between args */

  tp = tail = (char *)xmalloc(tail_len + 1);
  for (i = skip; i < argc; i++)
  {
    size_t len = strlen(argv[i]);
    memcpy(tp, argv[i], len);
    tp[len] = ' ';
    tp += len + 1;
  }
  tail[tail_len] = '\0';

  gettimeofday(&startt, NULL);
  return system(tail);
}

int
main(int argc, char **argv)
{
  char *arg1 = NULL, *arg2 = NULL;
  int ac = argc;
  char **av = argv;

  /* Don't let us crash because some naughty program left
     the FPU in shambles.  */
  _clear87();
  _fpreset();

  if (argc < 2)
    usage();

  std_err_fid = dup(1);
  std_err = fdopen(std_err_fid, "w");

  /* We requested that the startup code retains any quote characters
     in the argv[] elements.  So we need to unquote those that we
     process as we go.  */
  while (argc > 1 && (arg1 = unquoted_argv(1, argv, arg2))[0] == '-')
  {
    int temp;
    if (strcmp(arg1, "-i")==0 && argc > 2)
    {
      if ((temp = open(arg2 = unquoted_argv(2, argv, arg1),
		       O_RDONLY, 0666)) < 0
	  || dup2(temp, 0) == -1)
	fatal("redir: attempt to redirect stdin from %s failed", arg2);
      close(temp);
      argc--;
      argv++;
    }
    else if (strcmp(arg1, "-o")==0 && argc > 2)
    {
      if ((temp = open(arg2 = unquoted_argv(2, argv, arg1),
		       O_WRONLY|O_CREAT|O_TRUNC, 0666)) < 0
	  || dup2(temp, 1) == -1)
	fatal("redir: attempt to redirect stdout to %s failed", arg2);
      close(temp);
      argc--;
      argv++;
    }
    else if (strcmp(arg1, "-oa")==0 && argc > 2)
    {
      if ((temp = open(arg2 = unquoted_argv(2, argv, arg1),
		       O_WRONLY|O_APPEND|O_CREAT, 0666)) < 0
	  || dup2(temp, 1) == -1)
	fatal("redir: attempt to append stdout to %s failed", arg2);
      close(temp);
      argc--;
      argv++;
    }
    else if (strcmp(arg1, "-e")==0 && argc > 2)
    {
      if ((temp = open(arg2 = unquoted_argv(2, argv, arg1),
		       O_WRONLY|O_CREAT|O_TRUNC, 0666)) < 0
	  || dup2(temp, 2) == -1)
	fatal("redir: attempt to redirect stderr to %s failed", arg2);
      close(temp);
      argc--;
      argv++;
    }
    else if (strcmp(arg1, "-ea")==0 && argc > 2)
    {
      if ((temp = open(arg2 = unquoted_argv(2, argv, arg1),
		       O_WRONLY|O_APPEND|O_CREAT, 0666)) < 0
	  || dup2(temp, 2) == -1)
	fatal("redir: attempt to append stderr to %s failed", arg2);
      close(temp);
      argc--;
      argv++;
    }
    else if (strcmp(arg1, "-eo")==0)
    {
      if (dup2(1,2) == -1)
	fatal("redir: attempt to redirect stderr to stdout failed", 0);
    }
    else if (strcmp(arg1, "-oe")==0)
    {
      if (dup2(2,1) == -1)
	fatal("redir: attempt to redirect stdout to stderr failed", 0);
    }
    else if (strcmp(arg1, "-x")==0)
    {
      display_exit_code = 1;
    }
    else if (strcmp(arg1, "-t")==0)
    {
      time_it = 1;
    }
    else
      usage();
    argc--;
    argv++;
  }

  if (argc <= 1)
  {
    errno = EINVAL;
    fatal("Missing program name; aborting", "");
  }

  /* We do NOT want `redir' to abort if the child is interrupted
     or crashes for any reason.  */
  _control87(0x033f, 0xffff);	/* mask all numeric exceptions */
  __djgpp_exception_toggle();
  rv = run_program(ac, av, ac - argc + 1);
  __djgpp_exception_toggle();
  _control87(0x033f, 0xffff);	/* in case the child unmasked some */
  _clear87();			/* clean up after the child, just in case */
  _fpreset();
  gettimeofday(&endt, NULL);

  if (rv < 0)
    fatal("Error attempting to run program %s", argv[1]);

  if (display_exit_code)
  {
    if ((rv & 255) == rv)
      fprintf(std_err, "Exit code: %d\n", rv & 255);
    else
      fprintf(std_err, "Exit code: %d (0x%04x)\n", rv & 255, rv);
  }

  if (time_it)
  {
    long min, sec, hour, elapsed_sec, elapsed_usec, msec;
    elapsed_sec = endt.tv_sec - startt.tv_sec;
    elapsed_usec = endt.tv_usec - startt.tv_usec;
    if (elapsed_usec < 0)
    {
      elapsed_sec -= 1;
      elapsed_usec += 1000000L;
    }

    msec = (elapsed_usec + 500) / 1000;
    sec = elapsed_sec % 60;
    min = (elapsed_sec / 60) % 60;
    hour = elapsed_sec / 3600;
    if (elapsed_sec > 59)
      fprintf(std_err,
	      "Elapsed time: %ld.%03ld seconds (%ld:%02ld:%02ld.%03ld)\n",
	      elapsed_sec, msec, hour, min, sec, msec);
    else
      fprintf(std_err, "Elapsed time: %ld.%03ld seconds\n", elapsed_sec, msec);
  }

  return rv;
}
