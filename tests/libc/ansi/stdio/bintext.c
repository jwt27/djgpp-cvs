#include <stdio.h>
#include <io.h>
#include <fcntl.h>

int
main(void)
{
  FILE *f;

  f = fopen("/tmp/binary.dat", "wb");
  fprintf(f, "hello\nthere\n");
  fclose(f);

  f = fopen("/tmp/text.dat", "wt");
  fprintf(f, "hello\nthere\n");
  fclose(f);

  fflush(stdout);
  setmode(1, O_BINARY);
  printf("stdout in binary mode\nbinary\nbinary\n");
  fflush(stdout);
  setmode(1, O_TEXT);
  printf("stdout in text mode\ntext\ntext\n");
  fflush(stdout);
  setmode(1, O_BINARY);
  printf("stdout in binary mode\nbinary\nbinary\n");
  fflush(stdout);
  setmode(1, O_TEXT);
  printf("stdout in text mode\ntext\ntext\n");

  return 0;
}
