/*
 * File rand48.c.
 *
 * Copyright (C) 1999, 2000 Martin Str@"omberg <ams@ludd.luth.se>.
 *
 * This software may be used freely so long as this copyright notice is
 * left intact. There is no warranty on this software.
 *
 */

#include <stdlib.h>
#include <string.h>

#define RAND48_MULT0 (0xe66d)
#define RAND48_MULT1 (0xdeec)
#define RAND48_MULT2 (0x0005)
#define RAND48_ADD (0x000b)

static unsigned short internal_state[3] = {1, 0, 0};
static unsigned short multiplier0 = RAND48_MULT0;
static unsigned short multiplier1 = RAND48_MULT1;
static unsigned short multiplier2 = RAND48_MULT2;
static unsigned short additiver = RAND48_ADD;

static void next( 
		 unsigned short state[]
		 )
{
  unsigned short new_state[3];
  unsigned long tmp;

  tmp = state[0] * multiplier0 + additiver;
  new_state[0] = (unsigned short)(tmp  & 0xffff);

  tmp = (tmp >> 8*sizeof(unsigned short)) 
      + state[0] * multiplier1 
      + state[1] * multiplier0; 
  new_state[1] = (unsigned short)(tmp & 0xffff);

  tmp = (tmp >> 8*sizeof(unsigned short))
      + state[0] * multiplier2
      + state[1] * multiplier1
      + state[2] * multiplier0;
  new_state[2] = (unsigned short)(tmp & 0xffff);

  memcpy(state, new_state, 3*sizeof(unsigned short));
}

double drand48(void)
{
  return(erand48(internal_state));
}

double erand48(
	       unsigned short state[3]
	       )
{
  /* Thanks to Dieter Buerssner for showing how it ought to be done. */
  signed long long ll; /* Temporary result holder. */

  next(state);

  ll = (signed long long)( state[0] 
		       | ( (unsigned long)state[1] ) << 16 
		       | ( (unsigned long long)state[2] ) << 32 );

  return(ll * ( 1.0 / ( 1LL << 48 ) ) );

}


unsigned long lrand48(void)
{
  return(nrand48(internal_state));
}


unsigned long nrand48(
		      unsigned short state[3]
		      )
{

  next(state);
  return( ((unsigned long)state[2]) * 0x8000 
	+ ( ((unsigned long)state[1]) >> 1 )
	  );

}


long mrand48(void)
{
  return(jrand48(internal_state));
}


long jrand48(
	     unsigned short state[3]
	     )
{

  next(state);
  if( (state[2] & 0x8000) )
  {
    return( -1.0 * ((long)(state[2] & 0x7fff)) * 0x10000 + ((unsigned long)state[1]) );
  }
  else
  {
    return( ((long)(state[2] & 0x7fff)) * 0x10000 + ((unsigned long)state[1]) );
  }

}

void srand48(
	     long seedval
	     )
{

  /* Restore default multipliers and additiver. */
  multiplier0 = RAND48_MULT0;
  multiplier1 = RAND48_MULT1;
  multiplier2 = RAND48_MULT2;
  additiver = RAND48_ADD;

  /* Setup the new state. */
  internal_state[0] = 0x330e;
  internal_state[1] = (seedval & 0xffff);
  internal_state[2] = ( (seedval >> 16) & 0xffff);

}

unsigned short *seed48(
		       unsigned short state_seed[3]
		       )
{
  static unsigned short old_state[3];

  /* Restore default multipliers and additiver. */
  multiplier0 = RAND48_MULT0;
  multiplier1 = RAND48_MULT1;
  multiplier2 = RAND48_MULT2;
  additiver = RAND48_ADD;

  /* Remember old state. */
  memcpy(old_state, internal_state, 3*sizeof(unsigned short));

  /* Setup the new state. */
  memcpy(internal_state, state_seed, 3*sizeof(unsigned short));

  return(old_state);
}

void lcong48(
	     unsigned short param[7]
	     )
{
  
  /* Set the state. */
  internal_state[0] = param[0];
  internal_state[1] = param[1];
  internal_state[2] = param[2];

  /* Set the multipilers. */
  multiplier0 = param[3];
  multiplier1 = param[4];
  multiplier2 = param[5];

  /* Set the additiver. */
  additiver = param[6];

}
