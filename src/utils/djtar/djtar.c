/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <io.h>

#define NO_LFN(n) (!_use_lfn(n))

#include "oread.h"
#include "zread.h"

static void
Fatal(const char *msg)
{
  fprintf(stderr, "Fatal! %s!\n", msg);
  exit(1);
}

static char *
xstrdup(const char * source)
{
  char *ptr = strdup(source);
  if (!ptr)
    Fatal("Out of memory");
  return ptr;
}

/*------------------------------------------------------------------------*/

typedef struct CE {
  struct CE *next;
  char *from;
  char *to;
} CE;

/* Do not extract files and directories starting with prefixes in this list. */
/* It has precendence over only_dir below.  */
struct skip_dir_list
{
   char *skip_dir;
   struct skip_dir_list *next;
} *skip_dirs;

/* Extract only files and directories starting with prefixes in this list. */
struct only_dir_list
{
   char *only_dir;
   struct only_dir_list *next;
} *only_dirs;

#define HASHSIZE 2048
#define HASHMASK 2047
#define HASHBITS 11
CE *htab[HASHSIZE];

static unsigned long
hash(unsigned char *cp)
{
  unsigned long rv = 0;
  while (*cp)
    rv += *cp++;
  while (rv > HASHMASK)
    rv = (rv & HASHMASK) + (rv >> HASHBITS);
  return rv;
}

static void
store_entry(char *from, char *to)
{
  unsigned long h = hash((unsigned char *)from);
  CE *ce = (CE *)xmalloc(sizeof(CE));
  ce->from = xstrdup(from);
  ce->to = xstrdup(to);
  ce->next = htab[h];
  htab[h] = ce;
}

static char *
get_entry(char *from)
{
  CE *ce;
  for (ce = htab[hash((unsigned char *)from)]; ce; ce = ce->next)
  {
    if (strcmp(ce->from, from) == 0)
      return ce->to;
  }
  return from;
}

static void
do_name_changes(char *fname)
{
  struct skip_dir_list * new_entry; 
  FILE *f = fopen(fname, "r");
  char from[PATH_MAX], to[PATH_MAX];
  char line[PATH_MAX * 2 + 10];
  if (f == 0)
  {
    perror(fname);
    exit(1);
  }
  while (1)
  {
    fgets(line, sizeof(line), f);
    if (feof(f))
      break;
    from[0] = to[0] = 0;
    sscanf(line, "%s %s", from, to);
    if (from[0])
    {
      if (to[0])
        store_entry(from, to);
      else
      {
        new_entry = xmalloc(sizeof(struct skip_dir_list));
        new_entry->skip_dir = xstrdup(from);
        new_entry->next = skip_dirs;
        skip_dirs = new_entry;
      }
    }
  }
  fclose(f);
}

/*------------------------------------------------------------------------*/

FILE *change_file;

int dot_switch = 1;
int a_switch = 0;
int v_switch = 0;
int s_switch = 1;
int z_switch = 0;
int text_unix = 0;
int text_dos = 0;
int to_stdout = 0;
int to_tty = 0;
int ignore_csum = 0;
int list_only = 1;
char skipped_str[] = "[skipped]";

/*------------------------------------------------------------------------*/

typedef struct CHANGE {
  struct CHANGE *next;
  char *old;
  char *new;
  int isdir; /* 0=file, 1=dir, 2=skip */
} CHANGE;

CHANGE *change_root = NULL;

static int
any_changes_done(void)
{
  return change_root != NULL;
}

static void
dump_changes(void)
{
  CHANGE *c;
  for (c = change_root; c; c = c->next)
    fprintf(change_file, "%s -> %s\n", c->old, c->new);
}

int
change(char *fname, const char *problem, int isadir)
{
  CHANGE *ch;
  char new[PATH_MAX];
  char *pos;

  for (ch = change_root; ch; ch = ch->next)
  {
    size_t old_len = strlen(ch->old);

    if ((strncmp(fname, ch->old, old_len) == 0) && ch->isdir
        /* Don't use change rules which failed to work before.  */
        && access(ch->new, D_OK) == 0
        /* Don't be tricked if fname has ch->old as its substring.  */
        && (fname[old_len] == '\0' || fname[old_len] == '/'))
    {
      if (ch->isdir == 2)
      {
        fprintf(log_out, "  [ skipping %s ]\n", fname);
        return 0;
      }
/*      fprintf(log_out, "  [ changing %s to ", fname); */
      sprintf(new, "%s%s", ch->new, fname + old_len);
      strcpy(fname, new);
/*      fprintf(log_out, "%s ]\n", fname); */
      return 1;
    }
  }
  
  if (a_switch)
  {
    fprintf(log_out, "  %s %s\n", problem, fname);
    fflush(log_out);
    pos = new;
    pos[0] = '\n';
    pos[1] = '\0';
  }
  else
  {
    fprintf(log_out, "  %s %s\n  new name : ", problem, fname);
    fflush(log_out);
    new[0] = '\0';
    pos = fgets(new, sizeof(new), stdin);
  }
  if (!pos)
    Fatal("EOF while reading stdin");
  for (; *pos; ++pos)
  {
    if (*pos == '\n')
    {
      *pos = '\0';
      break;
    }
  }

  if ((new[0] == '\0') && (isadir == 2))
    return 0;
  if (isadir) isadir = 1;
  ch = (CHANGE *)xmalloc(sizeof(CHANGE));
  ch->next = change_root;
  change_root = ch;
  ch->old = xstrdup(fname);
  pos = strrchr(fname, '/');
  if (pos && (strchr(new, '/') == 0))
  {
    if (new[0] == 0)
      ch->new = skipped_str;
    else
    {
      ch->new = (char *)xmalloc(strlen(new) + (pos - fname) + 2);
      *pos = 0;
      sprintf(ch->new, "%s/%s", fname, new);
    }
  }
  else if (new[0] == 0)
    ch->new = skipped_str;
  else
    ch->new = xstrdup(new);
  ch->isdir = isadir;
  strcpy(fname, ch->new);
  if (new[0] == 0)
  {
    ch->isdir = 2;
    return 0;
  }
  return 1;
}

/*------------------------------------------------------------------------*/

int
isadir(char *n)
{
#ifdef __DJGPP__
  return (access(n, D_OK) == 0);
#else
  union REGS r;
  struct SREGS s;
  r.x.ax = 0x4300;
  r.x.dx = FP_OFF(n);
  s.ds = FP_SEG(n);
  int86x(0x21, &r, &r, &s);
  if (r.x.flags & 1)
    return 0;
  return r.x.cx & 0x10;
#endif
}

void
do_directories(char *n)
{
  char tmp[PATH_MAX];
  char *sl;
  int r;
  sprintf(tmp, "%s", n);
  n = tmp;
  for (sl = n; *sl; sl++)
  {
    if (sl > tmp && (*sl == '/' || *sl == '\\') && sl[-1] != ':')
    {
      char save = *sl;
      *sl = 0;
      do {
        if (isadir(n))
          break;
        r = mkdir (n, 0777);
        if (r)
          r = change(n, "Unable to create directory", 1);
      } while (r);
      *sl = save;
    }
  }
}

/*------------------------------------------------------------------------*/

/* Guess DOS file type by looking at its contents.  */
File_type
guess_file_type(char *buf, register size_t buflen)
{
  int crlf_seen = 0;
  /* Use unsigned char, so this will work with foreign characters.  */
  register unsigned char *bp = (unsigned char *)buf;

  while (buflen--)
  {
    /* Binary files have characters with ASCII code less then 32 decimal,
       unless they are one of: BS (for man pages), TAB, LF, FF, CR, ^Z. */
    if (*bp  < ' '  && !(*bp > 7 && *bp <= '\n') &&
        *bp != '\f' &&   *bp != '\r' && *bp != '\32')
      return DOS_BINARY;

    /* CR before LF means DOS text file (unless we later see
       binary characters).  */
    else if (*bp == '\r' && buflen && bp[1] == '\n')
      crlf_seen++;

    bp++;
  }

  return crlf_seen ? DOS_TEXT : UNIX_TEXT;
}

/*------------------------------------------------------------------------*/
char new[2048];  /* got to think about LFN's! */

char *
get_new_name(char *name_to_change, int *should_be_written)
{
  char *changed_name;
  struct skip_dir_list *skip_dir_entry;
  struct only_dir_list *only_dir_entry;

  /* ONLY_DIR says to extract only files which are siblings
     of that directory.  */
  *should_be_written = list_only == 0;

  if (*should_be_written)
  {
    skip_dir_entry = skip_dirs;
    while (skip_dir_entry)
    {
      if (!strncmp(skip_dir_entry->skip_dir, name_to_change, 
                   strlen(skip_dir_entry->skip_dir)))
      {
        char *following_char = name_to_change + strlen(name_to_change);
        if ((*following_char == '\0') || (*following_char == '/') ||
            (*following_char == '\\'))
          break;
      }
      skip_dir_entry = skip_dir_entry->next;
    }
    if (skip_dir_entry)
      *should_be_written = 0;
    else if (only_dirs)
    {
      only_dir_entry = only_dirs;
      while (only_dir_entry)
      {
        if (!strncmp(only_dir_entry->only_dir, name_to_change,
                     strlen(only_dir_entry->only_dir)))
          break;
        only_dir_entry = only_dir_entry->next;
      }
      if (!only_dir_entry)
        *should_be_written = 0;
    }
  }

  changed_name = get_entry(name_to_change);
  if (*should_be_written && !to_stdout && NO_LFN(changed_name))
  {
    static char info_[] = ".info-";
    static char _bzip2[] = ".bzip2", _bz2[] = ".bz2";
    static char *_tar_bz_extension[] = { ".tar.bz", ".tar.bz2", ".tar.bzip2", NULL};
    static char _tbz[] = ".tbz";
    static char _tar_gz[] = ".tar.gz", _tgz[] = ".tgz";
    static char xx[] = "++";
    char *bz2, *info, *tbz, *tgz, *plus;
    int i = 0;

    strcpy(new, changed_name);
    info = strstr(new, info_);
    if (info && isdigit(info[sizeof(info_) - 1]))
    {
      strcpy(info + 2, info + sizeof(info_) - 1);
      fprintf(log_out, "[ changing %s to %s ]\n", changed_name, new);
    }
    tgz = strstr(new, _tar_gz);
    if (tgz && tgz[sizeof(_tar_gz) - 1] == '\0')
    {
      strcpy(tgz, _tgz);
      fprintf(log_out, "[ changing %s to %s ]\n", changed_name, new);
    }
    while (_tar_bz_extension[i])
    {
      tbz = strstr(new, _tar_bz_extension[i]);
      if (tbz && tbz[strlen(_tar_bz_extension[i])] == '\0')
      {
        strcpy(tbz, _tbz);
        fprintf(log_out, "[ changing %s to %s ]\n", changed_name, new);
      }
      i++;
    }
    bz2 = strstr(new, _bzip2);
    if (bz2 && bz2[sizeof(_bzip2) - 1] == '\0')
    {
      strcpy(bz2, _bz2);
      fprintf(log_out, "[ changing %s to %s ]\n", changed_name, new);
    }
    plus = strstr(new, xx);
    if (plus)
    {
      register char *s = plus;

      while (s)
      {
        *s++ = 'x';
        *s++ = 'x';
        s = strstr(s, xx);
      }
      fprintf(log_out, "[ changing %s to %s ]\n", changed_name, new);
    }
    changed_name = new;

    if (dot_switch)
    {
      char *p = changed_name, *name_start = changed_name;
      int state = 0;
      /* 0 = start of item
         1 = before dot (not counting initial dot), but not first char
         2 = after first dot */

      /* ".gdbinit" -> "_gdbinit"
         "emacs-19.28.90/ChangeLog" -> "emacs-19.28-90/ChangeLog"
         "./dir/file" -> "./dir/file"
         "sh.lex.c" -> "sh_lex.c"
      */
      while (*p)
      {
        switch (*p++)
        {
          case '/':
          case '\\':
            state = 0;
            name_start = p;
            break;
          case '.':
            /* Don't waste our limited 8-char real estate in the
               name part too early, unless the name is really short. */
            if (state == 1 && p - name_start < 5)
            {
              size_t namelen    = strlen(p);
              char *next_slash  = memchr(p, '/', namelen);
              char *next_bslash = memchr(p, '\\', namelen);
              char *next_dot    = memchr(p, '.', namelen);

              /* Find where this name ends.  */
              if (next_slash == (char *)0)
              {
                if (next_bslash)
                  next_slash = next_bslash;
                else
                  next_slash = p + namelen;
              }
              else if (next_bslash && next_bslash < next_slash)
                next_slash = next_bslash;

              /* If name has more dots, convert this one to `_'. */
              if (next_dot && next_dot < next_slash)
              {
                p[-1] = '_';
                break;      /* don't bump `state' */
              }
            }

            /* Leave "./", "../", "/." etc. alone.  */
            if (state != 0 ||
                (*p && *p != '/' && *p != '\\' && *p != '.'))
              p[-1] = "_.-"[state];
            if (state < 2) state++;
            break;
          default:
            if (state == 0) state = 1;
        }
      }
    }
  }
  return changed_name;
}

/*------------------------------------------------------------------------*/

/* We could have a file in a Unix archive whose name is reserved on
   MS-DOS by a device driver.  Trying to extract such a file would
   fail at best and wedge us at worst.  We need to rename such files.  */
void
rename_if_dos_device(char *fn)
{
  char *base = strrchr(fn, '/');
  struct stat st_buf;

  /* We don't care about backslashified file names because archives
     created on DOS cannot possibly include DOS device names.  */
  if (base)
    base++;
  else
    base = fn;

  /* The list of character devices is not constant: it depends on
     what device drivers did they install in their CONFIG.SYS.
     `stat' will tell us if the basename of the file name is a
     character device.  */
  if (stat(base, &st_buf) == 0 && S_ISCHR(st_buf.st_mode))
  {
    char orig[PATH_MAX];

    strcpy(orig, fn);
    /* Prepend a '_'.  */
    memmove(base + 1, base, strlen(base) + 1);
    base[0] = '_';
    fprintf(log_out, "[ changing %s to %s ]\n", orig, fn);
  }
}

/*------------------------------------------------------------------------*/

void
make_directory(char *dirname)
{
  int status;

  do_directories(dirname);        /* make sure parent exists */
  do {
    if (strcmp(dirname, ".") == 0)
      status = 0;                /* current dir always exists */
    else if (strcmp(dirname, "..") == 0)
      status = !isadir(dirname); /* root might have no parent */
    else
      status = mkdir(dirname, 0755);
    if (status && (errno == EEXIST))
    {
      status = change(dirname, "Duplicate directory name", 2);
      continue;
    }
    if (status)
      status = change(dirname, "Unable to create directory", 1);
    else
      fprintf(log_out, "Making directory %s\n", dirname);
  } while (status);
}

/*------------------------------------------------------------------------*/

FILE *log_out = stdout;
static char djtarx[] = "djtarx.exe";
static char djtart[] = "djtart.exe";

int
main(int argc, char **argv)
{
  int i = 1;
  char *tp;
  char *xp;
  struct skip_dir_list *skip_entry;
  struct only_dir_list *only_entry;

  progname = strlwr(xstrdup(argv[0]));

  if (argc < 2)
  {
    fprintf(stderr, "Usage: %s [-n changeFile] [-p] [-i] [-t|x] [-e dir] [-o dir] [-v] [-u|d|b] [-[!].] [-!s] [-a] tarfile...\n", progname);
    exit(1);
  }

  /* DJTARX -> ``djtar -x'', DJTART -> ``djtar -t''.  */
  tp = strstr(progname, djtart);
  xp = strstr(progname, djtarx);
  /* Check both with and without .exe, just in case.  */
  if (tp && (tp[sizeof(djtart) - 1] == '\0' || tp[sizeof(djtart) - 5] == '\0'))
    list_only = 1;
  else if (xp && (xp[sizeof(djtarx) - 1] == '\0' || xp[sizeof(djtarx) - 5] == '\0'))
    list_only = 0;

  while ((argc > i) && (argv[i][0] == '-') && argv[i][1])
  {
    switch (argv[i][1])
    {
      case 'a':
        a_switch = 1;
        break;
      case 'n':
        do_name_changes(argv[++i]);
        break;
      case 'v':
        v_switch = 1;
        break;
      case 'u':
        text_unix = 1;
        text_dos = 0;
        break;
      case 'd':
        text_dos = 1;
        text_unix = 0;
        break;
      case 'b':
        text_dos = 0;
        text_unix = 0;
        break;
      case '.':
        dot_switch = 1;
        break;
      case '!':
        if (argv[i][2] == '.')
          dot_switch = 0;
        else if (argv[i][2] == 's')
          s_switch = 0;
        break;
      case 'e':
        skip_entry = xmalloc(sizeof(struct skip_dir_list));
        skip_entry->skip_dir = xstrdup(argv[++i]);
        skip_entry->next = skip_dirs;
        skip_dirs = skip_entry;
        break;
      case 'o':
        only_entry = xmalloc(sizeof(struct only_dir_list));
        only_entry->only_dir = xstrdup(argv[++i]);
        only_entry->next = only_dirs;
        only_dirs = only_entry;
        break;
      case 'p':
        to_stdout = 1;
        to_tty = isatty(fileno(stdout));
        log_out = stderr;
        text_dos = 1;
        break;
      case 't':
        list_only = 1;
        break;
      case 'i':
        ignore_csum = 1;
        break;
      case 'x':
        list_only = 0;
    }
    i++;
  }

  for (; i < argc; i++)
    if(stricmp(argv[i] + strlen(argv[i]) - 4, ".zip") == 0)
      epunzip_read(argv[i]);
    else
      tar_gz_read(argv[i]);

  if (to_stdout)
  {
    setmode(fileno(stdout), O_TEXT);
    return 0;
  }
  else if (any_changes_done())
  {
    change_file = fopen("tarchange.lst", "w");
    if (change_file != (FILE *)0)
    {
      dump_changes();
      fclose(change_file);
    }
    else
      return 1;
  }
  return 0;
}
