/*
**   Usage:   nm -g ../../lib/libc.a | check
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "slist.h"
#include "objs.h"

//-----------------------------------------------------------------------------

char *predefs[] = { "main", "edata", "end", "etext", "environ",
		    "__udivdi3", "__umoddi3", "__divdi3", "__moddi3",
		    0 };

char *ansi_fns[] = { "abort", "abs", "acos", "asctime", "asin",
"atan", "atan2", "atexit", "atof", "atoi", "atol", "bsearch",
"calloc", "ceil", "clearerr", "clock", "cos", "cosh", "ctime",
"difftime", "div", "errno", "exit", "exp", "fabs", "fclose", "feof",
"ferror", "fflush", "fgetc", "fgetpos", "fgets", "floor", "fmod",
"fopen", "fprintf", "fputc", "fputs", "fread", "free", "freopen",
"frexp", "fscanf", "fseek", "fsetpos", "ftell", "fwrite", "getc",
"getchar", "getenv", "gets", "gmtime", "isalnum", "isalpha",
"iscntrl", "isdigit", "isgraph", "islower", "isprint", "ispunct",
"isspace", "isupper", "isxdigit", "labs", "ldexp", "ldiv",
"localeconv", "localtime", "log", "log10", "longjmp", "main",
"malloc", "mblen", "mbstowcs", "mbtowc", "memchr", "memcmp", "memcpy",
"memmove", "memset", "mktime", "modf", "perror", "pow", "printf",
"putc", "putchar", "puts", "qsort", "raise", "rand", "realloc",
"remove", "rename", "rewind", "scanf", "setbuf", "setjmp",
"setlocale", "setvbuf", "signal", "sin", "sinh", "sprintf", "sqrt",
"srand", "sscanf", "strcat", "strchr", "strcmp", "strcoll", "strcpy",
"strcspn", "strerror", "strftime", "strlen", "strncat", "strncmp",
"strncpy", "strpbrk", "strrchr", "strspn", "strstr", "strtod",
"strtok", "strtol", "strtoul", "strxfrm", "system", "tan", "tanh",
"time", "tmpfile", "tmpnam", "tolower", "toupper", "ungetc",
"wcstombs", "vfprintf", "vprintf", "vsprintf", "wcstombs", "wctomb", 0
};

char *posix_fns[] = {
"_exit", "access", "alarm", "cfgetispeed", "cfgetospeed", "cfsetispeed",
"cfsetospeed", "chdir", "chmod", "chown", "close", "closedir", "confstr",
"creat", "ctermid", "dup", "dup2", "execl", "execle", "execlp", "execv",
"execve", "execvp", "fcntl", "fdopen", "fileno", "fnmatch", "fnmatch",
"fork", "fpathconf", "fpathconf", "fstat", "getcwd", "getegid", "geteuid",
"getgid", "getgrgid", "getgrnam", "getgroups", "getlogin", "getopt",
"getpgrp", "getpid", "getppid", "getpwnam", "getpwuid", "getuid", "glob",
"glob", "globfree", "globfree", "isatty", "kill", "link", "lseek", "mkdir",
"mkfifo", "open", "opendir", "optarg", "opterr", "optind", "optopt",
"pathconf", "pathconf", "pause", "pclose", "pipe", "popen", "read",
"readdir", "regcomp", "regerror", "regexec", "regfree", "rewinddir",
"rmdir", "setgid", "setpgid", "setsid", "setuid", "sigaction", "sigaddset",
"sigdelset", "sigemptyset", "sigfillset", "sigismember", "siglongjmp",
"sigpending", "sigprocmask", "sigsetjmp", "sigsuspend", "sleep", "stat",
"sysconf", "sysconf", "tcdrain", "tcflow", "tcflush", "tcgetattr",
"tcgetpgrp", "tcsendbreak", "tcsetattr", "tcsetpgrp", "times", "ttyname",
"tzname", "tzset", "umask", "uname", "unlink", "utime", "wait", "waitpid",
"wordexp", "wordfree", "write",
0 };

#define Tansi	0x01
#define Tposix	0x02
#define Tallow	0x04
#define Tother	0x08
#define Tstub	0x10

int name2type(char *c)
{
  int i;
  if (c[0] == '_')
    return Tallow;
  for (i=0; ansi_fns[i]; i++)
    if (strcmp(ansi_fns[i], c) == 0)
      return Tansi;
  for (i=0; posix_fns[i]; i++)
    if (strcmp(posix_fns[i], c) == 0)
      return Tposix;
  if (c[0] == '_' && c[1] == '_')
    for (i=0; posix_fns[i]; i++)
      if (strcmp(posix_fns[i], c+2) == 0)
	return Tposix;
  for (i=0; predefs[i]; i++)
    if (strcmp(predefs[i], c) == 0)
      return Tallow;
  return Tother;
}

char *type2name(int t)
{
  static char buf[10][10];
  static int bpi=0;
  bpi = (bpi+1)%10;
  char *bp = buf[bpi];
  if (t & Tansi) *bp++ = 'A';
  if (t & Tposix) *bp++ = 'P';
  if (t & Tallow) *bp++ = '-';
  if (t & Tother) *bp++ = '*';
  if (t == 0) *bp++ = '0';
  *bp = 0;
  return buf[bpi];
}

//-----------------------------------------------------------------------------

void figure_obj(Object *obj)
{
  int dt=0, rt=0;
  int i;

  for (i=0; i<obj->defs.count; i++)
    dt |= name2type(obj->defs[i]);
  for (i=0; i<obj->refs.count; i++)
    rt |= name2type(obj->refs[i]);
  obj->df = dt;
  obj->rf = rt;
}

void diagnose_obj(Object *obj)
{
  char *n = obj->name;
  int i, title=1;
  int dtp=0, rtp=0;

  if ((obj->df & Tansi) && (obj->rf & Tposix))
  {
    printf("%s: (A) -> (P)\n", n);
    dtp |= Tansi; rtp |= Tposix;
  }
  else if ((obj->df & Tansi) && (obj->rf & Tother))
  {
    printf("%s: (A) -> (O)\n", n);
    dtp |= Tansi; rtp |= Tother;
  }
  else if ((obj->df & Tposix) && (obj->rf & Tother))
  {
    printf("%s: (P) -> (O)\n", n);
    dtp |= Tposix; rtp |= Tother;
  }
  else if ((obj->df & Tposix) && (obj->df & Tother))
  {
    printf("%s: (P),(O)\n", n);
    dtp |= Tansi|Tposix;
  }
  else if ((obj->df & Tansi) && (obj->df & Tother))
  {
    printf("%s: (A),(O)\n", n);
    dtp |= Tansi|Tother;
  }
  else if ((obj->df & Tansi) && (obj->df & Tposix))
  {
    printf("%s: (A),(P)\n", n);
    dtp |= Tother|Tposix;
  }

  if ((obj->df & Tposix && obj->lf & Tansi)
      || ((obj->df & Tother && obj->lf & (Tansi | Tposix))))
  {
    dtp = -1;
    rtp = -1;
    printf("%s: (%c) != stub (%s)\n", n, obj->df & Tposix ? 'P' : 'O', obj->defs[0]);
  }

  for (i=0; i<obj->defs.count; i++)
  {
    int dt = name2type(obj->defs[i]);
    if (dt & dtp)
      printf("  T %s\n", obj->defs[i]);
  }
  for (i=0; i<obj->refs.count; i++)
  {
    int rt = name2type(obj->refs[i]);
    if (rt & rtp)
      printf("  U %s\n", obj->refs[i]);
  }
  if (dtp || rtp)
    printf("\n");

}

//-----------------------------------------------------------------------------

void link_obj(Object *o)
{
  Object *o2;
  int i, u, n=1;
  for (i=0; i<o->refs.count; i++)
  {
    u = 1;
    for (o2=Object::first; o2; o2=o2->next)
      if (o2->defs.has(o->refs[i]))
      {
	u = 0;
	o->deps.add(o2);
	if (o2->lf & Tstub)
	  printf("Impure stub %s(%s) -> %s\n", o->name, o->refs[i], o2->name);
	break;
      }
    if (u)
    {
      if (o->lf & Tstub)
	printf("Unsettled stub %s (%s)\n", o->name, o->refs[i]);
      else
      {
	if (n)
	{
	  printf("Undefined symbols from %s:\n", o->name);
	  n = 0;
	}
	printf("  U %s\n", o->refs[i]);
      }
    }
  }
  if (!n)
    printf("\n");
}

void propogate_obj(Object *o, int Plf=0)
{
  static int indent=1;
  int newlf;

  newlf = (o->lf | Plf | o->df) & ~Tstub;
  if (o->lf == newlf)
    return; /* got them all */
  indent++;
  o->lf = newlf;

  for (int i=0; i<o->deps.count; i++)
  {
    Object *o2 = o->deps.objs[i];
    newlf = (o->lf | o2->df | o2->lf) & ~Tstub;
    if (o2->lf != newlf)
    {
      propogate_obj(o2, newlf);
    }
  }
  indent--;
}

//-----------------------------------------------------------------------------

void do_missing(char *which, char **fns, StringList &all_defs)
{
  int n = 1;
  int w = 0;
  int i;

  for (i=0; fns[i]; i++)
  {
#if 0
    if (fns == posix_fns
	&& (strncmp(fns[i], "tc", 2)==0
	    || strncmp(fns[i], "cf", 2)==0
	    || strncmp(fns[i], "sig", 2)==0))
      continue;
#endif
    if (!all_defs.has(fns[i]))
    {
      if (n)
      {
	n = 0;
	printf("Missing %s functions:\n", which);
      }
      int s = strlen(fns[i]);
      if (w+s > 76)
      {
	putchar('\n');
	w = 0;
      }
      printf("%s ", fns[i]);
      w += s+1;
    }
  }
  if (!n)
    printf("\n\n");
}

//-----------------------------------------------------------------------------

main()
{
  char line[1000];
  char sym[1000];
  Object *obj = new Object("");
  int is_stub, i;

  StringList all_defs, all_refs, weak_defs;

  for (i=0; predefs[i]; i++)
  {
    obj->defs.add(predefs[i]);
    all_defs.add(predefs[i]);
  }
  obj = 0;

  while (fgets(line, 1000, stdin))
  {
    line[strlen(line)-1] = 0;
    if (line[0] == 0)
      continue;
    if (strcmp(line+strlen(line)-3, ".o:") == 0)
    {
      if (obj)
	figure_obj(obj);
      line[strlen(line)-1] = 0;
      obj = new Object(line);
      is_stub = (strncmp(line, "stub", 4) == 0 && isdigit(line[4]));
      if (is_stub)
	obj->lf |= Tstub;
    }
    else if (line[8] == ' ' && isupper(line[9]) && line[10] == ' ' && line[11] == '_')
    {
      if (line[9] == 'U')
      {
	all_refs.add(line+12);
	obj->refs.add(line+12);
      }
      else if (line[9] == 'C')
      {
	weak_defs.add(line+12);
	obj->defs.add(line+12);
      }
      else
      {
	if (all_defs.has(line+12))
	  printf("Multiply defined symbol: %s in %s\n", line+12, obj->name);
	all_defs.add(line+12);
	obj->defs.add(line+12);
      }
    }
  }
  if (obj)
    figure_obj(obj);

  for (i=0; i<weak_defs.count; i++)
    all_defs.add(weak_defs[i]);

  for (obj=Object::first; obj; obj=obj->next)
    link_obj(obj);
  for (obj=Object::first; obj; obj=obj->next)
    propogate_obj(obj);
  for (obj=Object::first; obj; obj=obj->next)
    diagnose_obj(obj);

  do_missing("ANSI", ansi_fns, all_defs);
  do_missing("POSIX", posix_fns, all_defs);

  return 0;
}
