/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>
#include <float.h>

char *
fcvt (double value, int ndigits, int *decpt, int *sign)
{
  static char fcvt_buf[2 * DBL_MAX_10_EXP + 10];
  return fcvtbuf (value, ndigits, decpt, sign, fcvt_buf);
}
