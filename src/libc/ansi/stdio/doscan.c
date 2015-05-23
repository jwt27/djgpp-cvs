/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <stddef.h>
#include <errno.h>
#include <libc/file.h>
#include <libc/local.h>

typedef enum {
  false = 0, true = 1
} bool;

#define SPC               01
#define STP               02

#define CHAR              0
#define SHORT             1
#define REGULAR           2
#define LONG              4
#define LONGDOUBLE        8

#define INT               0
#define FLOAT             1

#define DEFAULT_WIDTH     30000
#define BUFFER_INCREMENT  128

static int _innum(int *ptr, int type, int len, int size, FILE *iop,
                  int (*scan_getc)(FILE *), int (*scan_ungetc)(int, FILE *),
                  int *eofptr, const bool allocate_char_buffer);
static int _instr(char *ptr, int type, int len, FILE *iop,
                  int (*scan_getc)(FILE *), int (*scan_ungetc)(int, FILE *),
                  int *eofptr, const bool allocate_char_buffer);
static const char *_getccl(const unsigned char *s);

static char _sctab[256] = {
  0,0,0,0,0,0,0,0,
  0,SPC,SPC,SPC,SPC,SPC,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  SPC,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
};

static int nchars = 0;
static char decimal_point = '.';

int
_doscan(FILE *iop, const char *fmt, va_list argp)
{
  return(_doscan_low(iop, fgetc, ungetc, fmt, argp));
}

int
_doscan_low(FILE *iop, int (*scan_getc)(FILE *), int (*scan_ungetc)(int, FILE *), const char *fmt, va_list argp)
{
  register int ch;
  int nmatch, n, len, ch1;
  int *ptr, fileended, size;
  int suppressed;
  bool allocate_char_buffer;
  int previous_errno = errno;
  const va_list arg_list = argp;
  bool retrieve_arg_ptr;

  decimal_point = localeconv()->decimal_point[0];
  nchars = 0;
  nmatch = 0;
  fileended = 0;
  suppressed = 0;
  errno = 0;

  for (;;)
  {
    switch (ch = *fmt++)
    {
    case '\0':
      return nmatch;
    case '%':
      if ((ch = *fmt++) == '%')
        goto def;

      retrieve_arg_ptr = true;
      allocate_char_buffer = false;
      ptr = NULL;
repeat:
      if (ch != '*' && retrieve_arg_ptr)
        ptr = va_arg(argp, int *);
      else
        ch = *fmt++;
      n = len = 0;
      size = REGULAR;
      while (isdigit(ch & 0xff))
      {
        n = n * 10 + ch - '0';
        ch = *fmt++;
      }
      if (ch == '$')
      {
        /* C99 */
        /* for %n$ numeric conversion specifier */
        int i;
        for (argp = arg_list, i = 0; i < n; i++)
          ptr = va_arg(argp, int *);
        retrieve_arg_ptr = false;
        goto repeat;
      }
      else
        len = (n == 0) ? DEFAULT_WIDTH : n;

      if (ch == 'l')
      {
        size = LONG;
        ch = *fmt++;
        if (ch == 'l')
        {
          size = LONGDOUBLE; /* for long long and long double 'll' format */
          ch = *fmt++;
        }
      }
      else if (ch == 'h')
      {
        size = SHORT;
        ch = *fmt++;
        if (ch == 'h')
        {
          /* C99 */
          /* for 'hh' - char */
          size = CHAR;
          ch = *fmt++;
        }
      }
      else if (ch == 'L')
      {
        size = LONGDOUBLE;
        ch = *fmt++;
      }
      else if (ch == 'j')
      {
        /* C99 */
        size = LONGDOUBLE; /* for long long */
        ch = *fmt++;
      }
      else if (ch == 'z')
      {
        /* C99 */
        size = LONG;
        ch = *fmt++;
      }
      else if (ch == 't')
      {
        /* C99 */
        size = REGULAR;
        ch = *fmt++;
      }
      else if (ch == 'm')
      {
        /* POSIX.1 and GNU glibc extension */
        allocate_char_buffer = true;
        ch = *fmt++;
        if (ch == '[')
          fmt = _getccl((const unsigned char *)fmt);
      }
      else if (ch == '[')
        fmt = _getccl((const unsigned char *)fmt);

      if (isupper(ch & 0xff))
      {
        /* ch = tolower(ch);
           gcc gives warning: ANSI C forbids braced
           groups within expressions */
        ch += 'a' - 'A';
        /* The following if clause is an extension of ANSI/Posix spec: it
           allows to use %D, %U, %I etc. to store value in a long (rather
           than an int) and %lD, %lU etc. to store value in a long long.
           This extension is supported by some compilers (e.g. Borland C).
           Old pre-ANSI compilers, such as Turbo C 2.0, also interpreted
           %E, %F and %G to store in a double (rather than a float), but
           this contradicts the ANSI Standard, so we don't support it.
           %X should be treated as per the ANSI Standard - no length
           is implied by the upper-case x. */
        if (ch == 'd' || ch == 'i' || ch == 'o' || ch == 'u')
        {
          if (size == LONG)
            size = LONGDOUBLE;
          else if (size != LONGDOUBLE)
            size = LONG;
        }
      }
      if (ch == '\0')
        return EOF;

      if (ch == 'n')
      {
        if (!ptr)
          break;
        if (size == LONG)
          *(long *)ptr = nchars;
        else if (size == CHAR)
          *(char *)ptr = nchars;
        else if (size == SHORT)
          *(short *)ptr = nchars;
        else if (size == LONGDOUBLE)
          *(long long *)ptr = nchars;
        else
          *(int *)ptr = nchars;
        break;
      }

      if (_innum(ptr, ch, len, size, iop, scan_getc, scan_ungetc, &fileended, allocate_char_buffer))
      {
        if (ptr)
          nmatch++;
        else
          suppressed = 1;
      }
      else
      {
        if ((fileended && !nmatch && !suppressed) || (allocate_char_buffer && errno == ENOMEM))
          return EOF;

        errno = previous_errno;
        return nmatch;
      }
      break;
    case ' ':
    case '\n':
    case '\t':
    case '\r':
    case '\f':
    case '\v':
      while (((nchars++, ch1 = scan_getc(iop)) != EOF) && (_sctab[ch1 & 0xff] & SPC))
        ;

      if (ch1 != EOF)
        scan_ungetc(ch1, iop);

      nchars--;
      break;

    default:
def:
      ch1 = scan_getc(iop);
      if (ch1 != EOF) nchars++;
      if (ch1 != ch)
      {
        if (ch1 == EOF)
          return (nmatch || suppressed ? nmatch : EOF);
        scan_ungetc(ch1, iop);
        nchars--;
        return nmatch;
      }
    }
  }
}

static int
_innum(int *ptr, int type, int len, int size, FILE *iop,
       int (*scan_getc)(FILE *), int (*scan_ungetc)(int, FILE *),
       int *eofptr, const bool allocate_char_buffer)
{
  register char *np;
  char numbuf[64];
  register int c, base;
  int expseen, scale, negflg, c1, ndigit;
  long long lcval;
  int cpos;

  if (type == 'c' || type == 's' || type == '[')
    return (_instr(ptr ? (char *)ptr : (char *)NULL, type, len,
                   iop, scan_getc, scan_ungetc, eofptr, allocate_char_buffer));
  lcval = 0;
  ndigit = 0;
  scale = INT;
  if (type == 'a' || type == 'e' || type == 'f' || type == 'g')
    scale = FLOAT;
  base = 10;
  if (type == 'o')
    base = 8;
  else if (type == 'x' || type == 'p' || type == 'a')
    base = 16;
  np = numbuf;

  expseen = 0;
  negflg = 0;
  while (((nchars++, c = scan_getc(iop)) != EOF) && (_sctab[c & 0xff] & SPC))
    ;
  if (c == EOF) nchars--;
  if (c == '-')
  {
    negflg++;
    *np++ = c;
    c = scan_getc(iop);
    nchars++;
    len--;
  }
  else if (c == '+')
  {
    len--;
    c = scan_getc(iop);
    nchars++;
  }

  cpos = 0;
  for (; --len >= 0; *np++ = c, c = scan_getc(iop), nchars++)
  {
    cpos++;
    if (c == '0' && cpos == 1 && type == 'i')
      base = 8;
    if ((c == 'x' || c == 'X') && (type == 'a' || type == 'i' || type == 'x')
        && cpos == 2 && lcval == 0)
    {
      base = 16;
      continue;
    }
    if ((c != EOF && isdigit(c & 0xff)  && c - '0' < base)
        || (base == 16 && (('a' <= c && c <= 'f') || ('A' <= c && c <= 'F'))))
    {
      ndigit++;
      if (base == 8)
        lcval <<= 3;
      else if (base == 10)
        lcval = ((lcval << 2) + lcval) << 1;
      else
        lcval <<= 4;
      c1 = c;
      if (isdigit(c & 0xff))
        c -= '0';
      else if ('a' <= c && c <= 'f')
        c -= 'a' - 10;
      else
        c -= 'A' - 10;
      lcval += c;
      c = c1;
      continue;
    }
    else if (c == decimal_point)
    {
      if (scale == INT || base == 8)
        break;
      ndigit++;
      continue;
    }
    else if ((c == 'e' || c == 'E') && expseen == 0)
    {
      if (scale == INT || base == 8 || ndigit == 0)
        break;
      expseen++;
      *np++ = c;
      c = scan_getc(iop);
      nchars++;
      if (c != '+' && c != '-' && ('0' > c || c > '9'))
        break;
    }
    else
      break;
  }

  if (negflg)
    lcval = -lcval;

  if (c != EOF)
  {
    scan_ungetc(c, iop);
    *eofptr = 0;
  }
  else
    *eofptr = 1;

  nchars--;
  if (np == numbuf || (negflg && np == numbuf + 1))  /* gene dykes*/
    return 0;

  if (ptr == NULL)
    return 1;

  *np++ = 0;
  switch((scale << 4) | size)
  {
  case (FLOAT << 4) | SHORT:
  case (FLOAT << 4) | REGULAR:
    *(float *)ptr = atof(numbuf);
    break;

  case (FLOAT << 4) | LONG:
    *(double *)ptr = atof(numbuf);
    break;

  case (FLOAT << 4) | LONGDOUBLE:
    *(long double *)ptr = _atold(numbuf);
    break;

  case (INT << 4) | CHAR:
    *(char *)ptr = (char)lcval;
    break;

  case (INT << 4) | SHORT:
    *(short *)ptr = (short)lcval;
    break;

  case (INT << 4) | REGULAR:
    *(int *)ptr = (int)lcval;
    break;

  case (INT << 4) | LONG:
    *(long *)ptr = (long)lcval;
    break;

  case (INT << 4) | LONGDOUBLE:
    *(long long *)ptr = lcval;
    break;
  }

  return 1;
}

static int
_instr(char *ptr, int type, int len, FILE *iop,
       int (*scan_getc)(FILE *), int (*scan_ungetc)(int, FILE *),
       int *eofptr, const bool allocate_char_buffer)
{
  register int ch;
  char *arg_ptr = NULL, *orig_ptr = NULL;
  size_t string_length;
  int ignstp;
  int matched = 0;
  size_t buffer_size = BUFFER_INCREMENT;

  *eofptr = 0;
  if (type == 'c' && len == DEFAULT_WIDTH)
    len = 1;

  if (allocate_char_buffer)
  {
    if (!len)
    {
      errno = ENOMEM;
      return 0;
    }
    else
    {
      arg_ptr = ptr;
      orig_ptr = ptr = malloc(buffer_size);
      if (!ptr)
      {
        errno = ENOMEM;
        return 0;
      }
    }
  }

  ignstp = 0;
  if (type == 's')
    ignstp = SPC;

  while ((string_length = nchars++, ch = scan_getc(iop)) != EOF && _sctab[ch & 0xff] & ignstp)
    ;

  ignstp = SPC;
  if (type == 'c')
    ignstp = 0;
  else if (type == '[')
    ignstp = STP;

  while (ch != EOF && (_sctab[ch & 0xff] & ignstp) == 0)
  {
    matched = 1;
    if (ptr)
      *ptr++ = ch;

    if (allocate_char_buffer && type != 'c')
    {
      if (--buffer_size < 1)
      {
        const ptrdiff_t offset = ptr - orig_ptr;
        char *new_ptr = realloc(orig_ptr, (size_t)(offset + BUFFER_INCREMENT));
        if (!new_ptr)
        {
          free(orig_ptr);
          errno = ENOMEM;
          return 0;
        }
        orig_ptr = new_ptr;
        ptr = orig_ptr + offset;
        buffer_size = BUFFER_INCREMENT;

        if (--len < 1)
          len = DEFAULT_WIDTH;
      }
    }
    else if (--len < 1)
      break;

    ch = scan_getc(iop);
    nchars++;
  }

  if (ch != EOF)
  {
    if (len > 0)
    {
      scan_ungetc(ch, iop);
      nchars--;
    }
    *eofptr = 0;
  }
  else
  {
    nchars--;
    *eofptr = 1;
  }

  if (matched)
  {
    string_length = nchars - string_length;
    if (ptr && type != 'c')
    {
      *ptr++ = '\0';
      string_length++;
    }
    if (allocate_char_buffer)
    {
      *(char **)arg_ptr = realloc(orig_ptr, string_length);
      ptr = arg_ptr;
      if (!*ptr)
      {
        free(orig_ptr);
        errno = ENOMEM;
        return 0;
      }
    }

    return 1;
  }

  return 0;
}

static const char *
_getccl(const unsigned char *s)
{
  register int t;
  size_t c;

  t = 0;
  if (*s == '^')
  {
    t++;
    s++;
  }

  for (c = 0; c < (sizeof _sctab / sizeof _sctab[0]); c++)
  {
    if (t)
      _sctab[c] &= ~STP;
    else
      _sctab[c] |= STP;
  }

  if ((c = *s) == ']' || c == '-')
  { /* first char is special */
    if (t)
      _sctab[c] |= STP;
    else
      _sctab[c] &= ~STP;
    s++;
  }

  while ((c = *s++) != ']')
  {
    if (c == 0)
      return (const char *)--s;
    else if (c == '-' && *s != ']' && s[-2] < *s)
    {
      for (c = s[-2] + 1; c < *s; c++)
        if (t)
          _sctab[c & 0xff] |= STP;
        else
          _sctab[c & 0xff] &= ~STP;
    }
    else if (t)
      _sctab[c & 0xff] |= STP;
    else
      _sctab[c & 0xff] &= ~STP;
  }

  return (const char *)s;
}
