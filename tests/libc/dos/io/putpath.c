#include <libc/dosio.h>
#include <libc/farptrgs.h>
#include <stdio.h>
#include <go32.h>

void test(const char *p)
{
  int o = __tb, i, c;
  char buf[1024];
  _put_path(p);
  for (i = 0; i < 1023 && ((c = _farpeekb(_dos_ds,o++)) != 0); i++)
    buf[i] = c;
  buf[i] = 0;
  printf("%-30s %s\n", p, buf);
}

int main(void)
{
  test("c:");
  test("c:/");
  test("c://");
  test("c://");
  test("\\\\NET\\\\CDROM\\");
  test("//NET/CDROM/");
  test("c:/dev/djgpp");
  test("/dev/djgpp");
  test("/dev/null");
  test("/dev/tty");
  test("/dev/zero");
  test("/dev/null/");
  test("/dev/c");
  test("/dev/C");
  test("/dev/-");
  test("/dev/c/");
  test("/dev/c//");
  test("/dev/c/dos/");
  return 0;
}

