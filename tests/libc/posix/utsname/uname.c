#include <stdio.h>
#include <sys/utsname.h>

int
main(void)
{
  struct utsname n;
  uname(&n);
  printf("release=%s   version=%s\n", n.release, n.version);
  return 0;
}
