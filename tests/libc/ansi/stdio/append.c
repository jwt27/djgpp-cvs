#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FILE_NAME "append.dat"

int
main(void)
{
  FILE *f;
  int status = 0; /* Return value. */
  struct stat s;
  off_t len;

  f = fopen(FILE_NAME, "w");
  fprintf(f, "hello, there\n");
  fclose(f);
  stat(FILE_NAME, &s);
  len = s.st_size;

  f = fopen(FILE_NAME, "a");
  fprintf(f, "hello, there\n");
  fclose(f);
  stat(FILE_NAME, &s);
  if (s.st_size != len * 2)
  {
    printf("wrong size 1!\n");
    status++;
  }

  f = fopen(FILE_NAME, "a+");
  fseek(f, 0, SEEK_SET);
  fprintf(f, "hello, there\n");
  fclose(f);
  stat(FILE_NAME, &s);
  if (s.st_size != len * 3)
  {
    printf("wrong size 2!\n");
    status++;
  }

  return status;
}
