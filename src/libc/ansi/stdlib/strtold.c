/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <locale.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <libc/unconst.h>
#include <libc/ieee.h>

static long double powten[] =
{
  1e1L, 1e2L, 1e4L, 1e8L, 1e16L, 1e32L, 1e64L, 1e128L, 1e256L,
  1e512L, 1e1024L, 1e2048L, 1e4096L
};

long double
strtold(const char *s, char **sret)
{
  long double r;		/* result */
  int e, ne;			/* exponent */
  int sign;			/* +- 1.0 */
  int esign;
  int flags=0;
  int l2powm1;
  char decimal = localeconv()->decimal_point[0];

  r = 0.0L;
  sign = 1;
  e = ne = 0;
  esign = 1;

  while(*s && isspace((unsigned char)*s))
    s++;

  if (*s == '+')
    s++;
  else if (*s == '-')
  {
    sign = -1;
    s++;
  }

  /* Handle INF and INFINITY. */
  if ( ! strnicmp( "INF", s, 3 ) )
  {
    if( sret )
    {
      if ( ! strnicmp( "INITY", &s[3], 5 ) )
      {
	*sret = unconst((&s[8]), char *);
      }
      else
      {
	*sret = unconst((&s[3]), char *);
      }
    }

    if( 0 <= sign )
    {
      return INFINITY;
    }
    else
    {
      return -INFINITY;
    }
  }

  /* Handle NAN and NAN(<whatever>). */
  if ( ! strnicmp( "NAN", s, 3 ) )
  {
    _longdouble_union_t t;

    t.ld = NAN;


    if ( sign < 0 )
    {
      t.ldt.sign = 1;
    }
    
    if( s[3] == '(' )
    {
      unsigned long long mantissa_bits = 0;
      char *endptr = unconst((&s[4]), char *);
      
      mantissa_bits = strtoull(&s[4], &endptr, 0);
      if ( *endptr == ')' )
      {
	mantissa_bits = mantissa_bits & 0x7fffffffffffffffULL; /* Ignore
							       integer
							       bit. */
	if( mantissa_bits )
	{
	  t.ldt.mantissal = mantissa_bits & 0xffffffff;
	  t.ldt.mantissah = ((mantissa_bits >> 32) & 0xffffffff ) | 0x80000000;
	}
	if( sret )
	{
          *sret = endptr+1;
	}
        return (t.ld);
      }
      
      /* The subject sequence didn't match NAN(<number>), so match
	 only NAN. */
    }

    if( sret )
    {
      *sret = unconst((&s[3]), char *);
    }
    return (t.ld);
  }

  /* Handle ordinary numbers. */
  while ((*s >= '0') && (*s <= '9'))
  {
    flags |= 1;
    r *= 10.0L;
    r += *s - '0';
    s++;
  }

  if (*s == decimal)
  {
    s++;
    while ((*s >= '0') && (*s <= '9'))
    {
      flags |= 2;
      r *= 10.0L;
      r += *s - '0';
      s++;
      ne++;
    }
  }
  if (flags == 0)
  {
    if (sret)
      *sret = unconst(s, char *);
    return 0.0L;
  }

  if ((*s == 'e') || (*s == 'E'))
  {
    s++;
    if (*s == '+')
      s++;
    else if (*s == '-')
    {
      s++;
      esign = -1;
    }
    while ((*s >= '0') && (*s <= '9'))
    {
      e *= 10;
      e += *s - '0';
      s++;
    }
  }
  if (esign < 0)
  {
    esign = -esign;
    e = -e;
  }
  e = e - ne;
  if (e < -4096)
  {
    /* possibly subnormal number, 10^e would overflow */
    r *= 1.0e-2048L;
    e += 2048;
  }
  if (e < 0)
  {
    e = -e;
    esign = -esign;
  }
  if (e >= 8192)
    e = 8191;
  if (e)
  {
    long double d = 1.0L;
    l2powm1 = 0;
    while (e)
    {
      if (e & 1)
	d *= powten[l2powm1];
      e >>= 1;
      l2powm1++;
    }
    if (esign > 0)
      r *= d;
    else
      r /= d;
  }
  if (sret)
    *sret = unconst(s, char *);
  return r * sign;
}
