#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pc.h>

int
main(int argc, char **argv)
{
  int i;
  char buf[10], buf2[10];
  FILE *f;
  time_t start_time, end_time;
  int count=0;

  f = fopen("fseek2.tm~", "wb");
  for (i=0; i<65536; i++)
  {
    sprintf(buf, "%06x\r\n", i);
    fwrite(buf, 1, 8, f);
  }
  fclose(f);
  f = fopen("fseek2.tm~", "rb");

  srandom(time(0));
  time(&start_time);
  while (!kbhit())
  {
    i = random() % 65536;
    sprintf(buf, "%06x\r\n", i);
    fseek(f, i*8, SEEK_SET);
    fread(buf2, 1, 8, f);
    if (memcmp(buf, buf2, 8))
    {
      printf("Compare failed at %d, %.6s vs %.6s\n", i, buf, buf2);
      break;
    }
    if (ftell(f) != i*8+8)
    {
      printf("ftell failed at %d, %lx vs %x\n", i, ftell(f), i*8+8);
      break;
    }
    count++;
  }
  time(&end_time);
  if (end_time != start_time)
    printf("Rate: %d per sec\n", count/(end_time-start_time));
	
  return 0;
}
