#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int
main(int argc, char **argv)
{
  int i;
  struct stat s;
  for (i=0; i<20; i++)
  {
    int fd = open("nul", O_RDONLY);
    close(fd);
    printf("loop %d fd %d\n", i, fd);

    stat(argc > 1 ? argv[1] : "leak.c", &s);
  }
  return 0;
}
