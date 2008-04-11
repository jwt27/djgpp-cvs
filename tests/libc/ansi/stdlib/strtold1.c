#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

static const char *test_string[] = {
  /*  Integers.  */
  "0x1 ###",
  "0x2 ###",
  "0xF ###",
  "0x0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef ###",
  /*  Zeros.  */
  "0x0 ###",
  "0x0. ###",
  "0x.0 ###",
  "0x000.000 ###",
  "0x0.0P-00###",
  "0x0.0P+00###",
  "0x0.P-00###",
  "0x.0P-00###",
  "0X00000.0000000000000000000000000000000000000000000000000000000000FED0123P+123###",
  "0x0.0P+9999999999999999###",
  /*  Floating points.  */
  "0x1. ###",
  "0x1.P00###",
  "0x1P+0###",
  "0x.1P-0###",
  "0x12345678.9ABCDEF1p+12345 ###",
  "0xF.EDcba ###",
  "0x123456789abcdef1.fedcba987654321 ###",
  "0x123456789abcdeffedcba987654321.fedcba987654321 ###",
  /*  Overflow.  */
  "0x1.0P+9999999999999999###",
  "0x7.8P+123456789###",
  "0X00000.0000000000000000000000000000000000000000000000000000000000FED0123P+987654321###",
  /*  Underflow.  */
  "0x1.0P-9999999999999999###",
  "0x7.8P-123456789###",
  "0X00000.0000000000000000000000000000000000000000000000000000000000FED0123P-987654321###",
  /*  Errors.  */
  "0x 123 ###",
  "0x123. 123 ###",
  "0x123.123 p123 ###",
  "0x123.123p 123 ###",
  0
};

int main(void)
{
  char *endp;
  long double result;
  int i;

  printf("Testing different hex floating point strings to be converted.\n"
         "-------------------------------------------------------------\n");
  for (i = 0; test_string[i]; i++)
  {
    errno = 0;
    result = strtold(test_string[i], &endp);
    switch (errno)
    {
    case 0:
      printf("test string: <%s>\tendp: <%s>\tvalue: %Le\n", test_string[i], endp, result);
      break;
    case ERANGE:
      printf("test string: <%s>\tendp: <%s>\tvalue: %Le  ERANGE\n", test_string[i], endp, result);
      break;
    case EINVAL:
      printf("test string: <%s>\tendp: <%s>\tvalue: %Le  EINVAL\n", test_string[i], endp, result);
      break;
    }
  }

  /*
   *  Tests taken from different GNU configure scripts.
   */
  printf("Running certain GNU configure script tests to detect certain misfeatures.\n"
         "-------------------------------------------------------------------------\n");
  {
    /* Some versions of Linux strtold mis-parse strings with leading '+'.  */
    const char *string = " +69";
    char *term;
    long double value;
    value = strtold (string, &term);
    if (value != 69 || term != (string + 4))
      printf("strtold: test #1 failed.\tstr: <%s>  endp: <%s>  value: %Lg\n", string, term, value);
    else
      printf("strtold: test #1 OK.\tstr: <%s>  endp: <%s>  value: %Lg\n", string, term, value);
  }

  {
    /* Under Solaris 2.4, strtold returns the wrong value for the
       terminating character under some conditions.  */
    const char *string = "NaN";
    char *term;
    strtold (string, &term);
    if (term != string && *(term - 1) == 0)
      printf("strtold: test #2 failed.\tstr: <%s>  endp: <%s>\n", string, term);
    else
      printf("strtold: test #2 OK.\tstr: <%s>  endp: <%s>\n", string, term);
  }

  {
    /* Older glibc and Cygwin mis-parse "-0x".  */
    const char *string = "-0x";
    char *term;
    long double value = strtold (string, &term);
    if (1 / value != -HUGE_VALL || term != (string + 2))
      printf("strtold: test #3 failed.\tstr: <%s>  endp: <%s>  value: %Lg\n", string, term, value);
    else
      printf("strtold: test #3 OK.\tstr: <%s>  endp: <%s>  value: %Lg\n", string, term, value);
  }

  {
    /* Many platforms do not parse infinities.  */
    const char *string = "inf";
    char *term;
    long double value = strtold (string, &term);
    if (value != HUGE_VALL || term != (string + 3))
      printf("strtold: test #4 failed.\tstr: <%s>  endp: <%s>  value: %Lg\n", string, term, value);
    else
      printf("strtold: test #4 OK.\tstr: <%s>  endp: <%s>  value: %Lg\n", string, term, value);
  }

  return 0;
}

