#include <libc/dosio.h>
#include <go32.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
  int i;
  for (i=0; i<argc; i++)
  {
    char buf[200];
    _put_path(argv[i]);
    dosmemget(__tb, 200, buf);
    printf("`%s' -> `%s'\n", argv[i], buf);
  }
  return 0;
}
