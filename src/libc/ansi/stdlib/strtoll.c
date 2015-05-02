/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <libc/unconst.h>

long long int
strtoll(const char *nptr, char **endptr, int base)
{
  const unsigned char *s = (const unsigned char *) nptr;
  unsigned long long int acc;
  unsigned char c;
  unsigned long long int cutoff;
  int neg = 0, any, cutlim;

  /*
   * See strtol for comments as to the logic used.
   */
  do {
    c = *s++;
  } while (isspace(c));
  if (c == '-')
  {
    neg = 1;
    c = *s++;
  }
  else if (c == '+')
    c = *s++;
  if ((base == 0 || base == 16) &&
      c == '0' && (*s == 'x' || *s == 'X'))
  {
    c = s[1];
    s += 2;
    base = 16;
  }
  if (base == 0)
    base = c == '0' ? 8 : 10;

/* to prevent overflow, we take max-1 and add 1 after division */
  cutoff = neg ? -(LLONG_MIN+1) : LLONG_MAX-1;
  cutlim = cutoff % base;
  cutoff /= base;
  if (++cutlim == base)
  {
    cutlim = 0;
    cutoff++;
  }
  for (acc = 0, any = 0; ; c = *s++)
  {
    if (isdigit(c))
      c -= '0';
    else if (isalpha(c))
      c -= isupper(c) ? 'A' - 10 : 'a' - 10;
    else
      break;
    if (c >= base)
      break;
    if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
      any = -1;
    else
    {
      any = 1;
      acc *= base;
      acc += c;
    }
  }
  if (any < 0)
  {
    acc = neg ? LLONG_MIN : LLONG_MAX;
    errno = ERANGE;
  }
  else if (neg)
    acc = -acc;
  if (endptr != 0)
    *endptr = any ? (char *) unconst(s, unsigned char *) - 1 : unconst(nptr, char *);
  return acc;
}
