#include <stdio.h>
#ifdef __GO32__
#include <unistd.h>
#endif
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

int
main(int argc, char **argv)
{
  int f;
  unsigned char buf[1000];
  int r, i;

  if (argc < 2)
  {
    printf("usage: tread file\n");
    return 1;
  }

  printf("Binary:\n");
  f = _open(argv[1], O_RDONLY);
  while ((r=_read(f, buf, 1000)) > 0)
  {
    for (i=0; i<r; i++)
      printf(" %02x", buf[i]);
    printf(" - %d\n", lseek(f, 0L, 1));
  }
  _close(f);

  printf("Text:\n");
  f = open(argv[1], O_RDONLY|O_TEXT);
  while ((r=read(f, buf, 10)) > 0)
  {
    printf(" [%d]", r);
    for (i=0; i<r; i++)
      printf(" %02x", buf[i]);
    printf(" <%d>", lseek(f, 0L, 1));
  }
  printf("\n");
  close(f);

  return 0;
}
