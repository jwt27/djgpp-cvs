/*							qrand.c
 *
 *	Pseudorandom number generator
 *
 *
 *
 * SYNOPSIS:
 *
 * QELT q[NQ];
 *
 * drand( q );
 *
 *
 *
 * DESCRIPTION:
 *
 * Yields a random number 0.0 <= q < 1.0.
 *
 * The three-generator congruential algorithm by Brian
 * Wichmann and David Hill (BYTE magazine, March, 1987,
 * pp 127-8) is used. The period, given by them, is
 * 6953607871644.
 */



#include "qhead.h"
extern	QELT	qone[];
/*  Three-generator random number algorithm
 * of Brian Wichmann and David Hill
 * BYTE magazine, March, 1987 pp 127-8
 *
 * The period, given by them, is (p-1)(q-1)(r-1)/4 = 6.95e12.
 */
#define	iabs(x) (((int)x < 0) ? -(int)x : (int)x)
static int sx = 1;
static int sy = 10000;
static int sz = 3000;

/* Initializes starting seed */
void
qsrand(const unsigned UserSeed)
{
    sx = (iabs(UserSeed) | 1)   % 30000;
    sy = iabs(UserSeed + 10000) % 30000;
    sz = iabs(UserSeed +  3000) % 30000;
}
/* This function implements the three
 * congruential generators.
 */
static
int ranwh()
{
int r, s;

/*  sx = sx * 171 mod 30269 */
r = sx/177;
s = sx - 177 * r;
sx = 171 * s - 2 * r;
if( sx < 0 )
	sx += 30269;


/* sy = sy * 172 mod 30307 */
r = sy/176;
s = sy - 176 * r;
sy = 172 * s - 35 * r;
if( sy < 0 )
	sy += 30307;

/* sz = 170 * sz mod 30323 */
r = sz/178;
s = sz - 178 * r;
sz = 170 * s - 63 * r;
if( sz < 0 )
	sz += 30323;
/* The results are in static sx, sy, sz. */
return 0;
}


int qrand( QELT q[] )
{
QELT r;
int i;

for( i=0; i<NQ-3; i++ )
  {
#if WORDSIZE == 16
    ranwh();
    r = (sx * sy) + sz;
#else /* WORDSIZE is 32 */
    ranwh();
    r = ((sx * sy) + sz) & 0xffff;
    ranwh();
    r = r | (((sx * sy) + sz) << 16);
#endif
    q[i+3] = r;
  }
q[0] = 0;
q[1] = EXPONE;
q[2] = 0;
/* Ensure the significand is normalized.  */
#if WORDSIZE == 32
q[3] |= 0x80000000;
#else
q[3] |= 0x8000;
#endif
qsub(qone, q, q);
return 0;
}
