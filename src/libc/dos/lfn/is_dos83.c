/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
#include <fcntl.h>

int _is_DOS83(const char *fname)
{
  const char *s = fname;
  const char *e;
  char c, period_seen;

  if (*s == '.') {
    if (s[1] == 0)
      return 1;				/* "." is valid */
    if (s[1] == '.' && s[2] == 0)
      return 1;				/* ".." is valid */
    return 0;				/* starting period invalid */
  }

  period_seen = 0;
  e = s + 8;				/* end */

  while ((c = *s++))
    if (c == '.') {
      if(period_seen)
        return 0;			/* multiple periods invalid */
      period_seen = 1;
      e = s + 3;			/* already one past period */
    } else if (s > e)
      return 0;				/* name component too long */
    else if (c >= 'a' && c <= 'z')
      return 0;				/* lower case character */
    else if (c == '+' || c == ',' || c == ';' || c == ' ' ||
             c == '=' || c == '[' || c == ']')
      return 0;				/* special non-DOS characters */

  return 1;				/* all chars OK */
}

#ifdef TEST
#include <stdio.h>
#include <crt0.h>

int _crt0_startup_flags = _CRT0_FLAG_PRESERVE_FILENAME_CASE; /* for glob */

#define MAXDISPLAY 10

/* Example test usage: id83 * (or a file name, or ... can test whole disk) */

int main (int argc, char *argv[])
{
  char old, new, dif;
  char sh[14];
  char *f;
  int i,j,nd;
  nd = 0;
  for(i=1;i<argc;i++) {
    f = argv[i];
    for(j=strlen(f); j >= 0; j--)	/* Trim path */
      if(*(f+j) == '/') {
        f = f + j + 1;
        break;
      }
    old = !strcmp(_lfn_gen_short_fname(f, sh), f);
    new = _is_DOS83(f);
    dif = (old != new);
    if(dif)
      nd++;
    if(i == MAXDISPLAY)
      printf("Remaining test results suppressed unless different\n");
    if(dif || i < MAXDISPLAY)
      printf ("isDOS: %d old: %d Name: %s\n", new, old, f);
  }
  if(i >= MAXDISPLAY)
    printf("%d names processed, %d differences\n",i,nd);
  return 0;
}
#endif
