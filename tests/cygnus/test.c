#include <signal.h>
#include  "test.h"
#include <math.h>
#include <ieeefp.h>
#include <string.h>
#include <unistd.h>

int verbose;
static int count;
int inacc;
void bt(void);

int
_DEFUN(main,(ac, av),
       int ac _AND
       char **av)
{
  int i;
  int math2 = 1;
  int string= 1;
  int is = 1;
  int math= 1;
  int cvt = 1;
  int ieee= 1;
#ifdef __DJGPP__
  unsigned orig_fpu_ctrl = _control87(0, 0);
  _clear87();
  _fpreset();
  _control87(0x033f, 0xffff);	/* mask all numeric exceptions */

  /* In case of numeric problems, we need errno to be set,
     but we don't want `matherr' to be called.  This is what
     the _POSIX_ version of the library does.  */
  _LIB_VERSION = _POSIX_;
#endif
  bt();
  for (i = 1; i < ac; i++) 
  {
    if (strcmp(av[i],"-v")==0) 
     verbose ++;
    if (strcmp(av[i],"-nomath2") == 0)
     math2 = 0;
    if (strcmp(av[i],"-nostrin") == 0)
     string= 0;
    if (strcmp(av[i],"-nois") == 0)
     is = 0;
    if (strcmp(av[i],"-nomath") == 0)
     math= 0;
    if (strcmp(av[i],"-nocvt") == 0)
     cvt = 0;
    if (strcmp(av[i],"-noiee") == 0)
     ieee= 0;
  }
  if (cvt)
   test_cvt();
  
  if (math2)
   test_math2();
  if (string)
   test_string();
  if (math)
   test_math();
  if (is)
   test_is();
  if (ieee)  test_ieee();
#ifdef __DJGPP__
  /* Restore the FPU state after all the atrocities we've done.  */
  _clear87 ();
  _fpreset ();
  _control87 (orig_fpu_ctrl, 0xffff);
#endif
  printf("Tested %d functions, %d errors detected\n", count, inacc);
  if (!isatty(fileno(stdout)))
    fprintf(stderr, "Tested %d functions, %d errors detected\n", count, inacc);
  return 0;
}


static _CONST char *iname = "foo";
void 
_DEFUN(newfunc,(string),
       _CONST char *string)
{
  if (strcmp(iname, string)) 
  {
    printf("testing %s\n", string);
    fflush(stdout);
    iname = string;
  }
  
}


static int theline;

void line(li)
int li;
{
  if (verbose)  
  {
    printf("  %d\n", li);
  }
  theline = li;
  
  count++;
}



int redo = 0;
int reduce = 0;

int strtod_vector = 0;

int 
_DEFUN(bigger,(a,b),
	   __ieee_double_shape_type *a  _AND
	   __ieee_double_shape_type *b)
{

  if (a->parts.msw > b->parts.msw) 
    {

      return 1;
    } 
  else if (a->parts.msw == b->parts.msw) 
    {
      if (a->parts.lsw > b->parts.lsw) 
	{
	  return 1;
	}
    }
  return 0;
}



/* Return the first bit different between two double numbers */
int 
_DEFUN(mag_of_error,(is, shouldbe),
       double is _AND
       double shouldbe)
{
  __ieee_double_shape_type a,b;
  int i;
  int a_big;
  unsigned  int mask;
  unsigned long int __x;
  unsigned long int msw, lsw;						  
  a.value = is;
  
  b.value = shouldbe;
  
  if (a.parts.msw == b.parts.msw 
      && a.parts.lsw== b.parts.lsw) return 64;


  /* Subtract the larger from the smaller number */

  a_big = bigger(&a, &b);

  if (!a_big) {
    int t;
    t = a.parts.msw;
    a.parts.msw = b.parts.msw;
    b.parts.msw = t;

    t = a.parts.lsw;
    a.parts.lsw = b.parts.lsw;
    b.parts.lsw = t;
  }



  __x = (a.parts.lsw) - (b.parts.lsw);							
  msw = (a.parts.msw) - (b.parts.msw) - (__x > (a.parts.lsw));
  lsw = __x;								

  


  /* Find out which bit the difference is in */
  mask = 0x80000000UL;
  for (i = 0; i < 32; i++)
  {
    if (((msw) & mask)!=0) return i;
    mask >>=1;
  }
  
  mask = 0x80000000UL;
  for (i = 0; i < 32; i++)
  {
    
    if (((lsw) & mask)!=0) return i+32;
    mask >>=1;
  }
  
  return 64;
  
}

 int ok_mag;



void
_DEFUN(test_sok,(is, shouldbe),
       char *is _AND
       char *shouldbe)
{
  if (strcmp(is,shouldbe))
    {
    printf("%s:%d, inacurate answer: (%s should be %s)\n",
	   iname, 
	   theline,
	   is, shouldbe);
    inacc++;
  }
}
void
_DEFUN(test_iok,(is, shouldbe),
       int is _AND
       int shouldbe)
{
  if (is != shouldbe){
    printf("%s:%d, inacurate answer: (%08x should be %08x)\n",
	   iname, 
	   theline,
	   is, shouldbe);
    inacc++;
  }
}


/* Compare counted strings upto a certain length - useful to test single
   prec float conversions against double results
*/
void 
_DEFUN(test_scok,(is, shouldbe, len),
       char *is _AND
       char *shouldbe _AND
       int len)
{
  if (strncmp(is,shouldbe, len))
    {
    printf("%s:%d, inacurate answer: (%s should be %s)\n",
	   iname, 
	   theline,
	   is, shouldbe);
    inacc++;
  }
}

void
_DEFUN(test_eok,(is, shouldbe),
       int is _AND
       int shouldbe)
{
  if (is != shouldbe){
    printf("%s:%d, bad errno answer: (%d should be %d)\n",
	   iname, 
	   theline,
	   is, shouldbe);
    inacc++;
  }
}

void
_DEFUN(test_mok,(value, shouldbe, okmag),
       double value _AND
       double shouldbe _AND
       int okmag)
{
  __ieee_double_shape_type a,b;
  int mag = mag_of_error(value, shouldbe);
  if (mag == 0) 
  {
    /* error in the first bit is ok if the numbers are both 0 */
    if (value == 0.0 && shouldbe == 0.0)
     return;
    
  }
  a.value = shouldbe;
  b.value = value;
  
  if (mag < okmag) 
  {
    printf("%s:%d, wrong answer: bit %d ",
	   iname, 
	   theline,
	   mag);
     printf("%08lx%08lx %08lx%08lx) ",
	    a.parts.msw,	     a.parts.lsw,
	    b.parts.msw,	     b.parts.lsw);
    printf("(%g %g)\n",   a.value, b.value);
    inacc++;
  }
}

#ifdef __PCCNECV70__
kill() {}
getpid() {}
#endif

void bt(){

  double f1,f2;
  f1 = 0.0;
  f2 = 0.0/f1;
  printf("0.0 divided by 0.0 gives %g\n", f2);

}

#ifdef __DJGPP__

/* In case log2 and log2f are macros, define real functions, so that
   their address could be taken (one of the tests needs this).  */
#ifdef log2
double
(log2)(double arg)
{
  return log2(arg);
}
#endif /* log2 */
float
(log2f)(float arg)
{
  return log2f(arg);
}

#endif /* __DJGPP__ */
