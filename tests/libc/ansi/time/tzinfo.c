#include <stdio.h>
#include <time.h>
#include <stdlib.h>

const char *weekday[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

void
print(const char *msg, struct tm *t)
{
  printf("%s %2d:%02d %s\n", msg, t->tm_hour, t->tm_min, weekday[t->tm_wday]);
}

int
main(void)
{
  const char *d;
  time_t now;
  struct tm l, g;

  printf("TZ = %s\n", getenv("TZ") ? : "(unset)");

  time(&now);
  l = *localtime(&now);
  g = *gmtime(&now);

  print("Local time:  ", &l);
  print("GMT time:    ", &g);

  d = "East";
  if (l.tm_gmtoff < 0)
  {
    l.tm_gmtoff = -l.tm_gmtoff;
    d = "West";
  }

  printf("%s of GMT by %d:%02d (%s)\n",
	 d,
	 l.tm_gmtoff / 3600,
	 (l.tm_gmtoff / 60) % 60,
	 l.tm_zone);

  return 0;
}
