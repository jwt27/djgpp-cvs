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
#include <string.h>
#include <float.h>
#include <libc/ieee.h>

void print_help_and_exit(char *name)
{
  fprintf(stderr, "%s: Run like this: '%s [new fault set]'\n"
	  "\tIf 'new fault set' is given the bits 0-5 in the FPU control word will\n"
	  "\tbe set to bits 0-5 given\n",
	  name, name);

  exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
  float f;
  double d;
  long double ld;

  if( 2 < argc || ( 2 == argc && !strncmp("-h", argv[1], 2) ) )
  {
    print_help_and_exit(argv[0]);
  }

  printf("FPU control word: 0x%08x\n", _control87(0, 0));

  if( 2 == argc )
  {
    int control_bits = 0;
    char *p = NULL;

    control_bits = strtol(argv[1], &p, 0);
    if( p == NULL || *p != '\0' )
    {
      fprintf(stderr, "%s: new fault set should be a number\n", argv[0]);
      return EXIT_FAILURE;
    }

    control_bits = control_bits & 0x3f;
    _control87(control_bits, 0x3f);
    printf("FPU control word set to: 0x%08x\n", _control87(0, 0));
  }
    
  f = NAN;
  if( (*(float_t *)(&f)).exponent == 0xff && (*(float_t *)(&f)).mantissa != 0 )
  {
    puts("float = NAN: Ok.");
  }
  else
  {
    puts("float = NAN: Fail.");
  }

  d = NAN;
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

  ld = NAN;
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

  return EXIT_SUCCESS;
}
