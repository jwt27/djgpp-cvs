#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


int
main(void)
{
  char *s;
  const char *s0[] = {
    "/dev/env/TMPDIR/foobarXXXXXX",
    "/dev/env/TMPDIR/foobarXXXXXX",
    "/dev/env/TMPDIR/foobarXXXXX",
    "/dev/env/TMPDIR/foobarXXXXXY",
    "/dev/env/TMPDIR/foo/barXXXXXX",
    "/dev/env/TMPDIR/XXXXXX",
    "/dev/env/TMPDIR/XXXXXXXX"
  };
  unsigned int i;


  for (i = 0; i < sizeof s0 / sizeof s0[0]; i++)
  {
    s = strdup(s0[i]);
    errno = 0;
    mkdtemp(s);
    printf("%s -> %s  errno = %d\n", s0[i], s, errno);
  }

  return 0;
}
