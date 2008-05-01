/*
 * printf3.c
 * Test cases for %a and %A conversion specifiers new to the ANSI C99 standard.
 */

#include <stdio.h>
#include <stdlib.h>
#include <libc/ieee.h>

#define LOWER_CASE 0
#define UPPER_CASE 1

static const char *to_be_printed[30][2] = {
  /*
   *  Long double test.
   *  Normalized Finite.
   */
  {"0xf.123456789abcdefp+0", "-0XF.123456789ABCDEFP+0"},  /*  0: No precision given. Exact mantissa representation requested.  */
  {"0xf.123456789ab0000p+0", "-0XF.123456789AB0000P+0"},  /*  1: No precision given. Exact mantissa representation requested. Trailing zeros will be omitted.  */
  {"0xf.000000000000000p+0", "-0XF.000000000000000P+0"},  /*  2: No precision given. Exact mantissa representation requested. Trailing zeros will be omitted.  */
  {"0xf.123456789fp+0",      "-0XF.123456789FP+0"},       /*  3: 9 precision digits. Rounding to even.  */
  {"0xf.1234567891p+0",      "-0XF.1234567891P+0"},       /*  4: 9 precision digits. Rounding to even.  */
  {"0xf.1234567898p+0",      "-0XF.1234567898P+0"},       /*  5: 9 precision digits. Rounding to even.  */
  {"0xf.123456788p+0",       "-0XF.123456788P+0"},        /*  6: 8 precision digits. Rounding to even.  */
  {"0xf.f00000000000000p+0", "-0XF.F00000000000000P+0"},  /*  7: 0 precision digits. Rounding to even.  */
  {"0xf.100000000000000p+0", "-0XF.100000000000000P+0"},  /*  8: 0 precision digits. Rounding to even.  */
  {"0x8.fedcba987654321p+0", "-0X8.FEDCBA987654321P+0"},  /*  9: 0 precision digits. Alternate flag given.  */
  {"0xf.123456789abcdefp+0", "-0XF.123456789ABCDEFP+0"},  /* 10: 6 precision digits and 20 digits of field width.  */
  {"0xf.123456789abcdefp+0", "-0XF.123456789ABCDEFP+0"},  /* 11: 6 precision digits and 20 digits of field width and zero padding.  */
  {"0xf.123456789abcdefp+0", "-0XF.123456789ABCDEFP+0"},  /* 12: 6 precision digits and 20 digits of field width and zero padding and a sign.  */
  {"0xf.123456789abcdefp+0", "-0XF.123456789ABCDEFP+0"},  /* 13: 30 precision digits and 40 digits of field width and zero padding and a sign.  */

  /*
   *  Long double test.
   *  Denormalized Finite.
   *
   *  Long doubles as defined by intel's fpu.
   *  bias: 0x3fff
   *  Smallest licit exponent: -(bias - 1) = -16382
   *  Radix is 2.
   *  Mantissa of 64 bits with explicit integer bit.  Makes possible exact hex representation of mantissa.
   *  Shifting binary point by 3 places to the right allows to represent the integer part of the mantissa
   *  with one single hex digit. 
   */
  {"0x7.fffffffffffffffp-(16382 + 3)",  "-0X7.FFFFFFFFFFFFFFFP-(16382 + 3)"},  /* 14: No precision given. Exact mantissa representation requested.  */
  {"0x0.123456789abcdefp-(16382 + 3)",  "-0X0.123456789ABCDEFP-(16382 + 3)"},  /* 15: No precision given. Exact mantissa representation requested.  */
  {"0x0.800000000000000p-(16382 + 3)",  "-0X0.800000000000000P-(16382 + 3)"},  /* 16: No precision given. Exact mantissa representation requested. Trailing zeros will be omitted.  */
  {"0x0.000000012345678p-(16382 + 3)",  "-0X0.000000012345678P-(16382 + 3)"},  /* 17: 3 precision digits. Rounding to even.  */
  {"0x0.000000012345678p-(16382 + 3)",  "-0X0.000000012345678P-(16382 + 3)"},  /* 18: 12 precision digits. Rounding to even. Padding with 0.  */
  {"0x0.0000000f1000000p-(16382 + 3)",  "-0X0.0000000F1000000P-(16382 + 3)"},  /* 19: 0 precision digits. Rounding to even.  */
  {"0x0.0000000ff000000p-(16382 + 3)",  "-0X0.0000000FF000000P-(16382 + 3)"},  /* 20: 0 precision digits. Rounding to even.  */
  {"0x0.000000080000000p-(16382 + 3)",  "-0X0.000000080000000P-(16382 + 3)"},  /* 21: 0 precision digits. Alternate flag given.  */
  {"0x0.000000012345678p-(16382 + 3)",  "-0X0.000000012345678P-(16382 + 3)"},  /* 22: 6 precision digits and 20 digits of field width.  */
  {"0x0.000000012345678p-(16382 + 3)",  "-0X0.000000012345678P-(16382 + 3)"},  /* 23: 6 precision digits and 20 digits of field width and zero padding.  */
  {"+0x0.000000012345678p-(16382 + 3)", "-0X0.000000012345678P-(16382 + 3)"},  /* 24: 6 precision digits and 20 digits of field width and zero padding and a sign.  */
  {"+0x0.123456789abcdefp-(16382 + 3)", "-0X0.123456789ABCDEFP-(16382 + 3)"},  /* 25: 30 precision digits and 50 digits of field width and zero padding and a sign.  */

  /*
   *  Long double test.
   *  Zero, INF, NAN and Unnormal.
   */
  {"0x0.0p+0",      "-0X0.0P+0"},     /* 26: No precision given. Exact mantissa representation requested.  */
  {"infinity",      "-INFINITY"},     /* 27: No precision given. Exact mantissa representation requested.  */
  {"not a number",  "NOT A NUMBER"},  /* 28: No precision given. Exact mantissa representation requested.  */
  {"Unnormal",      "-Unnormal"}      /* 29: No precision given. Exact mantissa representation requested.  */
};

static const char *should_looks_like[30][2] = {
  /*
   *  Long double test.
   *  Normalized Finite.
   */
  {"0xf.123456789abcdefp+0", "-0XF.123456789ABCDEFP+0"},  /*  0: No precision given. Exact mantissa representation requested.  */
  {"0xf.123456789abp+0",     "-0XF.123456789ABP+0"},      /*  1: No precision given. Exact mantissa representation requested. Trailing zeros will be omitted.  */
  {"0xfp+0",                 "-0XFP+0"},                  /*  2: No precision given. Exact mantissa representation requested. Trailing zeros will be omitted.  */
  {"0xf.12345678ap+0",       "-0XF.12345678AP+0"},        /*  3: 9 precision digits. Rounding to even.  */
  {"0xf.123456789p+0",       "-0XF.123456789P+0"},        /*  4: 9 precision digits. Rounding to even.  */
  {"0xf.12345678ap+0",       "-0XF.12345678AP+0"},        /*  5: 9 precision digits. Rounding to even.  */
  {"0xf.12345678p+0",        "-0XF.12345678P+0"},         /*  6: 8 precision digits. Rounding to even.  */
  {"0x1p+4",                 "-0X1P+4"},                  /*  7: 0 precision digits. Rounding to even.  */
  {"0xfp+0",                 "-0XFP+0"},                  /*  8: 0 precision digits. Rounding to even.  */
  {"0x9.p+0",                "-0X9.P+0"},                 /*  9: 0 precision digits. Alternate flag given.  */
  {"       0xf.123456p+0",   "      -0XF.123456P+0"},     /* 10: 6 precision digits and 20 digits of field width.  */
  {"0x0000000f.123456p+0",   "-0X000000F.123456P+0"},     /* 11: 6 precision digits and 20 digits of field width and zero padding.  */
  {"+0x000000f.123456p+0",   "-0X000000F.123456P+0"},     /* 12: 6 precision digits and 20 digits of field width and zero padding and a sign.  */
  {"+0x00f.123456789abcdef000000000000000p+0",
   "-0X00F.123456789ABCDEF000000000000000P+0"},           /* 13: 30 precision digits and 40 digits of field width and zero padding and a sign.  */

  /*
   *  Long double test.
   *  Denormalized Finite.
   */
  {"0x7.fffffffffffffffp-16385",  "-0X7.FFFFFFFFFFFFFFFP-16385"},  /* 14: No precision given. Exact mantissa representation requested.  */
  {"0x1.23456789abcdefp-16389",   "-0X1.23456789ABCDEFP-16389"},   /* 15: No precision given. Exact mantissa representation requested.  */
  {"0x8p-16389",                  "-0X8P-16389"},                  /* 16: No precision given. Exact mantissa representation requested. Trailing zeros will be omitted.  */
  {"0x1.234p-16417",              "-0X1.234P-16417"},              /* 17: 3 precision digits. Rounding to even.  */
  {"0x1.234567800000p-16417",     "-0X1.234567800000P-16417"},     /* 18: 12 precision digits. Rounding to even. Padding with 0.  */
  {"0xfp-16417",                  "-0XFP-16417"},                  /* 19: 0 precision digits. Rounding to even.  */
  {"0x1p-16413",                  "-0X1P-16413"},                  /* 20: 0 precision digits. Rounding to even.  */
  {"0x8.p-16417",                 "-0X8.P-16417"},                 /* 21: 0 precision digits. Alternate flag given.  */
  {"   0x1.234568p-16417",   "  -0X1.234568P-16417"},              /* 22: 6 precision digits and 20 digits of field width.  */
  {"0x0001.234568p-16417",   "-0X001.234568P-16417"},              /* 23: 6 precision digits and 20 digits of field width and zero padding.  */
  {"+0x001.234568p-16417",   "-0X001.234568P-16417"},              /* 24: 6 precision digits and 20 digits of field width and zero padding and a sign.  */
  {"+0x000000001.23456789abcdef0000000000000000p-16389",
   "-0X000000001.23456789ABCDEF0000000000000000P-16389"},          /* 25: 30 precision digits and 50 digits of field width and zero padding and a sign.  */

  /*
   *  Long double test.
   *  Zero, INF, NAN and Unnormal.
   */
  {"0x0p+0",    "-0X0P+0"},    /* 26: No precision given. Exact mantissa representation requested.  */
  {"inf",       "-INF"},       /* 27: No precision given. Exact mantissa representation requested.  */
  {"nan",       "NAN"},        /* 28: No precision given. Exact mantissa representation requested.  */
  {"nan",       "NAN"},        /* 29: No precision given. Exact mantissa representation requested.  */
};

static long_double_t number[] = {
/*   mantissal    mantissah    exp             sgn  */
  {0x89ABCDEFU, 0xF1234567U, 0x3FFFU + 0x03U, 0x0U},  /*  0: 0xF.123456789ABCDEFp+0  */
  {0x89AB0000U, 0xF1234567U, 0x3FFFU + 0x03U, 0x0U},  /*  1: 0xF.123456700000000p+0  */
  {0x00000000U, 0xF0000000U, 0x3FFFU + 0x03U, 0x0U},  /*  2: 0xF.000000000000000p+0  */
  {0x89F00000U, 0xF1234567U, 0x3FFFU + 0x03U, 0x0U},  /*  3: 0xF.123456789Fp+0  */
  {0x89100000U, 0xF1234567U, 0x3FFFU + 0x03U, 0x0U},  /*  4: 0xF.1234567891p+0  */
  {0x89800000U, 0xF1234567U, 0x3FFFU + 0x03U, 0x0U},  /*  5: 0xF.1234567898p+0  */
  {0x88000000U, 0xF1234567U, 0x3FFFU + 0x03U, 0x0U},  /*  6: 0xF.123456788p+0  */
  {0x00000000U, 0xFF000000U, 0x3FFFU + 0x03U, 0x0U},  /*  7: 0xF.F00000000000000p+0  */
  {0x00000000U, 0xF1000000U, 0x3FFFU + 0x03U, 0x0U},  /*  8: 0xF.F00000000000000p+0  */
  {0x87654321U, 0x8FEDCBA9U, 0x3FFFU + 0x03U, 0x0U},  /*  9: 0x8.FEDCBA987654321p+0  */
  {0x89ABCDEFU, 0xF1234567U, 0x3FFFU + 0x03U, 0x0U},  /* 10: 0xF.123456789ABCDEFp+0  */
  {0x89ABCDEFU, 0xF1234567U, 0x3FFFU + 0x03U, 0x0U},  /* 11: 0xF.123456789ABCDEFp+0  */
  {0x89ABCDEFU, 0xF1234567U, 0x3FFFU + 0x03U, 0x0U},  /* 12: 0xF.123456789ABCDEFp+0  */
  {0x89ABCDEFU, 0xF1234567U, 0x3FFFU + 0x03U, 0x0U},  /* 13: 0xF.123456789ABCDEFp+0  */

  {0xFFFFFFFFU, 0x7FFFFFFFU, 0x0000U,         0x0U},  /* 14: 0x7.FFFFFFFFFFFFFFFp-16385  */
  {0x89ABCDEFU, 0x01234567U, 0x0000U,         0x0U},  /* 15: 0x0.123456789ABCDEFp-16385  */
  {0x00000000U, 0x08000000U, 0x0000U,         0x0U},  /* 16: 0x0.800000000000000p-16385  */
  {0x12345678U, 0x00000000U, 0x0000U,         0x0U},  /* 17: 0x0.000000012345678p-16385  */
  {0x12345678U, 0x00000000U, 0x0000U,         0x0U},  /* 18: 0x0.000000012345678p-16385  */
  {0xF1000000U, 0x00000000U, 0x0000U,         0x0U},  /* 19: 0x0.0000000F1000000p-16385  */
  {0xFF000000U, 0x00000000U, 0x0000U,         0x0U},  /* 20: 0x0.0000000FF000000p-16385  */
  {0x80000000U, 0x00000000U, 0x0000U,         0x0U},  /* 21: 0x0.000000080000000p-16385  */
  {0x12345678U, 0x00000000U, 0x0000U,         0x0U},  /* 22: 0x0.000000012345678p-16385  */
  {0x12345678U, 0x00000000U, 0x0000U,         0x0U},  /* 23: 0x0.000000012345678p-16385  */
  {0x12345678U, 0x00000000U, 0x0000U,         0x0U},  /* 24: 0x0.000000012345678p-16385  */
  {0x89ABCDEFU, 0x01234567U, 0x0000U,         0x0U},  /* 25: 0x0.123456789ABCDEFp-16385  */

  {0x00000000U, 0x00000000U, 0x0000U,         0x0U},  /* 26: 0x0.000000000000000p+0  */
  {0x00000000U, 0x80000000U, 0x7FFFU,         0x0U},  /* 27: INFINITY  */
  {0x00000000U, 0xC0000000U, 0x7FFFU,         0x0U},  /* 28: NAN  */
  {0x00000000U, 0x00000000U, 0x7FFFU,         0x0U},  /* 29: Unnormal  */
};



int
main (void)
{
  _longdouble_union_t value;
  unsigned int i = 0;

  printf("Testing normalized finite numbers.\n");
  printf("Printing value without specifying a precision.  Mantissa representation shall be exact.\n");
  printf("Test: %u\n", i);
  value.ldt = number[i];
  printf("value to be printed:    %s\n"
         "value printed with %%La: %La\n"
         "should be:              %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:    %s\n"
         "value printed with %%LA: %LA\n"
         "should be:              %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value without specifying a precision.\nMantissa representation shall be exact.\nTrailing zeros will always be omitted.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:    %s\n"
         "value printed with %%La: %La\n"
         "should be:              %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:    %s\n"
         "value printed with %%LA: %LA\n"
         "should be:              %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:    %s\n"
         "value printed with %%La: %La\n"
         "should be:              %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:    %s\n"
         "value printed with %%LA: %LA\n"
         "should be:              %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value with 9 digits of precision.  Rounding to even.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:      %s\n"
         "value printed with %%.9La: %.9La\n"
         "should be:                %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:      %s\n"
         "value printed with %%.9LA: %.9LA\n"
         "should be:                %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:      %s\n"
         "value printed with %%.9La: %.9La\n"
         "should be:                %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:      %s\n"
         "value printed with %%.9LA: %.9LA\n"
         "should be:                %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:      %s\n"
         "value printed with %%.9La: %.9La\n"
         "should be:                %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:      %s\n"
         "value printed with %%.9LA: %.9LA\n"
         "should be:                %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value with 8 digits of precision.  Rounding to even.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:      %s\n"
         "value printed with %%.8La: %.8La\n"
         "should be:                %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:      %s\n"
         "value printed with %%.8LA: %.8LA\n"
         "should be:                %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value with 0 digits of precision.  Rounding to even.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:      %s\n"
         "value printed with %%.0La: %.0La\n"
         "should be:                %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:      %s\n"
         "value printed with %%.0LA: %.0LA\n"
         "should be:                %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:      %s\n"
         "value printed with %%.0La: %.0La\n"
         "should be:                %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:      %s\n"
         "value printed with %%.0LA: %.0LA\n"
         "should be:                %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value with 0 digits of precision and alternate flag given.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:       %s\n"
         "value printed with %%#.0La: %#.0La\n"
         "should be:                 %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:       %s\n"
         "value printed with %%#.0LA: %#.0LA\n"
         "should be:                 %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value with 6 digits of precision and field width of 20 digits.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:        %s\n"
         "value printed with %%20.6La: %20.6La\n"
         "should be:                  %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:       %s\n"
         "value printed with %%20.6LA: %20.6LA\n"
         "should be:                  %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value with 6 digits of precision and field width of 20 digits and zero padding.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:         %s\n"
         "value printed with %%020.6La: %020.6La\n"
         "should be:                   %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:         %s\n"
         "value printed with %%020.6LA: %020.6LA\n"
         "should be:                   %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value with 6 digits of precision and field width of 20 digits and zero padding and a sign.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:           %s\n"
         "value printed with %%+020.6La: %+020.6La\n"
         "should be:                    %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:          %s\n"
         "value printed with %%+020.6LA: %+020.6LA\n"
         "should be:                    %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value with 30 digits of precision and field width of 40 digits and zero padding and a sign.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:            %s\n"
         "value printed with %%+040.30La: %+040.30La\n"
         "should be:                     %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:           %s\n"
         "value printed with %%+040.30LA: %+040.30LA\n"
         "should be:                     %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);




  printf("\n\nTesting denormalized finite numbers.  Smallest exponent: -16382.\nTo get a single nibble in the integer part of the mantissa\nthe exponent must be incremented by 3.\n");
  printf("Printing value without specifying a precision.  Mantissa representation shall be exact.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:    %s\n"
         "value printed with %%La: %La\n"
         "should be:              %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:    %s\n"
         "value printed with %%LA: %LA\n"
         "should be:              %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:    %s\n"
         "value printed with %%La: %La\n"
         "should be:              %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:    %s\n"
         "value printed with %%LA: %LA\n"
         "should be:              %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value without specifying a precision.  Mantissa representation shall be exact.\nTrailing zeros will always be omitted.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:    %s\n"
         "value printed with %%La: %La\n"
         "should be:              %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:    %s\n"
         "value printed with %%LA: %LA\n"
         "should be:              %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value with 3 digits of precision.  Rounding to even.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:      %s\n"
         "value printed with %%.3La: %.3La\n"
         "should be:                %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:      %s\n"
         "value printed with %%.3LA: %.3LA\n"
         "should be:                %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value with 12 digits of precision.  Rounding to even.\nPadding with 0 if precision is greater than number of significant digits in the mantissa.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:       %s\n"
         "value printed with %%.12La: %.12La\n"
         "should be:                 %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:       %s\n"
         "value printed with %%.12LA: %.12LA\n"
         "should be:                 %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value with 0 digits of precision.  Rounding to even.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:      %s\n"
         "value printed with %%.0La: %.0La\n"
         "should be:                %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:      %s\n"
         "value printed with %%.0LA: %.0LA\n"
         "should be:                %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:      %s\n"
         "value printed with %%.0La: %.0La\n"
         "should be:                %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:      %s\n"
         "value printed with %%.0LA: %.0LA\n"
         "should be:                %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value with 0 digits of precision and alternate flag given.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:       %s\n"
         "value printed with %%#.0La: %#.0La\n"
         "should be:                 %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:       %s\n"
         "value printed with %%#.0LA: %#.0LA\n"
         "should be:                 %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value with 6 digits of precision and field width of 20 digits.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:        %s\n"
         "value printed with %%20.6La: %20.6La\n"
         "should be:                  %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:       %s\n"
         "value printed with %%20.6LA: %20.6LA\n"
         "should be:                  %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value with 6 digits of precision and field width of 20 digits and zero padding.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:         %s\n"
         "value printed with %%020.6La: %020.6La\n"
         "should be:                   %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:         %s\n"
         "value printed with %%020.6LA: %020.6LA\n"
         "should be:                   %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value with 6 digits of precision and field width of 20 digits and zero padding and a sign.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:          %s\n"
         "value printed with %%+020.6La: %+020.6La\n"
         "should be:                    %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:          %s\n"
         "value printed with %%+020.6LA: %+020.6LA\n"
         "should be:                    %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  printf("\nPrinting value with 30 digits of precision and field width of 50 digits and zero padding and a sign.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:           %s\n"
         "value printed with %%+050.30La: %+050.30La\n"
         "should be:                     %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:           %s\n"
         "value printed with %%+050.30LA: %+050.30LA\n"
         "should be:                     %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);




  printf("\n\nTesting 0.0, NAN, INF and Unnormal.\n");
  printf("Printing value without specifying a precision.\n");
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:    %s\n"
         "value printed with %%La: %La\n"
         "should be:              %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:    %s\n"
         "value printed with %%LA: %LA\n"
         "should be:              %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);

  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:    %s\n"
         "value printed with %%La: %La\n"
         "should be:              %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:    %s\n"
         "value printed with %%LA: %LA\n"
         "should be:              %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:    %s\n"
         "value printed with %%La: %La\n"
         "should be:              %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:    %s\n"
         "value printed with %%LA: %LA\n"
         "should be:              %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);
  printf("Test: %u\n", ++i);
  value.ldt = number[i];
  printf("value to be printed:    %s\n"
         "value printed with %%La: %La\n"
         "should be:              %s\n\n", to_be_printed[i][LOWER_CASE], value.ld, should_looks_like[i][LOWER_CASE]);
  printf("value to be printed:    %s\n"
         "value printed with %%LA: %LA\n"
         "should be:              %s\n\n", to_be_printed[i][UPPER_CASE], -value.ld, should_looks_like[i][UPPER_CASE]);


  return(EXIT_SUCCESS);
}
