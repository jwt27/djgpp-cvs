/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>		/* For FILENAME_MAX */
#include <stdlib.h>
#include <errno.h>		/* For errno */
#include <ctype.h>		/* For tolower */
#include <string.h>		/* For strlen() */
#include <fcntl.h>		/* For LFN stuff */
#include <go32.h>
#include <dpmi.h>		/* For dpmisim */
#include <crt0.h>		/* For crt0 flags */
#include <sys/stat.h>
#include <libc/dosio.h>

static unsigned use_lfn;

static char *__get_current_directory(char *out, int drive_number);

static char *
__get_current_directory(char *out, int drive_number)
{
  __dpmi_regs r;
  char tmpbuf[FILENAME_MAX];

  memset(&r, 0, sizeof(r));
  if(use_lfn)
    r.x.ax = 0x7147;
  else
    r.h.ah = 0x47;
  r.h.dl = drive_number + 1;
  r.x.si = __tb_offset;
  r.x.ds = __tb_segment;
  __dpmi_int(0x21, &r);

  if (r.x.flags & 1)
  {
    errno = r.x.ax;
    return out;
  }
  else
  {
    dosmemget(__tb, sizeof(tmpbuf), tmpbuf);
    strcpy(out+1,tmpbuf);

    /* Root path, don't insert "/", it'll be added later */
    if (*(out + 1) != '\0')
      *out = '/';
    else
      *out = '\0';
    return out + strlen(out);
  }
}

__inline__ static int
is_slash(int c)
{
  return c == '/' || c == '\\';
}

__inline__ static int
is_term(int c)
{
  return c == '/' || c == '\\' || c == '\0';
}

/* Takes as input an arbitrary path.  Fixes up the path by:
   1. Removing consecutive slashes
   2. Removing trailing slashes
   3. Making the path absolute if it wasn't already
   4. Removing "." in the path
   5. Removing ".." entries in the path (and the directory above them)
   6. Adding a drive specification if one wasn't there
   7. Converting all slashes to '/'
 */
void
_fixpath(const char *in, char *out)
{
  int		drive_number;
  const char	*ip = in;
  char		*op = out;
  int		preserve_case = _preserve_fncase();
  char		*name_start;

  use_lfn = _USE_LFN;

  /* Add drive specification to output string */
  if (((*ip >= 'a' && *ip <= 'z') ||
       (*ip >= 'A' && *ip <= 'Z'))
      && (*(ip + 1) == ':'))
  {
    if (*ip >= 'a' && *ip <= 'z')
    {
      drive_number = *ip - 'a';
      *op++ = *ip++;
    }
    else
    {
      drive_number = *ip - 'A';
      if (*ip <= 'Z')
	*op++ = drive_number + 'a';
      else
	*op++ = *ip;
      ++ip;
    }
    *op++ = *ip++;
  }
  else
  {
    __dpmi_regs r;
    r.h.ah = 0x19;
    __dpmi_int(0x21, &r);
    drive_number = r.h.al;
    *op++ = drive_number + (drive_number < 26 ? 'a' : 'A');
    *op++ = ':';
  }

  /* Convert relative path to absolute */
  if (!is_slash(*ip))
    op = __get_current_directory(op, drive_number);

  /* Step through the input path */
  while (*ip)
  {
    /* Skip input slashes */
    if (is_slash(*ip))
    {
      ip++;
      continue;
    }

    /* Skip "." and output nothing */
    if (*ip == '.' && is_term(*(ip + 1)))
    {
      ip++;
      continue;
    }

    /* Skip ".." and remove previous output directory */
    if (*ip == '.' && *(ip + 1) == '.' && is_term(*(ip + 2)))
    {
      ip += 2;
      /* Don't back up over drive spec */
      if (op > out + 2)
	/* This requires "/" to follow drive spec */
	while (!is_slash(*--op));
      continue;
    }

    /* Copy path component from in to out */
    *op++ = '/';
    while (!is_term(*ip)) *op++ = *ip++;
  }

  /* If root directory, insert trailing slash */
  if (op == out + 2) *op++ = '/';

  /* Null terminate the output */
  *op = '\0';

  /* switch FOO\BAR to foo/bar, downcase where appropriate */
  for (op = out + 3, name_start = op - 1; *name_start; op++)
  {
    char long_name[FILENAME_MAX], short_name[13];

    if (*op == '\\')
      *op = '/';
    if (!preserve_case && (*op == '/' || *op == '\0'))
    {
      memcpy(long_name, name_start+1, op - name_start - 1);
      long_name[op - name_start - 1] = '\0';
      if (!strcmp(_lfn_gen_short_fname(long_name, short_name), long_name))
      {
	while (++name_start < op)
	  if (*name_start >= 'A' && *name_start <= 'Z')
	    *name_start += 'a' - 'A';
      }
      else
	name_start = op;
    }
    else if (*op == '\0')
      break;
  }
}

#ifdef TEST

int main (int argc, char *argv[])
{
  char fixed[FILENAME_MAX];
  if (argc > 1)
    {
      _fixpath (argv[1], fixed);
      printf ("You mean %s?\n", fixed);
    }
  return 0;
}

#endif
