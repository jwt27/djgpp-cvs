#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

int
main(void)
{
  FILE *f;
  struct stat s;
  size_t len;

  f = fopen("append.dat", "w");
  fprintf(f, "hello, there\n");
  fclose(f);
  stat("append.dat", &s);
  len = s.st_size;

  f = fopen("append.dat", "a");
  fprintf(f, "hello, there\n");
  fclose(f);
  stat("append.dat", &s);
  if (s.st_size != len * 2)
  {
    printf("wrong size!\n");
  }

  return 0;
}
