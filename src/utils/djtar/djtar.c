/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <dos.h>
#include <time.h>
#include <io.h>
#include <crt0.h>

#define NO_LFN (_crt0_startup_flags & _CRT0_FLAG_NO_LFN)

#include "oread.h"
#include "zread.h"

static void
Fatal(const char *msg)
{
  fprintf(stderr, "Fatal! %s!\n", msg);
  exit(1);
}

/*------------------------------------------------------------------------*/

typedef struct CE {
  struct CE *next;
  char *from;
  char *to;
} CE;

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
  unsigned long h = hash(from);
  CE *ce = (CE *)malloc(sizeof(CE));
  if (ce == 0)
    Fatal("Out of memory");
  ce->from = strdup(from);
  ce->to = strdup(to);
  ce->next = htab[h];
  htab[h] = ce;
}

static char *
get_entry(char *from)
{
  CE *ce;
  for (ce = htab[hash(from)]; ce; ce=ce->next)
  {
    if (strcmp(ce->from, from) == 0)
      return ce->to;
  }
  return from;
}

static void
DoNameChanges(char *fname)
{
  FILE *f = fopen(fname, "r");
  char from[100], to[100];
  char line[250];
  if (f == 0)
  {
    perror(fname);
    exit(1);
  }
  while (1)
  {
    fgets(line, 250, f);
    if (feof(f))
      break;
    to[0] = 0;
    sscanf(line, "%s %s", from, to);
    if (to[0])
      store_entry(from, to);
  }
  fclose(f);
}

/*------------------------------------------------------------------------*/

FILE *change_file;

int v_switch = 0;
int dot_switch = 1;
int text_unix = 0;
int text_dos = 0;
int z_switch = 0;
int to_stdout = 0;
int to_tty = 0;
int ignore_csum = 0;
int list_only = 1;
char skipped_str[] = "[skipped]";

char *only_dir;

/*------------------------------------------------------------------------*/

typedef struct CHANGE {
  struct CHANGE *next;
  char *old;
  char *new;
  int isdir; /* 0=file, 1=dir, 2=skip */
} CHANGE;

CHANGE *change_root = 0;

static void
dump_changes(void)
{
  CHANGE *c;
  for (c=change_root; c; c=c->next)
    fprintf(change_file, "%s -> %s\n", c->old, c->new);
}

static int
change(char *fname, const char *problem, int isadir)
{
  CHANGE *ch;
  char new[200];
  char *pos;

  for (ch=change_root; ch; ch = ch->next)
    if ((strncmp(fname, ch->old, strlen(ch->old)) == 0) && ch->isdir)
    {
      if (ch->isdir == 2)
      {
        fprintf(log_out, "  [ skipping %s ]\n", fname);
        return 0;
      }
/*      fprintf(log_out, "  [ changing %s to ", fname); */
      sprintf(new, "%s%s", ch->new, fname+strlen(ch->old));
      strcpy(fname, new);
/*      fprintf(log_out, "%s ]\n", fname); */
      return 1;
    }
  
  fprintf(log_out, "  %s %s\n  new name : ", problem, fname);
  gets(new);

  if ((strcmp(new, "") == 0) && (isadir == 2))
    return 0;
  if (isadir) isadir=1;
  ch = (CHANGE *)malloc(sizeof(CHANGE));
  if (ch == 0)
    Fatal("Out of memory");
  ch->next = change_root;
  change_root = ch;
  ch->old = strdup(fname);
  pos = strrchr(fname, '/');
  if (pos && (strchr(new, '/') == 0))
  {
    if (new[0] == 0)
      ch->new = skipped_str;
    else
    {
      ch->new = (char *)malloc(strlen(new) + (pos-fname) + 2);
      if (ch->new == 0)
        Fatal("Out of memory");
      *pos = 0;
      sprintf(ch->new, "%s/%s", fname, new);
    }
  }
  else if (new[0] == 0)
    ch->new = skipped_str;
  else
    ch->new = strdup(new);
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

static int
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

static void
do_directories(char *n)
{
  char tmp[100];
  char *sl;
  int r;
  sprintf(tmp, n);
  n = tmp;
  for (sl=n; *sl; sl++)
  {
    if ((*sl == '/' || *sl == '\\') && sl[-1] != ':')
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

typedef enum { DOS_BINARY, DOS_TEXT, UNIX_TEXT } File_type;

/* Guess DOS file type by looking at its contents.  */
static File_type
guess_file_type(char *buf, register size_t buflen)
{
  int crlf_seen = 0;
  /* Use unsigned char, so this will work with foreign characters.  */
  register unsigned char *bp = buf;

  while (buflen--)
    {
      /* Binary files have characters with ASCII code less then 32 decimal,
         unless they are one of: BS (for man pages), TAB, LF, FF, CR, ^Z. */
      if (*bp  < ' '  && !(*bp > 7 && *bp <= '\n') &&
          *bp != '\f' &&   *bp != '\r' && *bp != '\32')
        return DOS_BINARY;

      /* CR before LF means DOS text file (unless we later see
         binary characters).  */
      else if (*bp == '\r' && bp[1] == '\n')
        crlf_seen++;

      bp++;
    }

  return crlf_seen ? DOS_TEXT : UNIX_TEXT;
}

/*------------------------------------------------------------------------*/

typedef struct {
  char name[100];
  char operm[8];
  char ouid[8];
  char ogid[8];
  char osize[12];
  char otime[12];
  char ocsum[8];
  char flags[1];
  char filler[355];
} TARREC;

static TARREC header;
static int looking_for_header;
static char *changed_name;
static int first_block = 1;
static File_type file_type = DOS_BINARY;
static long perm, uid, gid, size;
static long posn = 0;
static time_t ftime;
static struct ftime ftimes;
static struct tm *tm;
static int r;
static int skipping;
static char new[2048];  /* got to think about LFN's! */

int
tarread(char *buf, long buf_size)
{
  int should_be_written;

  while (buf_size)
  {
    char *info;
    int write_errno = 0;
    int dsize = 512, wsize;

    if (skipping)
    {
      if (skipping <= buf_size)
      {
        bytes_out += skipping;
        buf      += skipping;
        buf_size -= skipping;
        skipping  = 0;
        looking_for_header = 1;
        if (buf_size < sizeof header)
          return 0;
      }
      else
      {
        bytes_out += buf_size;
        skipping -= buf_size;
        return 0;
      }
    }

    if (looking_for_header)
    {
      int head_csum = 0;
      int i;

      memcpy(&header, buf, sizeof header);
      if (header.name[0] == 0)
      {
        bytes_out += buf_size;  /* assume everything left should be counted */
        return EOF;
      }
      buf += sizeof header;
      buf_size -= sizeof header;
      bytes_out += sizeof header;
      first_block = 1;
      file_type = DOS_BINARY;
      looking_for_header = 0;

      /* ONLY_DIR says to extract only files which are siblings
         of that directory.  */
      should_be_written = list_only == 0;
      if (should_be_written &&
          only_dir && strncmp(only_dir, header.name, strlen(only_dir)))
        should_be_written = 0;
      sscanf(header.operm, " %lo", &perm);
      sscanf(header.ouid, " %lo", &uid);
      sscanf(header.ogid, " %lo", &gid);
      sscanf(header.osize, " %lo", &size);
      sscanf(header.otime, " %o", &ftime);
      sscanf(header.ocsum, " %o", &head_csum);
      for (i = 0; i < sizeof header; i++)
      {
        /* Checksum on header, but with the checksum field blanked out.  */
        int j = (i > 147 && i < 156) ? ' ' : *((unsigned char *)&header + i);

        head_csum -= j;
      }
      if (head_csum && !ignore_csum)
      {
        /* Probably corrupted archive.  Bail out.  */
        fprintf(log_out, "--- !!Directory checksum error!! ---\n");
        bytes_out += buf_size;
        return EOF;
      }

      changed_name = get_entry(header.name);
      if (should_be_written && !to_stdout && NO_LFN)
      {
        info = strstr(changed_name, ".info-");
        if (info)
        {
          strcpy(new, changed_name);
          info = strstr(new, ".info-");
          strcpy(info+2, info+6);
          fprintf(log_out, "[ changing %s to %s ]\n", changed_name, new);
        }
        else
        {
          char *tgz = strstr(changed_name, ".tar.gz");
          if (tgz)
          {
            strcpy(new, changed_name);
            tgz = strstr(new, ".tar.gz");
            strcpy(tgz, ".tgz");
            strcat(tgz, tgz+7);
            fprintf(log_out, "[ changing %s to %s ]\n", changed_name, new);
          }
          else
          {
            char *plus = strstr(changed_name, "++"), *plus2;
            if (plus)
            {
              strcpy(new, changed_name);
              plus2 = strstr(new, "++");
              strcpy(plus2, "plus");
              strcpy(plus2+4, plus+2);
              fprintf(log_out, "[ changing %s to %s ]\n", changed_name, new);
            }
            else
            {
              strcpy(new, changed_name);
            }
          }
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

      if (v_switch)
        fprintf(log_out, "%08lx %6lo ", posn, perm);
      else
        fprintf(log_out, "%c%c%c%c ",
               S_ISDIR(perm)  ? 'd' : '-',
               perm & S_IRUSR ? 'r' : '-',
               perm & S_IWUSR ? 'w' : '-',
               perm & S_IXUSR ? 'x' : '-');
      fprintf(log_out, "%.20s %9ld %s", ctime(&ftime)+4, size, changed_name);
#if 0
      fprintf(log_out, "(out: %ld)", bytes_out);
#endif
      if (header.flags[1] == 0x32)
        fprintf(log_out, " -> %s", header.filler);
      fprintf(log_out, "%s\n",
              !should_be_written && !list_only ? "\t[ skipped ]" : "");
      posn += 512 + ((size+511) & ~511);
#if 0
      fprintf(log_out, "%6lo %02x %12ld %s\n",perm,header.flags[0],size,changed_name);
#endif
      if (should_be_written == 0)
      {
        skipping = (size+511) & ~511;
        continue;
      }
      else if (changed_name[strlen(changed_name)-1] == '/' && !to_stdout)
      {
        changed_name[strlen(changed_name)-1] = 0;
        do {
          if (strcmp(changed_name, ".") == 0)
            r = 0;        /* current dir always exists */
          else if (strcmp(changed_name, "..") == 0)
            r = !isadir(changed_name); /* root has no parent */
          else
            r = mkdir(changed_name
#ifdef __GO32__
                      ,0
#endif
                      );
          if (r && (errno==EACCES))
          {
            change(changed_name, "Duplicate directory name", 2);
            continue;
          }
          if (r)
            r = change(changed_name, "Unable to create directory", 1);
          else
            fprintf(log_out, "Making directory %s\n", changed_name);
        } while (r);
        looking_for_header = 1;
        continue;
      }
      else
      {
open_file:
        if (!to_stdout)
        {
          do_directories(changed_name);
          r = open(changed_name,
                   O_WRONLY | O_BINARY | O_CREAT | O_EXCL, S_IWRITE | S_IREAD);
          if (r < 0)
            if (change(changed_name, "Cannot exclusively open file", 0))
              goto open_file;
            else
            {
              skipping = (size+511) & ~511;
              continue;
            }
        }
        else
        {
          r = fileno(stdout);
          if (!to_tty)
            setmode(r, O_BINARY);
        }
      }
    }

    while (size)
    {
      char tbuf[512];
      char *wbuf = buf;

      if (buf_size <= 0)    /* this buffer exhausted */
        return 0;
      if (size < 512)
        dsize = size;
      else if (buf_size < 512)
        dsize = buf_size;
      else
        dsize = 512;
      if (first_block && (text_dos || text_unix || to_tty))
      {
        file_type = guess_file_type(buf, dsize);
        first_block = 0;
        if (file_type == UNIX_TEXT && text_dos)
          setmode(r, O_TEXT);   /* will add CR chars to each line */
      }
      if ((text_unix || to_tty) && file_type == DOS_TEXT)
      {
        /* If they asked for text files to be written Unix style, or
           we are writing to console, remove the CR and ^Z characters
           from DOS text files.
           Note that we don't alter the original uncompressed data so
           as not to screw up the CRC computations.  */
        char *s=buf, *d=tbuf;
        while (s-buf < dsize)
        {
          if (*s != '\r' && *s != 26)
            *d++ = *s;
          s++;
        }
        wsize = d - tbuf;
        wbuf = tbuf;
      }
      else
      {
        wbuf = buf;
        wsize = dsize;
      }
      errno = 0;
      if (write(r, wbuf, wsize) < wsize)
      {
        if (errno == 0)
          errno = ENOSPC;
        fprintf(log_out, "%s: %s\n", changed_name, strerror(errno));
        write_errno = errno;
        break;
      }
      size     -= dsize;
      buf_size -= dsize;
      buf      += dsize;
      bytes_out += dsize;
    }

    if (!to_stdout)
    {
      close(r);
      r = open(changed_name, O_RDONLY);
      tm = localtime(&ftime);
      ftimes.ft_tsec = tm->tm_sec / 2;
      ftimes.ft_min = tm->tm_min;
      ftimes.ft_hour = tm->tm_hour;
      ftimes.ft_day = tm->tm_mday;
      ftimes.ft_month = tm->tm_mon+1;
      ftimes.ft_year = tm->tm_year - 80;
      setftime(r, &ftimes);
      close(r);
      chmod(changed_name, perm);
    }
    looking_for_header = 1;
    if (write_errno == ENOSPC)  /* target disk full: quit early */
    {
      bytes_out += buf_size;
      return EOF;
    }
    else if (write_errno)       /* other error: skip this file, try next */
      skipping = (size - dsize + 511) & ~511;
    else    /* skip the slack garbage to the next 512-byte boundary */
      skipping = 512 - dsize;
  }
  return 0;
}

/*------------------------------------------------------------------------*/

int part_nb;
static const char *zip_description[] = {
  "uncompressed",
  "COMPRESS'ed",
  "PACK'ed",
  "LZH'ed",
  "compressed by unknown method (4)",
  "compressed by unknown method (5)",
  "compressed by unknown method (6)",
  "compressed by unknown method (7)",
  "deflated by "
};

static void
tar_gz_read(char *fname)
{
  void *f;

  errno = 0;
  f = oread_open(fname);
  if (errno)
  {
    fprintf(log_out, "%s: %s\n", fname, strerror(errno));
    return;
  }
  ifname = fname;
  header_bytes = 0;
  clear_bufs(); /* clear input and output buffers */
  part_nb = 0;  /* FIXME!! handle multi-part gzip's */
  method = get_method(f);
  if (method < 0)
  {
    oread_close(f);
    return;     /* error message already emitted */
  }
  if (v_switch)
    fprintf(log_out, "-- `%s\' is %s%s --\n\n",
            fname,
            method > MAX_METHODS ? "corrupted (?)" : zip_description[method],
            method == DEFLATED ? (pkzip ? "PKZip" : "GZip") : "");

  bytes_out = 0;
  looking_for_header = 1;
  posn = 0;

  if ((*decompressor)(f) != OK)
  {
    fprintf(log_out,
            "\n%s: corrupted file; I might have written garbage\n", fname);
    fflush(log_out);
  }
  oread_close(f);
  method = -1;
}

/*------------------------------------------------------------------------*/

FILE * log_out = stdout;
static char djtarx[] = "djtarx.exe";
static char djtart[] = "djtart.exe";

int
main(int argc, char **argv)
{
  int i = 1;
  char *tp;
  char *xp;

  progname = strlwr(strdup(argv[0]));

  if (argc < 2)
  {
    fprintf(stderr, "Usage: %s [-n changeFile] [-p] [-i] [-t|x] [-o dir] [-v] [-u|d|b] [-[!].] tarfile...\n", progname);
    exit(1);
  }

  /* DJTARX -> ``djtar -x'', DJTART -> ``djtar -t''.  */
  tp = strstr(progname, djtart);
  xp = strstr(progname, djtarx);
  /* Check both with and without .exe, just in case.  */
  if (tp && (tp[sizeof(djtart)-1] == '\0' || tp[sizeof(djtart)-5] == '\0'))
    list_only = 1;
  else if (xp && (xp[sizeof(djtarx)-1] == '\0' || xp[sizeof(djtarx)-5] == '\0'))
    list_only = 0;
  while ((argc > i) && (argv[i][0] == '-') && argv[i][1])
  {
    switch (argv[i][1])
    {
      case 'n':
        DoNameChanges(argv[i+1]);
        i++;
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
	break;
      case 'o':
        only_dir = strdup(argv[++i]);
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
    tar_gz_read(argv[i]);

  if (to_stdout)
  {
    setmode(fileno(stdout), O_TEXT);
    return 0;
  }
  else
  {
    change_file = fopen("/tarchange.lst", "w");
    if (change_file != (FILE *)0)
    {
      dump_changes();
      fclose(change_file);
      return 0;
    }
    else
      return 1;
  }
}
