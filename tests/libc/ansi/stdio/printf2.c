/*
 * printf2.c
 * Test cases for conversion specifiers new to the ANSI C99 standard.
 */

#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int
main (void)
{
  /* --- char --- */

  /* Normal operation */
  printf("%hhd %hhi %hhu 0x%hhx 0x%hhX\n",
	 SCHAR_MAX, SCHAR_MAX, SCHAR_MAX, SCHAR_MAX, SCHAR_MAX);

  printf("%hhd %hhi %hhu 0x%hhx 0x%hhX\n",
	 UCHAR_MAX, UCHAR_MAX, UCHAR_MAX, UCHAR_MAX, UCHAR_MAX);

  /* Try truncating some ints */
  printf("%hhd %hhi\n", 128, 128);

  printf("%hhd %hhi 0%hho %hhu 0x%hhx 0x%hhX\n",
	 INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX);

  printf("%hhd %hhi 0%hho %hhu 0x%hhx 0x%hhX\n",
	 UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX);

  /* --- *intmax_t --- */

  printf("%jd %ji 0%jo %ju 0x%jx 0x%jX\n",
	 INTMAX_MAX, INTMAX_MAX, INTMAX_MAX,
	 INTMAX_MAX, INTMAX_MAX, INTMAX_MAX);

  printf("%jd %ji 0%jo %ju 0x%jx 0x%jX\n",
	 UINTMAX_MAX, UINTMAX_MAX, UINTMAX_MAX,
	 UINTMAX_MAX, UINTMAX_MAX, UINTMAX_MAX);

  /* --- ptrdiff_t --- */

  printf("%td %ti 0%to %tu 0x%tx 0x%tX\n",
	 INT_MAX, INT_MAX, INT_MAX,
	 INT_MAX, INT_MAX, INT_MAX);

  printf("%td %ti 0%to %tu 0x%tx 0x%tX\n",
	 UINT_MAX, UINT_MAX, UINT_MAX,
	 UINT_MAX, UINT_MAX, UINT_MAX);

  /* --- size_t --- */

  size_t  ssize_max = SSIZE_MAX;
  size_t  size_max  = SIZE_MAX;

  printf("%zd %zi 0%zo %zu 0x%zx 0x%zX\n",
	 ssize_max, ssize_max, ssize_max,
	 ssize_max, ssize_max, ssize_max);

  printf("%zd %zi 0%zo %zu 0x%zx 0x%zX\n",
	 size_max, size_max, size_max,
	 size_max, size_max, size_max);

  return(EXIT_SUCCESS);
}
