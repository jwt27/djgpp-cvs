/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2007 DJ Delorie, see COPYING.DJ for details */
/*
 * File _hash2v.c.
 *
 * From <http://burtleburtle.net/bob/hash/doobs.html>.
 * Two lines from below quoted: 
 * "By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
 * code any way you wish, private, educational, or commercial.  It's free."
 *
 * Modifications by Martin Str@"omberg <ams@ludd.ltu.se>:
 *   - unnecessary code removed
 *   - ub1 and ub4 typedefs converted into unsigned char and unsigned long
 *   - proper C89 parameters
 *   - detect length while calculating
 *   - fixed initial value (0)
 *   - generate a second hash value (b)
 *   - #include**s added
 *
 */

#include <search.h>

/*
--------------------------------------------------------------------
mix -- mix 3 32-bit values reversibly.
For every delta with one or two bits set, and the deltas of all three
  high bits or all three low bits, whether the original value of a,b,c
  is almost all zero or is uniformly distributed,
* If mix() is run forward or backward, at least 32 bits in a,b,c
  have at least 1/4 probability of changing.
* If mix() is run forward, every bit of c will change between 1/3 and
  2/3 of the time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
mix() was built out of 36 single-cycle latency instructions in a 
  structure that could supported 2x parallelism, like so:
      a -= b; 
      a -= c; x = (c>>13);
      b -= c; a ^= x;
      b -= a; x = (a<<8);
      c -= a; b ^= x;
      c -= b; x = (b>>13);
      ...
  Unfortunately, superscalar Pentiums and Sparcs can't take advantage 
  of that parallelism.  They've also turned some of those single-cycle
  latency instructions into multi-cycle latency instructions.  Still,
  this is the fastest good hash I could find.  There were about 2^^68
  to choose from.  I only looked at a billion or so.
--------------------------------------------------------------------
*/
#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

unsigned long _hash2v( const unsigned char *k, unsigned long *v2 )
{
  int cont, which;
  unsigned long a, b, c, len;

  /* Set up the internal state */
  len = 0;
  a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
  c = 0;         /* No previous hash value. */

   /*---------------------------------------- handle most of the key */
  cont = 1;
  while( cont )
  {
    for( which = 0; which < 12; which++ )
    {
      if( !k[which] )
      {
	cont = 0;
	break;
      }
    }
    if( cont )
    {
      a += (k[0] +((unsigned long)k[1]<<8) +((unsigned long)k[2]<<16)
	    +((unsigned long)k[3]<<24));
      b += (k[4] +((unsigned long)k[5]<<8) +((unsigned long)k[6]<<16)
	    +((unsigned long)k[7]<<24));
      c += (k[8] +((unsigned long)k[9]<<8) +((unsigned long)k[10]<<16)
	    +((unsigned long)k[11]<<24));
      mix(a,b,c);
      k += 12; len += 12;
    }
  }

  len += which;

   /*------------------------------------- handle the last 11 bytes */
  c += len;
  switch(which)              /* all the case statements fall through */
  {
  case 11: c+=((unsigned long)k[10]<<24);
  case 10: c+=((unsigned long)k[9]<<16);
  case 9 : c+=((unsigned long)k[8]<<8);
    /* the first byte of c is reserved for the length */
  case 8 : b+=((unsigned long)k[7]<<24);
  case 7 : b+=((unsigned long)k[6]<<16);
  case 6 : b+=((unsigned long)k[5]<<8);
  case 5 : b+=k[4];
  case 4 : a+=((unsigned long)k[3]<<24);
  case 3 : a+=((unsigned long)k[2]<<16);
  case 2 : a+=((unsigned long)k[1]<<8);
  case 1 : a+=k[0];
    /* case 0: nothing left to add */
  }
  mix(a,b,c);
  /*-------------------------------------------- report the result */

  *v2 = b;
  return c;
}


#ifdef TEST
/*
 * Original hash function code.
 *
 */

typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;   /* unsigned 1-byte quantities */

/*
--------------------------------------------------------------------
hash() -- hash a variable-length key into a 32-bit value
  k       : the key (the unaligned variable-length array of bytes)
  len     : the length of the key, counting by bytes
  initval : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Every 1-bit and 2-bit delta achieves avalanche.
About 6*len+35 instructions.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 32 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
In which case, the hash table should have hashsize(10) elements.

If you are hashing n strings (ub1 **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);

By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
code any way you wish, private, educational, or commercial.  It's free.

See http://burtleburtle.net/bob/hash/evahash.html
Use for hash table lookup, or anything where one collision in 2^^32 is
acceptable.  Do NOT use for cryptographic purposes.
--------------------------------------------------------------------
*/

ub4 hash( k, length, initval)
register ub1 *k;        /* the key */
register ub4  length;   /* the length of the key */
register ub4  initval;  /* the previous hash, or an arbitrary value */
{
   register ub4 a,b,c,len;

   /* Set up the internal state */
   len = length;
   a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
   c = initval;         /* the previous hash value */

   /*---------------------------------------- handle most of the key */
   while (len >= 12)
   {
      a += (k[0] +((ub4)k[1]<<8) +((ub4)k[2]<<16) +((ub4)k[3]<<24));
      b += (k[4] +((ub4)k[5]<<8) +((ub4)k[6]<<16) +((ub4)k[7]<<24));
      c += (k[8] +((ub4)k[9]<<8) +((ub4)k[10]<<16)+((ub4)k[11]<<24));
      mix(a,b,c);
      k += 12; len -= 12;
   }

   /*------------------------------------- handle the last 11 bytes */
   c += length;
   switch(len)              /* all the case statements fall through */
   {
   case 11: c+=((ub4)k[10]<<24);
   case 10: c+=((ub4)k[9]<<16);
   case 9 : c+=((ub4)k[8]<<8);
      /* the first byte of c is reserved for the length */
   case 8 : b+=((ub4)k[7]<<24);
   case 7 : b+=((ub4)k[6]<<16);
   case 6 : b+=((ub4)k[5]<<8);
   case 5 : b+=k[4];
   case 4 : a+=((ub4)k[3]<<24);
   case 3 : a+=((ub4)k[2]<<16);
   case 2 : a+=((ub4)k[1]<<8);
   case 1 : a+=k[0];
     /* case 0: nothing left to add */
   }
   mix(a,b,c);
   /*-------------------------------------------- report the result */
   return c;
}


/* Code to make sure that _hash2v() behaves identically to hash(). */

#include <errno.h>
#include <stdio.h>

#define LEN 4096

int main(int argc, char *argv[])
{
  char *ret;
  char s[LEN];
  FILE *f;
  size_t len;
  unsigned long hash_v, hash2v_v, junk;

  if( argc != 2 )
  {
    fprintf(stderr, "Run like this: '%s <file name>'.\n", argv[0]);
    return 1;
  }

  f = fopen(argv[1], "r");
  if( !f )
  {
    fprintf(stderr, "Failed to open '%s', errno = %d.\n", argv[1], errno);
    return 3;
  }

  ret = fgets(s, LEN, f);
  while( ret )
  {
    len = strlen(s);
    hash_v = hash(s, len, 0);
    hash2v_v = _hash2v(s, &junk);
    if( hash_v != hash2v_v )
    {
      printf("hash = 0x%08lx != 0x%08lx = hash2v\n", hash_v, hash2v_v);
    }

    ret = fgets(s, LEN, f);
  }

  return 0;
}

#endif
