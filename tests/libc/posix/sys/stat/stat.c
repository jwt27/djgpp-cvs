#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

void
djstat(char *f)
{
  struct stat s;
  stat(f, &s);
  printf("%d.%d %10d %24.24s %s\n", s.st_dev, s.st_ino, s.st_size, asctime(localtime(&s.st_mtime)), f);
}

int
main(int argc, char **argv)
{
  int i;
  for (i=1; i<argc; i++)
    djstat(argv[i]);
  return 0;
}
