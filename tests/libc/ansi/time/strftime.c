/* this is the Autoconf test program that GNU programs use
   to detect if strftime is working.
   The results are checked against strftime() from GNU libc.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#ifdef DJGPP
/*  Do not include this if the code shall be compiled with linux too.  */
#include <libc/unconst.h>
#endif

static int
compare (const char *fmt, const struct tm *tm, const char *expected)
{
  char buf[99];
  strftime (buf, 99, fmt, tm);
  if (strcmp (buf, expected))
    {
#if 1
      printf ("fmt: \"%s\", expected \"%s\", got \"%s\"\n",
	      fmt, expected, buf);
#endif
      return 1;
    }
  return 0;
}

int
main (void)
{
  int n_fail = 0;
  struct tm *tm;
  time_t t = 738367; /* Fri Jan  9 13:06:07 1970 */
  tm = gmtime (&t);

  /* This is necessary to make strftime give consistent zone strings and
     e.g., seconds since the epoch (%s).  */
#ifdef DJGPP
  putenv (unconst("TZ=GMT0", char *));
#endif

#undef CMP
#define CMP(Fmt, Expected) n_fail += compare ((Fmt), tm, (Expected))

  CMP ("%-m", "1");		/* GNU */
  CMP ("%A", "Friday");
  CMP ("%^A", "FRIDAY");	/* The ^ is a GNU extension.  */
  CMP ("%B", "January");
  CMP ("%^B", "JANUARY");
  CMP ("%C", "19");		/* POSIX.2 */
  CMP ("%D", "01/09/70");	/* POSIX.2 */
  CMP ("%G", "1970");		/* GNU */
  CMP ("%H", "13");
  CMP ("%I", "01");
  CMP ("%M", "06");
  CMP ("%P", "pm");
  CMP ("%R", "13:06");		/* POSIX.2 */
  CMP ("%S", "07");
  CMP ("%T", "13:06:07");	/* POSIX.2 */
  CMP ("%U", "01");
  CMP ("%V", "02");
  CMP ("%W", "01");
  CMP ("%X", "13:06:07");
  CMP ("%Y", "1970");
  CMP ("%Z", "GMT");
  CMP ("%_m", " 1");		/* GNU */
  CMP ("%a", "Fri");
  CMP ("%^a", "FRI");
  CMP ("%b", "Jan");
  CMP ("%^b", "JAN");
  CMP ("%c", "Fri Jan  9 13:06:07 1970");
  CMP ("%^c", "FRI JAN  9 13:06:07 1970");
  CMP ("%d", "09");
  CMP ("%e", " 9");		/* POSIX.2 */
  CMP ("%g", "70");		/* GNU */
  CMP ("%h", "Jan");		/* POSIX.2 */
  CMP ("%^h", "JAN");
  CMP ("%j", "009");
  CMP ("%k", "13");		/* GNU */
  CMP ("%l", " 1");		/* GNU */
  CMP ("%m", "01");
  CMP ("%n", "\n");		/* POSIX.2 */
  CMP ("%p", "PM");
  CMP ("%r", "01:06:07 PM");	/* POSIX.2 */
  CMP ("%s", "738367");		/* GNU */
  CMP ("%t", "\t");		/* POSIX.2 */
  CMP ("%u", "5");		/* POSIX.2 */
  CMP ("%w", "5");
  CMP ("%x", "01/09/70");
  CMP ("%y", "70");
  CMP ("%z", "+0000");		/* GNU */

  /*  Check GNU flag #. Inverts the case.  */
  CMP ("%#A", "FRIDAY");
  CMP ("%#^A", "FRIDAY");
  CMP ("%^#A", "FRIDAY");
  CMP ("%#a", "FRI");
  CMP ("%#^a", "FRI");
  CMP ("%^#a", "FRI");
  CMP ("%#B", "JANUARY");
  CMP ("%#^B", "JANUARY");
  CMP ("%^#B", "JANUARY");
  CMP ("%#b", "JAN");
  CMP ("%#^b", "JAN");
  CMP ("%^#b", "JAN");
  CMP ("%p", "PM");
  CMP ("%#p", "pm");
  CMP ("%#Z", "gmt");

  /*  Check E and O mofifier. Ignore it.  */
  CMP ("%EC", "19");
  CMP ("%Ec", "Fri Jan  9 13:06:07 1970");
  CMP ("%EX", "13:06:07");
  CMP ("%Ex", "01/09/70");
  CMP ("%EY", "1970");
  CMP ("%Ey", "70");
  CMP ("%Od", "09");
  CMP ("%Oe", " 9");
  CMP ("%OH", "13");
  CMP ("%OI", "01");
  CMP ("%OM", "06");
  CMP ("%Om", "01");
  CMP ("%OS", "07");
  CMP ("%Ou", "5");
  CMP ("%OU", "01");
  CMP ("%OV", "02");
  CMP ("%OW", "01");
  CMP ("%Ow", "5");
  CMP ("%Oy", "70");

  /*  Check G, g and V specifiers.
      The first two examples are from:
        <http://www.opengroup.org/onlinepubs/000095399/functions/strftime.html>
  */
  t = 883441421;  /*  Tue Dec 30 0:23:41 1997.  */
  tm = gmtime (&t);
  CMP ("%V", "01");
  CMP ("%W", "52");
  CMP ("%G", "1998");
  CMP ("%Y", "1997");
  CMP ("%g", "98");
  CMP ("%y", "97");

  t = 915245172;  /*  Sat Jan  2 02:46:12 1999.  */
  tm = gmtime (&t);
  CMP ("%V", "53");
  CMP ("%W", "00");
  CMP ("%G", "1998");
  CMP ("%Y", "1999");
  CMP ("%g", "98");
  CMP ("%y", "99");

  t = 1212808584;  /*  Sat Jun  7 03:16:24 2008.  */
  tm = gmtime (&t);
  CMP ("%V", "23");
  CMP ("%W", "22");
  CMP ("%G", "2008");
  CMP ("%Y", "2008");
  CMP ("%g", "08");
  CMP ("%y", "08");

  exit (n_fail ? 1 : 0);
}
