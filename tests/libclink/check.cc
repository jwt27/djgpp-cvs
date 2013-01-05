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

const char *predefs[] = { "main", "edata", "end", "etext", "_environ",
		    "__udivdi3", "__umoddi3", "__divdi3",
		    "__moddi3", "__cmpdi2", 0 };

/* Note: tzname is POSIX, but we list it here because it is a datum,
   not a function, and we can't stub it.  ctime() sets tzname, and
   ctime is ANSI and tzname is POSIX.  Sigh. */

const char *ansi_c89_fns[] = { "abort", "abs", "acos", "asctime", "asin",
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
"time", "tmpfile", "tmpnam", "tolower", "toupper", "tzname", "ungetc",
"vfprintf", "vprintf", "vsprintf", "wcstombs", "wctomb", 0
};

const char *ansi_c99_fns[] = { "_Exit", "acosf", "acosh", "acoshf",
"acoshl", "acosl", "asinf", "asinh", "asinhf", "asinhl", "asinl",
"atan2f", "atan2l", "atanf", "atanh", "atanhf", "atanhl", "atanl",
"atoll", "btowc", "cabs", "cabsf", "cabsl", "cacos", "cacosf",
"cacosh", "cacoshf", "cacoshl", "cacosl", "carg", "cargf", "cargl",
"casin", "casinf", "casinh", "casinhf", "casinhl", "casinl", "catan",
"catanf", "catanh", "catanhf", "catanhl", "catanl", "cbrt", "cbrtf",
"cbrtl", "ccos", "ccosf", "ccosh", "ccoshf", "ccoshl", "ccosl",
"ceilf", "ceill", "cexp", "cexpf", "cexpl", "cimag", "cimagf",
"cimagl", "clog", "clogf", "clogl", "conj", "conjf", "conjl",
"copysign", "copysignf", "copysignl", "cosf", "coshf", "coshl",
"cosl", "cpow", "cpowf", "cpowl", "cproj", "cprojf", "cprojl",
"creal", "crealf", "creall", "csin", "csinf", "csinh", "csinhf",
"csinhl", "csinl", "csqrt", "csqrtf", "csqrtl", "ctan", "ctanf",
"ctanh", "ctanhf", "ctanhl", "ctanl", "erf", "erff", "erfl", "erfc",
"erfcf", "erfcl", "exp2", "exp2f", "exp2l", "expf", "expl", "expm1",
"expm1f", "expm1l", "fabsf", "fabsl", "fdim", "fdimf", "fdiml",
"feclearexcept", "fegetenv", "fegetexceptflag", "fegetround",
"feholdexcept", "feraiseexcept", "fesetenv", "fesetexceptflag",
"fesetround", "fetestexcept", "feupdateenv", "fgetwc", "fgetws",
"fwscanf", "floorf", "floorl", "fma", "fmaf", "fmal", "fmax", "fmaxf",
"fmaxl", "fmin", "fminf", "fminl", "fmodf", "fmodl", "fpclassify",
"fputwc", "fputws", "frexpf", "frexpl", "fwide", "fwprintf", "getwc",
"getwchar", "hypot", "hypotf", "hypotl", "ilogb", "ilogbf", "ilogbl",
"imaxabs", "imaxdiv", "isblank", "isfinite", "isgreater",
"isgreaterequal", "isinf", "isless", "islessequal", "islessgreater",
"isnan", "isnormal", "isunordered", "iswalnum", "iswalpha",
"iswblank", "iswcntrl", "iswctype", "iswdigit", "iswgraph",
"iswlower", "iswprint", "iswpunct", "iswspace", "iswupper",
"iswxdigit", "ldexpf", "ldexpl", "lgamma", "lgammaf", "lgammal",
"llabs", "lldiv", "llrint", "llrintf", "llrintl", "llround",
"llroundf", "llroundl", "log1p", "log2", "logb", "log10f", "log10l",
"log1pf", "log1pl", "log2f", "log2l", "logbf", "logbl", "logf",
"logl", "lrint", "lrintf", "lrintl", "lround", "lroundf", "lroundl",
"mbrlen", "mbrtowc", "mbsinit", "mbsrtowcs", "modff", "modfl",
"nan", "nanf", "nanl", "nearbyint", "nearbyintf", "nearbyintl",
"nextafter", "nextafterf", "nextafterl", "nexttoward", "nexttowardf",
"nexttowardl", "powf", "powl", "putwc", "putwchar", "remainder",
"remainderf", "remainderl", "remquo", "remquof", "remquol", "rint",
"rintf", "rintl", "round", "roundf", "roundl", "scalbln", "scalblnf",
"scalblnl", "scalbn", "scalbnf", "scalbnl", "signbit", "sinf",
"sinhf", "sinhl", "sinl", "snprintf", "strtoimax", "strtoumax",
"sqrtf", "sqrtl", "strtof", "strtold", "strtoll", "strtoull",
"swprinf", "swscanf", "tanf", "tanhf", "tanhl", "tanl", "tgamma",
"tgammaf", "tgammal", "towctrans", "towlower", "towupper", "trunc",
"truncf", "truncl", "ungetwc", "vfscanf", "vfwprintf", "vfwscanf",
"vscanf", "vsnprintf",  "vsscanf", "vswprintf", "vswscanf",
"vwprintf", "vwscanf", "wcrtomb", "wcscat", "wcschr", "wcscmp",
"wcscoll", "wcscpy", "wcscspn", "wcsftime", "wcslen", "wcsncat",
"wcsncmp", "wcsncpy", "wcspbrk", "wcsrchr", "wcsrtombs", "wcsspn",
"wcsstr", "wcstod", "wcstof", "wcstoimax", "wcstok", "wcstol",
"wcstold", "wcstoll", "wcstoul", "wcstoull", "wcstoumax", "wcsxfrm",
"wctob", "wctrans", "wctype", "wmemchr", "wmemcmp", "wmemcpy",
"wmemmove", "wmemset", "wprintf", "wscanf", 0
};

const char *posix_fns[] = { "_exit", "_longjmp", "_setjmp",
"_tolower", "_toupper", "a64l", "accept", "access", "alarm",
"asctime_r", "basename", "bcmp", "bcopy", "bind", "bzero",
"catclose", "catgets", "catopen",
"cfgetispeed", "cfgetospeed", "cfsetispeed", "cfsetospeed",
"chdir", "chmod", "chown", "close", "closedir", "closelog",
"confstr", "connect", "creat", "ctermid", "ctime_r",
"dbm_clearerr", "dbm_close", "dbm_delete", "dbm_error", "dbm_fetch",
"dbm_firstkey", "dbm_nextkey", "dbm_open", "dbm_store", "dirname",
"dlclose", "dlerror", "dlopen", "dlsym", "drand48", "dup", "dup2",
"ecvt", "endgrent", "endhostent", "endnetent", "endprotoent", "endpwent",
"endservent", "endutxent", "erand48",
"execl", "execle", "execlp", "execv", "execve", "execvp",
"fchdir", "fchmod", "fchown", "fcntl", "fcvt", "fdopen", "ffs", "fileno",
"flockfile", "fmtmsg", "fnmatch", "fork", "fpathconf", "freeaddrinfo",
"fseeko", "fstat", "fstatvfs", "fsync", "ftello", "ftime", "ftok",
"ftruncate", "ftrylockfile", "ftw", "funlockfile", "gcvt", "getaddrinfo",
"getc_unlocked", "getchar_unlocked", "getcontext", "getcwd",
"getdate", "getegid", "geteuid", "getgid", "getgrent",
"getgrgid", "getgrgid_r", "getgrnam", "getgrnam_r", "getgroups",
"gethostbyaddr", "gethostbyname", "gethostent", "gethostid",
"gethostname", "getitimer", "getlogin", "getlogin_r", "getnameinfo",
"getnetbyaddr", "getnetbyname", "getnetent", "getopt", "getpeername",
"getpgid", "getpgrp", "getpid", "getppid", "getpriority",
"getprotobyname", "getprotobynumber", "getprotoent", "getpwent",
"getpwnam", "getpwnam_r", "getpwuid", "getpwuid_r", "getrlimit",
"getrusage", "getservbyname", "getservbyport", "getservent", "getsid",
"getsockname", "getsockopt", "getsubopt", "gettimeofday",
"getuid", "getutxent", "getutxid", "getutxline", "getwd",
"glob", "globfree", "gmtime_r", "grantpt",
"h_errno", "hcreate", "hdestroy", "hsearch",
"htonl", "htons", "iconv", "iconv_close", "iconv_open",
"if_freenameindex", "if_indextoname", "if_nameindex", "if_nametoindex",
"index", "inet_addr", "inet_ntoa", "inet_ntop", "inet_pton",
"initstate", "insque", "isascii", "isatty",
"j0", "j1", "jn", "jrand48", "kill", "killpg", "l64a", "lchown",
"lcong48", "lfind", "link", "listen", "localtime_r", "lockf",
"lrand48", "lsearch", "lseek", "lstat", "makecontext", "memccpy",
"mkdir", "mkfifo", "mknod", "mkstemp", "mktemp", "mmap", "mprotect",
"mrand48", "msgctl", "msgget", "msgrcv", "msgsnd", "msync",
"munmap", "nftw", "nice", "nl_langinfo", "nrand48", "ntohl", "ntohs",
"open", "opendir", "openlog", "optarg", "opterr", "optind", "optopt",
"pathconf", "pause", "pclose", "pipe", "poll", "popen", "pread",
"pselect", "putc_unlocked", "putchar_unlocked", "putenv",
"pututxline", "pwrite", "rand_r", "random", "read",
"readdir", "readdir_r", "readlink", "readv", "realpath",
"recv", "recvfrom", "recvmsg",
"regcomp", "regerror", "regexec", "regfree", "remque", "rewinddir",
"rindex", "rmdir", "scalb", "sched_yield", "seed48",
"seekdir", "select", "semctl", "semget", "semop",
"send", "sendmsg", "sendto", "setcontext", "setegid", "setenv", "seteuid",
"setgid", "setgrent", "sethostent", "setitimer", "setlogmask",
"setnetent", "setpgid", "setpgrp", "setpriority", "setprotoent",
"setpwent", "setregid", "setreuid", "setrlimit", "setservent", "setsid",
"setsockopt", "setstate", "setuid", "setutxent",
"shmat", "shmctl", "shmdt", "shmget", "shutdown",
"sigaction", "sigaddset", "sigaltstack", "sigdelset", "sigemptyset",
"sigfillset", "sighold", "sigignore", "siginterrupt", "sigismember",
"siglongjmp", "sigpause", "sigpending", "sigprocmask", "sigrelse",
"sigset", "sigsetjmp", "sigsuspend", "sigwait", "sleep",
"sockatmark", "socket", "socketpair", "srand48",
"srandom", "stat", "statvfs", "strcasecmp", "strdup", "strerror_r",
"strfmon", "strftime", "strncasecmp", "strptime", "strtok_r",
"swab", "swapcontext", "symlink", "sync", "sysconf", "syslog",
"tcdrain", "tcflow", "tcflush", "tcgetattr", "tcgetpgrp",
"tcgetsid", "tcsendbreak", "tcsetattr", "tcsetpgrp",
"tdelete", "telldir", "tempnam", "tfind", "times", "toascii",
"truncate", "tsearch", "ttyname", "ttyname_r",
"twalk", "tzset", "ualarm", "ulimit", "umask", "uname",
"unlink", "unlockpt", "unsetenv", "usleep", "utime", "utimes",
"vfork", "wait", "waitid", "waitpid",
"wcswcs", "wcswidth", "wcwidth",
"wordexp", "wordfree", "write", "writev",
"y0", "y1", "yn", 0
};

#define Tansi_c89	0x01
#define Tansi_c99	0x02
#define Tposix		0x04
#define Tallow		0x08
#define Tother		0x10
#define Tstub		0x20

int name2type(char *c)
{
  int i;
  if (c[0] == '_')
    return Tallow;
  for (i=0; ansi_c89_fns[i]; i++)
    if (strcmp(ansi_c89_fns[i], c) == 0)
      return Tansi_c89;
  for (i=0; ansi_c99_fns[i]; i++)
    if (strcmp(ansi_c99_fns[i], c) == 0)
      return Tansi_c99;
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

#if 0
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
#endif

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

  if ((obj->df & Tansi_c89) && (obj->rf & Tposix))
  {
    printf("%s: (C89) -> (P)\n", n);
    dtp |= Tansi_c89; rtp |= Tposix;
  }
  if ((obj->df & Tansi_c89) && !(obj->rf & Tansi_c89) && (obj->rf & Tansi_c99))
  {
    printf("%s: (C89) -> (C99)\n", n);
    dtp |= Tansi_c89; rtp |= Tposix;
  }
  else if ((obj->df & Tansi_c89) && (obj->rf & Tother))
  {
    printf("%s: (C89) -> (O)\n", n);
    dtp |= Tansi_c89; rtp |= Tother;
  }
  else if ((obj->df & Tansi_c99) && (obj->rf & Tposix))
  {
    printf("%s: (C99) -> (P)\n", n);
    dtp |= Tansi_c99; rtp |= Tposix;
  }
  else if ((obj->df & Tansi_c99) && (obj->rf & Tother))
  {
    printf("%s: (C99) -> (O)\n", n);
    dtp |= Tansi_c99; rtp |= Tother;
  }
  else if ((obj->df & Tposix) && (obj->rf & Tother))
  {
    printf("%s: (P) -> (O)\n", n);
    dtp |= Tposix; rtp |= Tother;
  }
  else if ((obj->df & Tposix) && (obj->df & Tother))
  {
    printf("%s: (P),(O)\n", n);
    dtp |= Tansi_c89|Tansi_c99|Tposix;
  }
  else if ((obj->df & Tansi_c89) && (obj->df & Tother))
  {
    printf("%s: (C89),(O)\n", n);
    dtp |= Tansi_c89|Tother;
  }
  else if ((obj->df & Tansi_c89) && (obj->df & Tposix))
  {
    printf("%s: (C89),(P)\n", n);
    dtp |= Tother|Tposix;
  }
  else if ((obj->df & Tansi_c99) && (obj->df & Tother))
  {
    printf("%s: (C99),(O)\n", n);
    dtp |= Tansi_c99|Tother;
  }
  else if ((obj->df & Tansi_c99) && (obj->df & Tposix))
  {
    printf("%s: (C99),(P)\n", n);
    dtp |= Tother|Tposix;
  }

  if ((obj->df & Tposix && obj->lf & (Tansi_c89 | Tansi_c99))
      || ((obj->df & Tother && obj->lf & (Tansi_c89 | Tansi_c99 | Tposix))))
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

void do_missing(const char *which, const char **fns, StringList &all_defs)
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

  do_missing("ANSI C89", ansi_c89_fns, all_defs);
  do_missing("ANSI C99", ansi_c99_fns, all_defs);
  do_missing("POSIX", posix_fns, all_defs);

  return 0;
}
