#include <libc/ieee.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>


int main(void)
{
  _double_union_t x, y;
  double result;

  y.d = M_PI;  /*
                *  An arbitrary finite value.  It must have a mantissa's
                *  fractional part different from 0 in the lower 32 bit
                *  or the bug will not be triggered.
                */
  x.dt.sign      = 0;
  x.dt.exponent  = 0x07FF;
  x.dt.mantissah = 0x000000000;
  x.dt.mantissal = 0x000000000;  /*  +Infinity.  */

  /*
   *  Compute atan of the quotient of a positive finite number with pos. infinity.
   *  The result must be +0.0.  The brocken version of atan2 returns errno = EDOM
   *  and as result NaN.
   */
  errno = 0;
  result = atan2(y.d, x.d);
  if (errno)
    printf("errno=%d\ny=%f  x=%f:  result=%f  result must be +0.0\n", errno, y.d, x.d, result);
  else if (result == +0.0)
    printf("errno=%d\ny=%f  x=%f:  result=%f\nTest passed.\n\n", errno, y.d, x.d, result);
  else
    printf("test failure not triggered by tested atan2 bug.\n");


  /*
   *  Compute atan of the quotient of a negative finite number with neg. infinity.
   *  The result must be -PI.  The brocken version of atan2 returns errno = EDOM
   *  and as result NaN.
   */
  errno = 0;
  y.dt.sign = 1;  /*  -Finite value.  */
  x.dt.sign = 1;  /*  -Infinity.  */
  result = atan2(y.d, x.d);
  if (errno)
    printf("errno=%d\ny=%f  x=%f:  result=%f  result must be -PI\n", errno, y.d, x.d, result);
  else if (result == -M_PI)
    printf("errno=%d\ny=%f  x=%f:  result=%f\nTest passed.", errno, y.d, x.d, result);
  else
    printf("test failure not triggered by tested atan2 bug.\n");

  return 0;
}
