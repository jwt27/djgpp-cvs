/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>		/* For FILENAME_MAX */
#include <errno.h>		/* For errno */
#include <string.h>		/* For strlen() */
#include <go32.h>
#include <dpmi.h>		/* FOR dpmisim */
#include <sys/stat.h>
#include <libc/dosio.h>

static char *__get_current_directory(char *out, int drive_number);

static char *
__get_current_directory(char *out, int drive_number)
{
  __dpmi_regs r;
  char tmpbuf[FILENAME_MAX];

  memset(&r, 0, sizeof(r));
  if(_USE_LFN)
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
  char	*op = out;

  /* Add drive specification to output string */
  if (((*ip >= 'a' && *ip <= 'z') ||
       (*ip >= 'A' && *ip <= 'Z'))
      && (*(ip + 1) == ':'))
  {
    if (*ip >= 'a' && *ip <= 'z')
      drive_number = *ip - 'a';
    else
      drive_number = *ip - 'A';
    *op++ = *ip++;
    *op++ = *ip++;
  }
  else
  {
    __dpmi_regs r;
    r.h.ah = 0x19;
    __dpmi_int(0x21, &r);
    drive_number = r.h.al;
    *op++ = drive_number + 'a';
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

  /* convert slashes (else we miss some) */
  for (op=out; *op; op++)
    if (*op == '\\')
      *op = '/';
}
