/*
 * File t-nan.c.
 *
 * Copyright (C) 2003 Martin Str@"omberg <ams@ludd.luth.se>.
 *
 * This software may be used freely so long as this copyright notice is
 * left intact. There is no warranty on this software.
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <libc/ieee.h>

int
main (void)
{
  float f = NAN;
  double d = NAN;
  long double ld = NAN;

  if( (*(float_t *)(&f)).exponent == 0xff && (*(float_t *)(&f)).mantissa != 0 )
  {
    puts("float = NAN: Ok.");
  }
  else
  {
    puts("float = NAN: Fail.");
  }

  if( (*(double_t *)(&d)).exponent == 0x7ff 
   && ( (*(double_t *)(&d)).mantissah != 0
     || (*(double_t *)(&d)).mantissal != 0 ) )
  {
    puts("double = NAN: Ok.");
  }
  else
  {
    puts("double = NAN: Fail.");
  }

  if( (*(long_double_t *)(&ld)).exponent == 0x7fff 
   && ( (*(long_double_t *)(&ld)).mantissah != 0
     || (*(long_double_t *)(&ld)).mantissal != 0 ) )
  {
    puts("long double = NAN: Ok.");
  }
  else
  {
    puts("long double = NAN: Fail.");
  }

  return(EXIT_SUCCESS);
}
