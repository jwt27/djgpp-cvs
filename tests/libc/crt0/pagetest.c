#include <stdio.h>
#include <stdlib.h>
#include <pc.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
  unsigned long pmax, p, cs, i;
  unsigned long *pp, *pbase;

  if (argc < 2)
  {
    printf("pagetest {kb}\n");
    return 1;
  }

  pmax = atoi(argv[1]) / 4;
  printf("%ld pages\n", pmax);

  pbase = (unsigned long *)sbrk(pmax*4096);
  if ((int)pbase == -1)
  {
    fprintf(stderr, "sbrk failed\n");
    exit(1);
  }
  for (p=0; p<pmax; p++)
  {
    printf("\rinit page %ld  ", p);
    fflush(stdout);
    pp = pbase + p*1024;
    cs = 0;
    for (i=0; i<1023; i++)
    {
      pp[i] = random();
      cs += pp[i];
    }
    pp[i] = cs;
  }
  printf("\r                         \r");
  while (!kbhit())
  {
    p = random() % pmax;
    printf("\rpage %ld  ", p);
    fflush(stdout);
    pp = pbase + p*1024;
    cs = 0;
    for (i=0; i<1023; i++)
      cs += pp[i];
    if (pp[i] != cs)
      printf("Bad cs!\n");
  }
  return 0;
}
