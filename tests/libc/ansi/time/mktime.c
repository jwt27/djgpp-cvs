#include <stdio.h>
#include <string.h>
#include <time.h>

void
try(int x)
{
  struct tm tm;
  time_t t, d;
  static time_t prev=0;

  memset(&tm, 0, sizeof(tm));
  tm.tm_sec = 0;
  tm.tm_min = (x % 4) * 15;
  tm.tm_hour = (x/4) % 24;
  tm.tm_mday = 6;
  tm.tm_mon = 3;
  tm.tm_year = 97;
  tm.tm_isdst = -1;

  printf("%04d/%02d/%02d %02d:%02d -> ",
	 tm.tm_year, tm.tm_mon, tm.tm_mday,
	 tm.tm_hour, tm.tm_min);

  t = mktime(&tm);

  if (prev == 0) prev = t-900; /* "normal" */
  d = t - prev;
  prev = t;

  printf("%04d/%02d/%02d %02d:%02d -> %10d %10d\n",
	 tm.tm_year, tm.tm_mon, tm.tm_mday,
	 tm.tm_hour, tm.tm_min, t, d);

}

int
main(void)
{
  int x;
  for (x=0; x < 6*4; x++)
    try(x);
  return 0;
}
