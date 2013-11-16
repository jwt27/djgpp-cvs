/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */


#include "main-t.h"

int isspecialf_test(void)
{
  _float_union_t ieee754;
  int i, rv, failures = 0;
  unsigned long int mask;


  /*  Check infinity.  */
  ieee754.ft.sign = 0;
  ieee754.ft.exponent = 0xFF;
  ieee754.ft.mantissa = 0x00000000;

  rv = isinff(ieee754.f);
  if (rv != 1)
  {
    printf("isinff failed: returned = %d  should be = 1\n", rv);
    failures++;
  }
  ieee754.ft.exponent = 0xFE;
  rv = isinff(ieee754.f);
  if (rv != 0)
  {
    printf("isinff failed: returned = %d  should be = 0\n", rv);
    failures++;
  }


  /*  Check SNaN.  */
  ieee754.ft.sign = 0;
  ieee754.ft.exponent = 0xFF;
  ieee754.ft.mantissa = 0x00000001;

  for (mask = 1, i = 0; i < 21; i++, mask <<= 1, ieee754.ft.sign = 0, ieee754.ft.exponent = 0xFF, ieee754.ft.mantissa |= mask)
  {
    rv = isnanf(ieee754.f);
    if (rv != 1)
    {
      printf("isnanf failed: returned = %d  should be = 1\n", rv);
      failures++;
    }
    ieee754.ft.exponent = 0xFE;
    rv = isnanf(ieee754.f);
    if (rv != 0)
    {
      printf("isnanf failed: returned = %d  should be = 0\n", rv);
      failures++;
    }
  }


  /*  Check QNaN.  */
  ieee754.ft.sign = 0;
  ieee754.ft.exponent = 0xFF;
  ieee754.ft.mantissa = 0x00100000;

  for (mask = 0x00100000, i = 0; i < 21; i++, mask >>= 1, ieee754.ft.sign = 0, ieee754.ft.exponent = 0xFF, ieee754.ft.mantissa |= mask)
  {
    rv = isnanf(ieee754.f);
    if (rv != 1)
    {
      printf("isnanf failed: returned = %d  should be = 1\n", rv);
      failures++;
    }
    ieee754.ft.exponent = 0xFE;
    rv = isnanf(ieee754.f);
    if (rv != 0)
    {
      printf("isnanf failed: returned = %d  should be = 0\n", rv);
      failures++;
    }
  }


  /*  Check finite.  */
  ieee754.ft.sign = 0;
  ieee754.ft.exponent = 0xFF;
  ieee754.ft.mantissa = 0x00000000;

  for (mask = 1, i = 0; i < 22; i++, mask <<= 1, ieee754.ft.sign = 0, ieee754.ft.exponent = 0xFF, ieee754.ft.mantissa |= mask)
  {
    rv = finitef(ieee754.f);
    if (rv != 0)
    {
      printf("finitef failed: returned = %d  should be = 1\n", rv);
      failures++;
    }
    ieee754.ft.exponent = 0xFE;
    rv = finitef(ieee754.f);
    if (rv != 1)
    {
      printf("finitef failed: returned = %d  should be = 0\n", rv);
      failures++;
    }
  }

  printf("%s\n", failures ? "isinff, isnanf or finitef tests failed." : "isinff, isnanf and finitef tests succeded.");


  return failures ? 1 : 0;
}


int isspecial_test(void)
{
  _double_union_t ieee754;
  int i, rv, failures = 0;
  unsigned long int mask;


  /*  Check infinity.  */
  ieee754.dt.sign = 0;
  ieee754.dt.exponent = 0x7FF;
  ieee754.dt.mantissah = 0x00000000;
  ieee754.dt.mantissal = 0x00000000;

  rv = isinf(ieee754.d);
  if (rv != 1)
  {
    printf("isinf failed: returned = %d  should be = 1\n", rv);
    failures++;
  }
  ieee754.dt.exponent = 0x7FE;
  rv = isinf(ieee754.d);
  if (rv != 0)
  {
    printf("isinf failed: returned = %d  should be = 0\n", rv);
    failures++;
  }


  /*  Check SNaN.  */
  ieee754.dt.sign = 0;
  ieee754.dt.exponent = 0x7FF;
  ieee754.dt.mantissah = 0x00000000;
  ieee754.dt.mantissal = 0x00000001;

  for (mask = 1, i = 0; i < 32; i++, mask <<= 1, ieee754.dt.sign = 0, ieee754.dt.exponent = 0x7FF, ieee754.dt.mantissal |= mask)
  {
    rv = isnan(ieee754.d);
    if (rv != 1)
    {
      printf("isnan failed: returned = %d  should be = 1\n", rv);
      failures++;
    }
    ieee754.dt.exponent = 0x7FE;
    rv = isnan(ieee754.d);
    if (rv != 0)
    {
      printf("isnan failed: returned = %d  should be = 0\n", rv);
      failures++;
    }
  }
  for (mask = 1, i = 0; i < 20; i++, mask <<= 1, ieee754.dt.sign = 0, ieee754.dt.exponent = 0x7FF, ieee754.dt.mantissah |= mask)
  {
    rv = isnan(ieee754.d);
    if (rv != 1)
    {
      printf("isnan failed: returned = %d  should be = 1\n", rv);
      failures++;
    }
    ieee754.dt.exponent = 0x7FE;
    rv = isnan(ieee754.d);
    if (rv != 0)
    {
      printf("isnan failed: returned = %d  should be = 0\n", rv);
      failures++;
    }
  }


  /*  Check QNaN.  */
  ieee754.dt.sign = 0;
  ieee754.dt.exponent = 0x7FF;
  ieee754.dt.mantissah = 0x00080000;
  ieee754.dt.mantissal = 0x00000000;

  for (mask = 0x00080000, i = 0; i < 20; i++, mask >>= 1, ieee754.dt.sign = 0, ieee754.dt.exponent = 0x7FF, ieee754.dt.mantissah |= mask)
  {
    rv = isnan(ieee754.d);
    if (rv != 1)
    {
      printf("isnan failed: returned = %d  should be = 1\n", rv);
      failures++;
    }
    ieee754.dt.exponent = 0x7FE;
    rv = isnan(ieee754.d);
    if (rv != 0)
    {
      printf("isnan failed: returned = %d  should be = 0\n", rv);
      failures++;
    }
  }
  for (mask = 0x80000000, i = 0; i < 32; i++, mask >>= 1, ieee754.dt.sign = 0, ieee754.dt.exponent = 0x7FF, ieee754.dt.mantissal |= mask)
  {
    rv = isnan(ieee754.d);
    if (rv != 1)
    {
      printf("isnan failed: returned = %d  should be = 1\n", rv);
      failures++;
    }
    ieee754.dt.exponent = 0x7FE;
    rv = isnan(ieee754.d);
    if (rv != 0)
    {
      printf("isnan failed: returned = %d  should be = 0\n", rv);
      failures++;
    }
  }


  /*  Check finite.  */
  ieee754.dt.sign = 0;
  ieee754.dt.exponent = 0x7FF;
  ieee754.dt.mantissah = 0x00000000;
  ieee754.dt.mantissal = 0x00000000;

  for (mask = 1, i = 0; i < 32; i++, mask <<= 1, ieee754.dt.sign = 0, ieee754.dt.exponent = 0x7FF, ieee754.dt.mantissal |= mask)
  {
    rv = finite(ieee754.d);
    if (rv != 0)
    {
      printf("finite failed: returned = %d  should be = 1\n", rv);
      failures++;
    }
    ieee754.dt.exponent = 0x7FE;
    rv = finite(ieee754.d);
    if (rv != 1)
    {
      printf("finite failed: returned = %d  should be = 0\n", rv);
      failures++;
    }
  }
  for (mask = 1, i = 0; i < 20; i++, mask <<= 1, ieee754.dt.sign = 0, ieee754.dt.exponent = 0x7FF, ieee754.dt.mantissah |= mask)
  {
    rv = finite(ieee754.d);
    if (rv != 0)
    {
      printf("finite failed: returned = %d  should be = 1\n", rv);
      failures++;
    }
    ieee754.dt.exponent = 0x7FE;
    rv = finite(ieee754.d);
    if (rv != 1)
    {
      printf("finite failed: returned = %d  should be = 0\n", rv);
      failures++;
    }
  }

  printf("%s\n", failures ? "isinf, isnan or finite tests failed." : "isinf, isnan and finite tests succeded.");


  return failures ? 1 : 0;
}


int isspeciall_test(void)
{
  _longdouble_union_t ieee754;
  int i, rv, failures = 0;
  unsigned long int mask;


  /*  Check infinity.  */
  ieee754.ldt.sign = 0;
  ieee754.ldt.exponent = 0x7FFF;
  ieee754.ldt.mantissah = 0x80000000;
  ieee754.ldt.mantissal = 0x00000000;

  rv = isinfl(ieee754.ld);
  if (rv != 1)
  {
    printf("isinfl failed: returned = %d  should be = 1\n", rv);
    failures++;
  }
  ieee754.ldt.exponent = 0x7FFE;
  rv = isinfl(ieee754.ld);
  if (rv != 0)
  {
    printf("isinfl failed: returned = %d  should be = 0\n", rv);
    failures++;
  }


  /*  Check SNaN.  */
  ieee754.ldt.sign = 0;
  ieee754.ldt.exponent = 0x7FFF;
  ieee754.ldt.mantissah = 0x80000000;
  ieee754.ldt.mantissal = 0x00000001;

  for (mask = 1, i = 0; i < 32; i++, mask <<= 1, ieee754.ldt.sign = 0, ieee754.ldt.exponent = 0x7FFF, ieee754.ldt.mantissal |= mask)
  {
    rv = isnanl(ieee754.ld);
    if (rv != 1)
    {
      printf("isnanl failed: returned = %d  should be = 1\n", rv);
      failures++;
    }
    ieee754.ldt.exponent = 0x7FFE;
    rv = isnanl(ieee754.ld);
    if (rv != 0)
    {
      printf("isnanl failed: returned = %d  should be = 0\n", rv);
      failures++;
    }
  }
  for (mask = 1, i = 0; i < 30; i++, mask <<= 1, ieee754.ldt.sign = 0, ieee754.ldt.exponent = 0x7FFF, ieee754.ldt.mantissah |= mask)
  {
    rv = isnanl(ieee754.ld);
    if (rv != 1)
    {
      printf("isnanl failed: returned = %d  should be = 1\n", rv);
      failures++;
    }
    ieee754.ldt.exponent = 0x7FFE;
    rv = isnanl(ieee754.ld);
    if (rv != 0)
    {
      printf("isnanl failed: returned = %d  should be = 0\n", rv);
      failures++;
    }
  }


  /*  Check QNaN.  */
  ieee754.ldt.sign = 0;
  ieee754.ldt.exponent = 0x7FFF;
  ieee754.ldt.mantissah = 0xA0000000;
  ieee754.ldt.mantissal = 0x00000000;

  for (mask = 0x20000000, i = 0; i < 31; i++, mask >>= 1, ieee754.ldt.sign = 0, ieee754.ldt.exponent = 0x7FFF, ieee754.ldt.mantissah |= mask)
  {
    rv = isnanl(ieee754.ld);
    if (rv != 1)
    {
      printf("isnanl failed: returned = %d  should be = 1\n", rv);
      failures++;
    }
    ieee754.ldt.exponent = 0x7FFE;
    rv = isnanl(ieee754.ld);
    if (rv != 0)
    {
      printf("isnanl failed: returned = %d  should be = 0\n", rv);
      failures++;
    }
  }
  for (mask = 0x80000000, i = 0; i < 32; i++, mask <<= 1, ieee754.ldt.sign = 0, ieee754.ldt.exponent = 0x7FFF, ieee754.ldt.mantissal |= mask)
  {
    rv = isnanl(ieee754.ld);
    if (rv != 1)
    {
      printf("isnanl failed: returned = %d  should be = 1\n", rv);
      failures++;
    }
    ieee754.ldt.exponent = 0x7FFE;
    rv = isnanl(ieee754.ld);
    if (rv != 0)
    {
      printf("isnanl failed: returned = %d  should be = 0\n", rv);
      failures++;
    }
  }


  /*  Check finite.  */
  ieee754.ldt.sign = 0;
  ieee754.ldt.exponent = 0x7FFF;
  ieee754.ldt.mantissah = 0x80000000;
  ieee754.ldt.mantissal = 0x00000000;

  for (mask = 1, i = 0; i < 32; i++, mask <<= 1, ieee754.ldt.sign = 0, ieee754.ldt.exponent = 0x7FFF, ieee754.ldt.mantissal |= mask)
  {
    rv = finitel(ieee754.ld);
    if (rv != 0)
    {
      printf("finitel failed: returned = %d  should be = 1\n", rv);
      failures++;
    }
    ieee754.ldt.exponent = 0x7FFE;
    rv = finitel(ieee754.ld);
    if (rv != 1)
    {
      printf("finitel failed: returned = %d  should be = 0\n", rv);
      failures++;
    }
  }
  for (mask = 1, i = 0; i < 31; i++, mask <<= 1, ieee754.ldt.sign = 0, ieee754.ldt.exponent = 0x7FFF, ieee754.ldt.mantissah |= mask)
  {
    rv = finitel(ieee754.ld);
    if (rv != 0)
    {
      printf("finitel failed: returned = %d  should be = 1\n", rv);
      failures++;
    }
    ieee754.ldt.exponent = 0x7FFE;
    rv = finitel(ieee754.ld);
    if (rv != 1)
    {
      printf("finitel failed: returned = %d  should be = 0\n", rv);
      failures++;
    }
  }

  printf("%s\n", failures ? "isinfl, isnanl or finitel tests failed." : "isinfl, isnanl and finitel tests succeded.");


  return failures ? 1 : 0;
}
