#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

int
main(void)
{
  time_t t;

  time(&t);
  printf("start: %s", ctime(&t));

  sleep(10);

  time(&t);
  printf("end  : %s", ctime(&t));

  return 0;
}
