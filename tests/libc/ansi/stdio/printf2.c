#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int
main (void)
{
  signed char   sc = SCHAR_MAX;
  unsigned char uc = UCHAR_MAX;

  /* Normal operation */
  printf("%hhd %hhi %hhu 0x%hhx 0x%hhX\n",
	 sc, sc, uc, uc, uc);

  /* Try truncating some ints */
  printf("%hhd %hhi\n", 128, 128);

  printf("%hhd %hhi %hhu 0x%hhx 0x%hhX\n",
	 INT_MAX, INT_MAX, UINT_MAX, UINT_MAX, UINT_MAX);

  return(EXIT_SUCCESS);
}
