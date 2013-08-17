#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <libc/unconst.h>

#define buflen 80

void timetest(int *fail, time_t t, const char er[buflen])
{
  char or[buflen];

  if (!strftime (or, buflen, "%Y-%m-%d %H:%M:%S %Z", localtime(&t)))
  {
    fprintf(stderr, "strftime failed   FAIL\n");
    exit(-1);
  }

  printf("time_t %ld  expected %s\n", (long)t, er);
  printf("                        got %s", or);
  if (strcmp(er, or)) *fail = 1, printf("   FAIL");
  printf ("\n");
}

int main (int argc, char **argv)
{
  int fail = 0;
  static const char *env_string[] = {"TZ=:America/New_York", "TZ=:Europe/Berlin", "TZ=:America/Buenos_Aires"};
  char *tz;

  if (tz = unconst(env_string[0], char *), putenv(tz))
  {
    fprintf(stderr, "putenv failed   FAIL\n");
    exit(-1);
  }
  tzset();

  timetest(&fail, 1357016400, "2013-01-01 00:00:00 EST");
  timetest(&fail, 1362898799, "2013-03-10 01:59:59 EST");
  timetest(&fail, 1362898800, "2013-03-10 03:00:00 EDT");
  timetest(&fail, 1372780800, "2013-07-02 12:00:00 EDT");
  timetest(&fail, 1383454800, "2013-11-03 01:00:00 EDT");
  timetest(&fail, 1383458400, "2013-11-03 01:00:00 EST");
  timetest(&fail, 1388552399, "2013-12-31 23:59:59 EST");
  printf("Test #1 %s\n", (fail ? "FAILED" : "passed"));


  printf("\n\n\n");


  if (tz = unconst(env_string[1], char *), putenv(tz))
  {
    fprintf(stderr, "putenv failed   FAIL\n");
    exit(-1);
  }
  tzset();

  timetest(&fail, 1356998461, "2013-01-01 01:01:01 CET");
  timetest(&fail, 1359766922, "2013-02-02 02:02:02 CET");
  timetest(&fail, 1370491566, "2013-06-06 06:06:06 CEST");
  timetest(&fail, 1376701904, "2013-08-17 03:11:44 CEST");
  timetest(&fail, 1379401749, "2013-09-17 09:09:09 CEST");
  timetest(&fail, 1384164692, "2013-11-11 11:11:32 CET");
  timetest(&fail, 1388530799, "2013-12-31 23:59:59 CET");
  printf("Test #2 %s\n", (fail ? "FAILED" : "passed"));


  printf("\n\n\n");


  if (tz = unconst(env_string[2], char *), putenv(tz))
  {
    fprintf(stderr, "putenv failed   FAIL\n");
    exit(-1);
  }
  tzset();

  timetest(&fail, 1357012861, "2013-01-01 01:01:01 ART");
  timetest(&fail, 1359781322, "2013-02-02 02:02:02 ART");
  timetest(&fail, 1370509566, "2013-06-06 06:06:06 ART");
  timetest(&fail, 1376738799, "2013-08-17 08:26:39 ART");
  timetest(&fail, 1379417277, "2013-09-17 08:27:57 ART");
  timetest(&fail, 1384179071, "2013-11-11 11:11:11 ART");
  timetest(&fail, 1388545199, "2013-12-31 23:59:59 ART");
  printf("Test #3 %s\n", (fail ? "FAILED" : "passed"));

  return 0;
}
