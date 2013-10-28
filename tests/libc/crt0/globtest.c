#include <stdio.h>
#include <fnmatch.h>
#include <glob.h>

int
main(int argc, char **argv)
{
  unsigned i;
  glob_t g;
  printf("Pattern: `%s'\n", argv[1]);
  glob(argv[1], 0, 0, &g);
  for (i=0; i<g.gl_pathc; i++)
    printf("-> `%s'\n", g.gl_pathv[i]);
  return 0;
}
