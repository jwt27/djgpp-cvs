#include <stdio.h>
#include <stdarg.h>

void x(const char *f, ...)
{
  const char *c;
  va_list a;
  printf("rounded size of f is %ld\n", __dj_va_rounded_size(f));
  va_start(a, f);
  printf("&f = %p\n", &f);
  printf("a = %p\n", a);
  do {
    c = va_arg(a, char *);
    printf("arg = `%s'\n", c);
  } while (c);
  va_end(a);
}

int
main(void)
{
  x("abc", "def", 0);
  x("abc", "def", "ghi", "jkl", 0);
  return 0;
}
