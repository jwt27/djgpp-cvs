#include <dirent.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
  DIR *d;
  struct dirent *de;
  printf("opend(%s)\n", argv[1]);
  d = opendir(argv[1]);
  if (d == 0)
  {
    perror(argv[1]);
    return 1;
  }
  while ((de=readdir(d)))
    printf("%s\n", de->d_name);
  closedir(d);
  return 0;
}
