#include <stdio.h>
#include <string.h>
#include <time.h>

extern char __dj_date_format[10];
extern char __dj_time_format[16];

int main(int ac, char *av[])
{
  char buf[99];
  time_t t = time(NULL);

  strcpy(__dj_date_format, "%d|%m|%Y");
  strcpy(__dj_time_format, "[%H|%M|%S]");

  strftime(buf, sizeof(buf), "%x %X", gmtime(&t));
  printf("%s\n", buf);
  return 0;
}
