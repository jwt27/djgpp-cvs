#include <stdio.h>
#include <stdlib.h>
#include <sys/fsext.h>
#include <unistd.h>

int
functions(__FSEXT_Fnumber func_number, int *rv, va_list args)
{
 printf ("Function: %d\n", func_number );

 return 8;
}

int
main(void)
{
  int s1, s2;
  char x[3];

  s1 = __FSEXT_alloc_fd ( functions );
  s2 = __FSEXT_alloc_fd ( functions );

  printf ("Reading 1:\n");
  read ( s1, x, 3 );
  printf ("Reading 2:\n");
  read ( s2, x, 3 );
  printf ("Writing 1:\n");
  write ( s1, "Hey", 3 );
  printf ("Writing 2:\n");
  write ( s2, "Hey", 3 );

  return 0;
}
