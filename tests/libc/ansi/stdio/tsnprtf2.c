/*
 * tsnprtf2.c - Test _doprnt() failure case for snprintf()
 */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

/* Simulate an encoding error by making _doprnt() fail every time. snprintf()
 * and vsnprintf() both invoke _doprnt(). */

int
_doprnt (const char *format, va_list args, FILE *file)
{
  return(-1);
}

int
main (void)
{
  char holder[24];
  int i;

  /* Test handling of encoding errors */
  i = snprintf(holder, sizeof(holder), "%s", "foo");

  if (i >= 0)
    {
      fputs("FAILED generating encoding error", stderr);
      exit(EXIT_FAILURE);
    }

  puts("SUCCESS");
  return(EXIT_SUCCESS);
}
