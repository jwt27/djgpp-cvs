/*
 * File _hash2v.c.
 *
 * Copyright (C) 2005 Martin Str@"omberg <ams@ludd.ltu.se>.
 *
 * This software may be used freely so long as this copyright notice is
 * left intact. There is no warranty on this software.
 *
 */

#include <errno.h>
#include <math.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>

#define LEN 4096
#define MAX_BUCKETS 0x20000000

int main(int argc, char *argv[])
{
  char *ret;
  char s[LEN];
  FILE *f;
  long double chi2, corr, p_inv, s2_h, s2_h2;
  size_t buckets, i, len, lines, mask, v, v2;
  size_t *h, *h2;

  if( argc != 3 )
  {
    fprintf(stderr, "Run like this: '%s <buckets> <file name>'.\n"
	    "Make sure that the lines are unique (run the files throught 'sort -u' e.g.),\n"
	    "otherwise the results will not be correct.\n", argv[0]);
    return 1;
  }

  buckets = strtoul(argv[1], NULL, 0);

  /* This number of bucket finding _must_ give the same number of buckets
     as the one in hcreate(). */
  buckets += buckets/3;
  i = 7;
  if( buckets < 0x80 )
  {
    buckets = 0x80;
  }
  if( MAX_BUCKETS < buckets )
  {
    buckets = MAX_BUCKETS;
    i = 26;
  }

  while( (1<<i) < buckets )
  {
    i++;
  }
  buckets = 1<<i;
  mask = buckets-1;
  printf("Using %lu buckets (mask 0x%08lx).\n", buckets, mask);

  h = calloc(buckets, sizeof(size_t));
  if( !h )
  {
    fprintf(stderr, "Out of memory while trying to allocate 0x%lx bytes.\n", buckets*sizeof(size_t));
    return 2;
  }
  h2 = calloc(buckets, sizeof(size_t));
  if( !h2 )
  {
    fprintf(stderr, "Out of memory while trying to allocate 0x%lx bytes.\n", buckets*sizeof(size_t));
    return 2;
  }

  f = fopen(argv[2], "r");
  if( !f )
  {
    fprintf(stderr, "Failed to open '%s', errno = %d.\n", argv[2], errno);
    return 3;
  }

  corr = 0;
  s2_h = 0;
  s2_h2 = 0;
  lines = 0;
  ret = fgets(s, LEN, f);
  while( ret )
  {
    len = strlen(s);
    if( LEN-1 <= len )
    {
      fprintf(stderr, "Warning: Found line (%ld) with length at least %d (if longer\n"
	      "it's been splitted).\n", lines, LEN-1);
    }
    lines++;
    v = _hash2v(s, &v2)&mask;
    v2 &= mask;

#if 0
    printf("v=0x%08lx, v2=0x%08lx\n", v, v2);
#endif

    /* For chi2 test. */
    h[v]++;
    if( ! h[v] )
    {
      fprintf(stderr, "Overflow while increasing bucket 0x%08lx.\n", v);
      return 4;
    }
    h2[v2]++;
    if( ! h2[v2] )
    {
      fprintf(stderr, "Overflow while increasing bucket 0x%08lx.\n", v2);
      return 4;
    }

    /* For correlation coefficent. */
    s2_h += (v-((long double)buckets)/2)*(v-((long double)buckets)/2);
    s2_h2 += (v2-((long double)buckets)/2)*(v2-((long double)buckets)/2);
    corr += (v-((long double)buckets)/2)*(v2-((long double)buckets)/2);

    ret = fgets(s, LEN, f);
  }

  printf("Hashed %ld lines.\n", lines);

  /* Calculate chi2 value for the first hash value. */
  p_inv = buckets;

  chi2 = 0;
  for( i = 0; i < buckets; i++ )
  {
    chi2 += p_inv*h[i]*h[i]/lines;
  }
  chi2 -= lines;
  printf("Chi2 of first hash value = %Lf. Expected %ld+-%f (%f..%f).\n"
	 , chi2, buckets, 3*sqrt(buckets), buckets-3*sqrt(buckets)
	 , buckets+3*sqrt(buckets));

  /* Calculate chi2 value for the second hash value. */
  chi2 = 0;
  for( i = 0; i < buckets; i++ )
  {
    chi2 += p_inv*h2[i]*h2[i]/lines;
  }
  chi2 -= lines;
  printf("Chi2 of second hash value = %Lf. Expected %ld+-%f (%f..%f).\n"
	 , chi2, buckets, 3*sqrt(buckets), buckets-3*sqrt(buckets)
	 , buckets+3*sqrt(buckets));

  /* Calculate correlation coefficient. */
  s2_h /= lines-1;
  s2_h2 /= lines-1;
  corr /= lines-1;

  printf("Correlation coefficient = %Lf.\n", corr/sqrt(s2_h)/sqrt(s2_h2));
  

  return 0;
}
