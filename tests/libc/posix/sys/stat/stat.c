#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

void
djstat(char *f)
{
  struct stat s;
  if (stat(f, &s) < 0)
    printf("stat(%s) failed\n", f);
  printf("%2d.%-5d %10d %3d %24.24s %s\n", s.st_dev, s.st_ino, s.st_size,
	 s.st_nlink,
	 asctime(localtime(&s.st_mtime)), f);
}

int
main(int argc, char **argv)
{
  int i;
  for (i=1; i<argc; i++)
    djstat(argv[i]);
  return 0;
}
