#include <stdlib.h>
#include <stdio.h>
#include <dos.h>

void
check(int ex)
{
  int rv;
  asm("pushf; popl %0" : "=g" (rv));
  rv = (rv>>9) & 1;
  printf("-> %d(=%d?)\n", rv, ex);
  if (rv != ex)
  {
    if (ex)
      printf("interrupts not actually enabled\n");
    else
      printf("interrupts not actually disabled\n");
  }
}

void
try(int i, const char *s, int ex, int ex2)
{
  printf("%d(=%d?) -> %s ", i, ex, s);
  if (ex != -1 && i != ex)
  {
    printf("-> incorrect; expected %d\n", ex);
    exit(1);
  }
  check(ex2);
}

int
main(void)
{
  enable();
  try(enable(),  "enable ",  1, 1);
  try(disable(), "disable", 1, 0);
  try(disable(), "disable", 0, 0);
  try(enable(),  "enable ",  0, 1);
  try(enable(),  "enable ",  1, 1);
  try(disable(), "disable", 1, 0);
  try(enable(),  "enable ",  0, 1);
  return 0;
}
