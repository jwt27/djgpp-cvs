
#include "test.h"
#include <ieeefp.h>

#ifdef __DJGPP__

fp_rnd
fpgetround (void)
{
  unsigned ctrl = _control87 (0, 0);

  switch ((ctrl & MCW_RC)) {
    case RC_NEAR:
      return FP_RN;
    case RC_DOWN:
      return FP_RM;
    case RC_UP:
      return FP_RP;
    default:
      return FP_RZ;
  }
}

fp_rnd
fpsetround (fp_rnd new_rounding)
{
  fp_rnd old_rounding = fpgetround ();
  unsigned newcw;

  switch (new_rounding) {
    case FP_RN:
      newcw = RC_NEAR;
      break;
    case FP_RM:
      newcw = RC_DOWN;
      break;
    case FP_RP:
      newcw = RC_UP;
      break;
    default:
      newcw = RC_CHOP;
  }

  _control87 (newcw, MCW_RC);
  return old_rounding;
}

fp_except
fpgetmask (void)
{
  unsigned ctrl = _control87 (0, 0);
  fp_except retval = 0;

  if ((ctrl & EM_INVALID) == 0)
    retval |= FP_X_INV;
  if ((ctrl & EM_ZERODIVIDE) == 0)
    retval |= FP_X_DX;
  if ((ctrl & EM_OVERFLOW) == 0)
    retval |= FP_X_OFL;
  if ((ctrl & EM_UNDERFLOW) == 0)
    retval |= FP_X_UFL;
  if ((ctrl & EM_INEXACT) == 0)
    retval |= FP_X_IMP;

  return retval;
}

fp_except
fpsetmask (fp_except new_mask)
{
  fp_except old_mask = fpgetmask ();
  unsigned ctrl = ~0;

  _clear87 ();
  _fpreset ();

  if ((new_mask & FP_X_INV))
    ctrl &= ~EM_INVALID;
  if ((new_mask & FP_X_DX))
    ctrl &= ~EM_ZERODIVIDE;
  if ((new_mask & FP_X_OFL))
    ctrl &= ~EM_OVERFLOW;
  if ((new_mask & FP_X_UFL))
    ctrl &= ~EM_UNDERFLOW;
  if ((new_mask & FP_X_IMP))
    ctrl &= ~EM_INEXACT;

  _control87 (ctrl, MCW_EM);
  return old_mask;
}

fp_except
fpgetsticky (void)
{
  return fpgetmask ();
}

fp_except
fpsetsticky (fp_except new)
{
  return fpsetmask (new);
}

fp_rdi
fpgetroundtoi (void)
{
  return 0;
}

fp_rdi
fpsetroundtoi (fp_rdi new)
{
  return 0;
}

#endif


/* Test fp getround and fp setround */

void
_DEFUN_VOID(test_getround)
{

  newfunc("fpgetround/fpsetround");
  line(1);
  fpsetround(FP_RN);
  test_iok(fpgetround(), FP_RN);
  line(2);
  fpsetround(FP_RM);
  test_iok(fpgetround(), FP_RM);
  line(3);
  fpsetround(FP_RP);
  test_iok(fpgetround(), FP_RP);
  line(4);
  fpsetround(FP_RZ);
  test_iok(fpgetround(), FP_RZ);
}

/* And fpset/fpgetmask */
void
_DEFUN_VOID(test_getmask)
{
  newfunc("fpsetmask/fpgetmask");
  line(1);
  fpsetmask(FP_X_INV);
  test_iok(fpgetmask(),FP_X_INV);
  line(2);
  fpsetmask(FP_X_DX);
  test_iok(fpgetmask(),FP_X_DX);
  line(3);
  fpsetmask(FP_X_OFL );
  test_iok(fpgetmask(),FP_X_OFL);
  line(4);
  fpsetmask(FP_X_UFL);
  test_iok(fpgetmask(),FP_X_UFL);
  line(5);
  fpsetmask(FP_X_IMP);
  test_iok(fpgetmask(),FP_X_IMP);
}

void
_DEFUN_VOID(test_getsticky)
{
  newfunc("fpsetsticky/fpgetsticky");
  line(1);
  fpsetsticky(FP_X_INV);
  test_iok(fpgetsticky(),FP_X_INV);
  line(2);
  fpsetsticky(FP_X_DX);
  test_iok(fpgetsticky(),FP_X_DX);
  line(3);
  fpsetsticky(FP_X_OFL );
  test_iok(fpgetsticky(),FP_X_OFL);
  line(4);
  fpsetsticky(FP_X_UFL);
  test_iok(fpgetsticky(),FP_X_UFL);
  line(5);
  fpsetsticky(FP_X_IMP);
  test_iok(fpgetsticky(),FP_X_IMP);
}

void
_DEFUN_VOID(test_getroundtoi)
{
  newfunc("fpsetroundtoi/fpgetroundtoi");
  line(1);
  fpsetroundtoi(FP_RDI_TOZ);
  test_iok(fpgetroundtoi(),FP_RDI_TOZ);

  line(2);
  fpsetroundtoi(FP_RDI_RD);
  test_iok(fpgetroundtoi(),FP_RDI_RD);

}

double
 _DEFUN(dnumber,(msw, lsw),
	int msw _AND
	int lsw)
{

  __ieee_double_shape_type v;
  v.parts.lsw = lsw;
  v.parts.msw = msw;
  return v.value;
}

  /* Lets see if changing the rounding alters the arithmetic.
     Test by creating numbers which will have to be rounded when
     added, and seeing what happens to them */
 /* Keep them out here to stop  the compiler from folding the results */
double n;
double m;
double add_rounded_up;
double add_rounded_down;
double sub_rounded_down ;
double sub_rounded_up ;
  double r1,r2,r3,r4;
void
_DEFUN_VOID(test_round)
{
  n =                dnumber(0x40000000, 0x00000008); /* near 2 */
  m =                dnumber(0x40400000, 0x00000003); /* near 3.4 */

  add_rounded_up   = dnumber(0x40410000, 0x00000004); /* For RN, RP */
  add_rounded_down = dnumber(0x40410000, 0x00000003); /* For RM, RZ */
  sub_rounded_down = dnumber(0xc0410000, 0x00000004); /* for RN, RM */
  sub_rounded_up   = dnumber(0xc0410000, 0x00000003); /* for RP, RZ */

  newfunc("fpsetround");

  line(1);

#ifdef __DJGPP__
  _clear87 ();
  _fpreset ();
  _control87 (0x033f, 0xffff);	/* mask all numeric exceptions */
#endif

  fpsetround(FP_RN);
  r1 = n + m;
  test_mok(r1, add_rounded_up, 64);

  line(2);
  fpsetround(FP_RM);
  r2 = n + m;
  test_mok(r2, add_rounded_down, 64);

  fpsetround(FP_RP);
  line(3);
  r3 = n + m;
  test_mok(r3,add_rounded_up, 64);

  fpsetround(FP_RZ);
  line(4);
  r4 = n + m;
  test_mok(r4,add_rounded_down,64);


  fpsetround(FP_RN);
  r1 = - n - m;
  line(5);
  test_mok(r1,sub_rounded_down,64);

  fpsetround(FP_RM);
  r2 = - n - m;
  line(6);
  test_mok(r2,sub_rounded_down,64);


  fpsetround(FP_RP);
  r3 = - n - m;
  line(7);
  test_mok(r3,sub_rounded_up,64);

  fpsetround(FP_RZ);
  r4 = - n - m;
  line(8);
  test_mok(r4,sub_rounded_up,64);
}


void
_DEFUN_VOID(test_ieee)
{
  fp_rnd old = fpgetround();
  test_getround();
  test_getmask();
  test_getsticky();
  test_getroundtoi();

  test_round();
  fpsetround(old);


}


