//      qfloat.h
//
//      C++ class for q-type floating point arithmetic.
//
// SYNOPSIS:
//
// #include "qfloat.h"
//
// qfloat a, b, c;
// a = b * c - 1.5;
//
// DESCRIPTION:
//
// Provides C++ arithmetic operators, functions, and type conversions
// for Q type floating point arithmetic.  Programs must be linked
// with the library libmq.a of supporting Q type routines.
//
// Using the GNU GCC compiler, the -O3 switch enables inlining which
// generates fairly efficient code for the various function calls.
// This class reportedly works with Microsoft and Borland C++ also.
//
// Cephes Math Library Release 2.4:  April, 1996
// Copyright 1996 by Stephen L. Moshier


#ifndef __QFLOAT_H__
#       define __QFLOAT_H__

// Needed only to get QELT and NQ:
#include "qhead.h"
#include <stdio.h>

struct qfloatstruct
  {
    QELT ar[NQ];
//    unsigned short ar[12];
  };

extern "C"
{
  void qclear (qfloatstruct *);
  void e24toq (const float *, qfloatstruct *);
  void etoq   (const double *, qfloatstruct *);
  void e64toq (const long double *, qfloatstruct *);
  void ltoq   (long int *, qfloatstruct *);
  void asctoq (const char *, qfloatstruct *);
}

class qfloat
{
  public:
  qfloatstruct a;
// ------------
// Constructor.
// ------------
  qfloat () {};
//  qfloat () {qclear(&a);};
// ----------------------------------------
// Conversions from base classes to qfloat.
// ----------------------------------------
  qfloat (float x)       {e24toq (&x, &a);}
  qfloat (double x)      {etoq (&x, &a);}
  qfloat (long double x) {e64toq (&x, &a);}
//  ------------------------------------------
//  qfloat (long double x) {e113toq (&x, &a);}	// 128-bit
//  ------------------------------------------
  qfloat (long int x) {ltoq (&x, &a);}
  qfloat (int x)      {long lx = x; ltoq (&lx, &a);}
// ----------------------------------------------
// For conversion and assignment x = "1.234e4321"
//  ----------------------------------------------
  qfloat (char *x) {asctoq (x, &a);}
// -------------------------------------------------------
// Type conversion to a base class is not possible in C++,
// so you have to use these functions instead, e.g.
//
//      int i = qtoi(q);
// -------------------------------------------------------
  friend int    xtoi (qfloat);
  friend long   xtol (qfloat);
  friend float  xtof (qfloat);
  friend double xtod (qfloat);
  friend long double xtold (qfloat);
};

extern "C"
{
  void qabs	(qfloat *);
  void qacos	(const qfloat *, qfloat *);
  void qacosh	(const qfloat *, qfloat *);
  void qadd 	(const qfloat &, const qfloat &, qfloat &);
  void qasin	(const qfloat *, qfloat *);
  void qasinh	(const qfloat *, qfloat *);
  void qatanh	(const qfloat *, qfloat *);
  void qatn	(const qfloat *, qfloat *);
  void qatn2	(const qfloat *, const qfloat *, qfloat *);
  void qcbrt	(const qfloat *, qfloat *);
  int  qcmp	(const qfloat &, const qfloat &);
  void qcos	(const qfloat *, qfloat *);
  void qcosh	(const qfloat *, qfloat *);
  void qcot	(const qfloat *, qfloat *);
  void qdiv	(const qfloat &, const qfloat &, qfloat &);
  void qerf	(const qfloat *, qfloat *);
  void qerfc	(const qfloat *, qfloat *);
  void qexp	(const qfloat *, qfloat *);
  void qexp10	(const qfloat *, qfloat *);
  void qfloor	(const qfloat *, qfloat *);
  void qfrexp	(const qfloat *, long *, qfloat *);
  void qifrac	(const qfloat *, long *, qfloat *);
  void qjn	(const qfloat *, const qfloat *, qfloat *);
  void qldexp	(const qfloat *, long  , qfloat *);
  void qlgam	(const qfloat *, qfloat *);
  void qlog	(const qfloat *, qfloat *);
  void qlog10	(const qfloat *, qfloat *);
  void qmul	(const qfloat &, const qfloat &, qfloat &);
  void qneg	(qfloat *);
  void qpow	(const qfloat *, const qfloat *, qfloat *);
  void qrand	(qfloat *);
  void qremain	(const qfloat &, const qfloat &, qfloat &);
  void qsin	(const qfloat *, qfloat *);
  void qsinh	(const qfloat *, qfloat *);
  void qsqrt	(const qfloat *, qfloat *);
  void qsrand	(unsigned int);
  void qsub	(const qfloat &, const qfloat &, qfloat &);
  void qtan	(const qfloat *, qfloat *);
  void qtanh	(const qfloat *, qfloat *);
  void qtoasc	(const qfloat *, char *, const int);
  void qtoe	(const qfloat *, double *);
  void qtoe24	(const qfloat *, float  *);
  void qtoe64	(const qfloat *, long double *);
  void qyn	(const qfloat *, const qfloat *, qfloat *);
}
// --------------------------
// Conversions to base class.
// --------------------------
inline int xtoi (qfloat x)
{
  qfloat y;
  long l;
  qifrac(&x, &l, &y);
  return (int) l;
}

inline long xtol (qfloat x)
{
  qfloat y; long l;
  qifrac(&x, &l, &y);
  return l;
}

inline double xtod (qfloat x)
{
  double d;
  qtoe(&x, &d);
  return d;
}

inline float xtof (qfloat x)
{
  float f;
  qtoe24(&x, &f);
  return f;
}

inline long double xtold (qfloat x)
{
  long double D;
  qtoe64(&x, &D);
  return D;
}

// --------------------
// Overloaded operators
// --------------------
inline qfloat operator += (qfloat & x, const qfloat & y)
{
  qadd (x, y, x);
  return x;
}

inline qfloat operator + (const qfloat & x, const qfloat & y)
{
  qfloat z;
  qadd (x, y, z);
  return z;
}

inline qfloat operator - (const qfloat & x, const qfloat & y)
{
  qfloat z;
  qsub (y, x, z);
  return z;
}

// Unary negation.
inline qfloat operator - (const qfloat & x)
{
  qfloat z;
  z = x;
  qneg (&z);
  return z;
}

inline qfloat operator -= (qfloat & x, const qfloat & y)
{
  qsub (y, x, x);
  return x;
}

inline qfloat operator * (const qfloat & x, const qfloat & y)
{
  qfloat z;
  qmul (x, y, z);
  return z;
}

inline qfloat operator *= (qfloat & x, const qfloat & y)
{
  qmul (y, x, x);
  return x;
}

inline qfloat operator / (const qfloat & x, const qfloat & y)
{
  qfloat z;
  qdiv (y, x, z);
  return z;
}

inline qfloat operator /= (qfloat & x, const qfloat & y)
{
  qdiv (y, x, x);
  return x;
}

inline qfloat operator % (const qfloat & x, const qfloat & y)
{
  qfloat z;
  qremain (y, x, z);
  return z;
}

inline int operator == (const qfloat & x, const qfloat & y)
{
  return (qcmp (x, y) == 0);
}

inline int operator != (const qfloat & x, const qfloat & y)
{
  return (qcmp (x, y) != 0);
}

inline int operator < (const qfloat & x, const qfloat & y)
{
  return (qcmp (x, y) == -1);
}

inline int operator > (const qfloat & x, const qfloat & y)
{
  return (qcmp (x, y) == 1);
}

inline int operator >= (const qfloat & x, const qfloat & y)
{
  return (qcmp (x, y) >= 0);
}

inline int operator <= (const qfloat & x, const qfloat & y)
{
  return (qcmp (x, y) <= 0);
}

// ----------------------------------------------------
// Define extended precision function calls analogously.
// ----------------------------------------------------
inline qfloat xabs (const qfloat & x)
{
  qfloat y;
  y = x;
  qabs (&y);
  return y;
}

inline qfloat xacos (const qfloat & x)
{
  qfloat y;
  qacos (&x, &y);
  return y;
}

inline qfloat xacosh (const qfloat & x)
{
  qfloat y;
  qacosh (&x, &y);
  return y;
}

inline qfloat xasin (const qfloat & x)
{
  qfloat y;
  qasin (&x, &y);
  return y;
}

inline qfloat xasinh (const qfloat & x)
{
  qfloat y;
  qasinh (&x, &y);
  return y;
}

inline qfloat xatan (const qfloat & x)
{
  qfloat y;
  qatn (&x, &y);
  return y;
}

inline qfloat xatan2 (const qfloat & y, const qfloat & x)
{
  qfloat z;
  qatn2 (&y, &x, &z);
  return z;
}

inline qfloat xatanh (const qfloat & x)
{
  qfloat y;
  qatanh (&x, &y);
  return y;
}

inline qfloat xcbrt (const qfloat & x)
{
  qfloat y;
  qcbrt (&x, &y);
  return y;
}

inline qfloat xcos (const qfloat & x)
{
  qfloat y;
  qcos (&x, &y);
  return y;
}

inline qfloat xcosh (const qfloat & x)
{
  qfloat y;
  qcosh (&x, &y);
  return y;
}

inline qfloat xcot (const qfloat & x)
{
  qfloat y;
  qcot (&x, &y);
  return y;
}

inline qfloat xerf (const qfloat & x)
{
  qfloat y;
  qerf (&x, &y);
  return y;
}

inline qfloat xerfc (const qfloat & x)
{
  qfloat y;
  qerfc (&x, &y);
  return y;
}

inline qfloat xexp (const qfloat & x)
{
  qfloat y;
  qexp (&x, &y);
  return y;
}

inline qfloat xexp10 (const qfloat & x)
{
  qfloat y;
  qexp10 (&x, &y);
  return y;
}

inline qfloat xfloor (const qfloat &x)
{
  qfloat y;
  qfloor (&x, &y);
  return y;
}

inline qfloat xfrexp (const qfloat &x, long & e)
{
  qfloat y;
  qfrexp (&x, &e, &y);
  return y;
}

inline qfloat xjn (const qfloat & x, const qfloat & y)
{
  qfloat z;
  qjn (&x, &y, &z);
  return z;
}

inline qfloat xldexp (const qfloat &x, long e)
{
  qfloat y;
  qldexp (&x, e, &y);
  return y;
}
// ---------------------------------------
// Natural Logarithm of the Gamma Function
// ---------------------------------------
inline qfloat xlgamma (const qfloat & x)
{
  qfloat y;
  qlgam (&x, &y);
  return y;
}
// -----------------
// Natural Logarithm
// -----------------
inline qfloat xlog (const qfloat & x)
{
  qfloat y;
  qlog (&x, &y);
  return y;
}
// ----------------
// Common Logarithm
// ----------------
inline qfloat xlog10 (const qfloat & x)
{
  qfloat y;
  qlog10 (&x, &y);
  return y;
}
// ------------------
// Power Function x^y
// ------------------
inline qfloat xpow (const qfloat & x, const qfloat & y)
{
  qfloat z;
  qpow (&x, &y, &z);
  return z;
}

inline qfloat xrand (void)
{
     qfloat z;
     qrand (&z);
     return z;
}

inline qfloat xsin (const qfloat & x)
{
  qfloat y;
  qsin (&x, &y);
  return y;
}

inline qfloat xsinh (const qfloat & x)
{
  qfloat y;
  qsinh (&x, &y);
  return y;
}

inline qfloat xsqrt (const qfloat & x)
{
  qfloat y;
  qsqrt (&x, &y);
  return y;
}

inline qfloat xtan (const qfloat & x)
{
  qfloat y;
  qtan (&x, &y);
  return y;
}

inline qfloat xtanh (const qfloat & x)
{
  qfloat y;
  qtanh (&x, &y);
  return y;
}

inline qfloat xyn (const qfloat & x, const qfloat & y)
{
  qfloat z;
  qyn (&x, &y, &z);
  return z;
}

// Print decimal value of X with N digits.
// Precede value with string s1, follow with string s2.
inline void qprint ( const int n, const char *s1, const qfloat &x,
                     const char *s2)
{
  char str[128];
  qtoasc (&x, str, n);
  printf ("%s %s %s", s1, str, s2);
}

inline void xprintf (const char *s1, const qfloat &x, const int n,
                     const char *s2)
{
  char str[128];
  qtoasc (&x, str, n);
  printf ("%s%s%s", s1, str, s2);
}

inline void xsprintf (const char *s1, const qfloat &x, const int n,
                     const char *s2, char *OutStr)
{
  char str[128];
  qtoasc (&x, str, n);
  sprintf (OutStr, "%s%s%s", s1, str, s2);
}
#endif /* __QFLOAT_H__ */
