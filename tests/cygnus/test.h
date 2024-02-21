#define HAVE_FLOAT 1
#define X(x) (char *)x

#include <sys/cdefs.h>
#include <errno.h>
#include <math.h>
#include <float.h>
#include <ieeefp.h>
#include <stdio.h>

#ifdef __IEEE_BIG_ENDIAN

typedef union
{
  double value;
  struct
  {
    unsigned int sign : 1;
    unsigned int exponent: 11;
    unsigned int fraction0:4;
    unsigned int fraction1:16;
    unsigned int fraction2:16;
    unsigned int fraction3:16;

  } number;
  struct
  {
    unsigned int sign : 1;
    unsigned int exponent: 11;
    unsigned int quiet:1;
    unsigned int function0:3;
    unsigned int function1:16;
    unsigned int function2:16;
    unsigned int function3:16;
  } nan;
  struct
  {
    unsigned long msw;
    unsigned long lsw;
  } parts;
    long aslong[2];
} __ieee_double_shape_type;

#endif

#ifdef __IEEE_LITTLE_ENDIAN

typedef union
{
  double value;
  struct
  {
#ifdef __SMALL_BITFIELDS
    unsigned int fraction3:16;
    unsigned int fraction2:16;
    unsigned int fraction1:16;
    unsigned int fraction0: 4;
#else
    unsigned int fraction1:32;
    unsigned int fraction0:20;
#endif
    unsigned int exponent :11;
    unsigned int sign     : 1;
  } number;
  struct
  {
#ifdef __SMALL_BITFIELDS
    unsigned int function3:16;
    unsigned int function2:16;
    unsigned int function1:16;
    unsigned int function0:3;
#else
    unsigned int function1:32;
    unsigned int function0:19;
#endif
    unsigned int quiet:1;
    unsigned int exponent: 11;
    unsigned int sign : 1;
  } nan;
  struct
  {
    unsigned long lsw;
    unsigned long msw;
  } parts;

  long aslong[2];

} __ieee_double_shape_type;

#endif

#ifdef __IEEE_BIG_ENDIAN

typedef union
{
  float value;
  struct
  {
    unsigned int sign : 1;
    unsigned int exponent: 8;
    unsigned int fraction0: 7;
    unsigned int fraction1: 16;
  } number;
  struct
  {
    unsigned int sign:1;
    unsigned int exponent:8;
    unsigned int quiet:1;
    unsigned int function0:6;
    unsigned int function1:16;
  } nan;
  long p1;

} __ieee_float_shape_type;

#endif

#ifdef __IEEE_LITTLE_ENDIAN

typedef union
{
  float value;
  struct
  {
    unsigned int fraction0: 7;
    unsigned int fraction1: 16;
    unsigned int exponent: 8;
    unsigned int sign : 1;
  } number;
  struct
  {
    unsigned int function1:16;
    unsigned int function0:6;
    unsigned int quiet:1;
    unsigned int exponent:8;
    unsigned int sign:1;
  } nan;
  long p1;

} __ieee_float_shape_type;

#endif

#ifdef _DOUBLE_IS_32BITS
#undef __ieee_double_shape_type
#define __ieee_double_shape_type __ieee_float_shape_type
#endif

typedef struct
{
  unsigned long msw, lsw;
} question_struct_type;


typedef struct
{
  char error_bit;
  char errno_val;
  char merror;
  int line;

  question_struct_type qs[3];
} one_line_type;

#define MVEC_START(x) one_line_type x[] =  {
#define MVEC_END    0,};

int _EXFUN(mag_of_error,(double, double));

#define ERROR_PERFECT 20
#define ERROR_FAIL -1

#define AAAA 15
#define AAA 10
#define AA  6
#define A   5
#define B   3
#define C   1
#define VECOPEN(x,f) \
{\
  char buffer[100];\
   sprintf(buffer,"%s_vec.c",x);\
    f = fopen(buffer,"w");\
     fprintf(f,"#include \"test.h\"\n");\
      fprintf(f," one_line_type %s_vec[] = {\n", x);\
}

#define VECCLOSE(f,name,args)\
{\
  fprintf(f,"0,};\n");      \
   fprintf(f,"void test_%s(m) int m; {run_vector_1(m,%s_vec,(char *)(%s),\"%s\",\"%s\");   }	\n",\
	   name,\
	   name,name,name,args);\
	    fclose(f);\
}

typedef struct
{
  int line;

  char *string;
  double value;
  int endscan;
} double_type;

typedef struct
{
  int line;

  char *string;
  float value;
  int endscan;
} float_type;

typedef struct
{
  long int value;
  char end;
  char errno_val;
} int_scan_type;

typedef struct
{
  int line;
  int_scan_type octal;
  int_scan_type decimal;
  int_scan_type hex;
  int_scan_type normal;
  int_scan_type alphabetical;
  char *string;
} int_type;

typedef struct
{
  int line;
  double value;
  char *estring;
  int e1;
  int e2;
  int e3;
  char *fstring;
  int f1;
  int f2;
  int f3;
  char *gstring;
  int g1;
} ddouble_type;

typedef struct
{
  int line;
  double value;
  char *result;
  char *format_string;
} sprint_double_type;

typedef struct
{
  int line;
  float value;
  char *result;
  char *format_string;
} sprint_float_type;

typedef struct
{
  int line;
  int value;
  char *result;
  char *format_string;
} sprint_int_type;

void _EXFUN(test_ieee,(void));
void _EXFUN(test_math2,(void));
void _EXFUN(test_math,(void));
void _EXFUN(test_string,(void));
void _EXFUN(test_is,(void));
void _EXFUN(test_cvt,(void));

void _EXFUN(line,(int));

void _EXFUN(test_mok, (double, double, int));
void _EXFUN(test_mokf, (float, float, int));
void _EXFUN(test_iok, (int, int));
void _EXFUN(test_eok, (int, int));
void _EXFUN(test_sok, (char *, char*));
void _EXFUN(test_scok, (char *, char*, int));
void _EXFUN(newfunc,(_CONST char *));

void _EXFUN(run_vector_1, (int, one_line_type *, char *, char *, char *));


void _EXFUN(test_acos, (int));
void _EXFUN(test_acosf, (int));
void _EXFUN(test_acosh, (int));
void _EXFUN(test_acoshf, (int));
void _EXFUN(test_asin, (int));
void _EXFUN(test_asinf, (int));
void _EXFUN(test_asinh, (int));
void _EXFUN(test_asinhf, (int));
void _EXFUN(test_atan, (int));
void _EXFUN(test_atan2, (int));
void _EXFUN(test_atan2f, (int));
void _EXFUN(test_atanf, (int));
void _EXFUN(test_atanh, (int));
void _EXFUN(test_atanhf, (int));
void _EXFUN(test_ceil, (int));
void _EXFUN(test_ceilf, (int));
void _EXFUN(test_cos, (int));
void _EXFUN(test_cosf, (int));
void _EXFUN(test_cosh, (int));
void _EXFUN(test_coshf, (int));
void _EXFUN(test_erf, (int));
void _EXFUN(test_erfc, (int));
void _EXFUN(test_erfcf, (int));
void _EXFUN(test_erff, (int));
void _EXFUN(test_exp, (int));
void _EXFUN(test_expf, (int));
void _EXFUN(test_fabs, (int));
void _EXFUN(test_fabsf, (int));
void _EXFUN(test_floor, (int));
void _EXFUN(test_floorf, (int));
void _EXFUN(test_fmod, (int));
void _EXFUN(test_fmodf, (int));
void _EXFUN(test_frexp, (int));
void _EXFUN(test_frexpf, (int));
void _EXFUN(test_gamma, (int));
void _EXFUN(test_gammaf, (int));
void _EXFUN(test_hypot, (int));
void _EXFUN(test_hypotf, (int));
void _EXFUN(test_j0, (int));
void _EXFUN(test_j0f, (int));
void _EXFUN(test_j1, (int));
void _EXFUN(test_j1f, (int));
void _EXFUN(test_jn, (int));
void _EXFUN(test_jnf, (int));
void _EXFUN(test_ldexp, (int));
void _EXFUN(test_ldexpf, (int));
void _EXFUN(test_log, (int));
void _EXFUN(test_logf, (int));
void _EXFUN(test_log10, (int));
void _EXFUN(test_log10f, (int));
void _EXFUN(test_log1p, (int));
void _EXFUN(test_log1pf, (int));
void _EXFUN(test_log2, (int));
void _EXFUN(test_log2f, (int));
void _EXFUN(test_modf, (int));
void _EXFUN(test_modff, (int));
void _EXFUN(test_pow, (int));
void _EXFUN(test_powf, (int));
void _EXFUN(test_sin, (int));
void _EXFUN(test_sinf, (int));
void _EXFUN(test_sinh, (int));
void _EXFUN(test_sinhf, (int));
void _EXFUN(test_sqrt, (int));
void _EXFUN(test_sqrtf, (int));
void _EXFUN(test_tan, (int));
void _EXFUN(test_tanf, (int));
void _EXFUN(test_tanh, (int));
void _EXFUN(test_tanhf, (int));
void _EXFUN(test_y0, (int));
void _EXFUN(test_y0f, (int));
void _EXFUN(test_y1, (int));
void _EXFUN(test_y1f, (int));
void _EXFUN(test_yn, (int));
void _EXFUN(test_ynf, (int));
