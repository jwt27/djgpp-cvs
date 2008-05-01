/*
 *  This program test flags, conversion modifiers and conversion specifiers
 *  of the printf familiy of function.  During compilation some warnings
 *  will be generated, this is part of the test and is intentional.  Thus
 *  the program can not be compiled with the stock makefile because this
 *  stops at warnings.  You must manually compile the program taking care
 *  that the library to be tested is used when linking.
 */

#include <stdio.h>
#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <libc/ieee.h>


void flags_test(FILE *out)
{
  fprintf(out, "Testing all flags.\n"
               "==================\n");
  fprintf(out, "Flag: \"'\"\n"
               "      The integer portion of the result of a decimal conversion (%%i, %%d, %%u, %%f,\n"
               "      %%F, %%g, or %%G) shall be formatted with thousands' grouping characters.\n"
               "      For other conversions the behavior is undefined. The non-monetary grouping\n"
               "      character is used.\n"
               "arg: %d   format string: \"%%'d\"   <%'d>\n"
               "arg: %d   format string: \"%%'i\"   <%'i>\n"
               "arg: %d   format string: \"%%'u\"   <%'u>\n"
               "arg: %.1f format string: \"%%'f\"   <%'f>\n"
               "arg: %.1f format string: \"%%'F\"   <%'F>\n"
               "arg: %.1f format string: \"%%'g\"   <%'g>\n"
               "arg: %.1f format string: \"%%'G\"   <%'G>\n\n",
               1, 1,
               1, 1,
               1, 1,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0);



  fprintf(out, "Flag: \"-\"\n"
               "      The result of the conversion shall be left-justified within the field. The\n"
               "      conversion is right-justified if this flag is not specified.\n"
               "arg: %d   format string: \"%%-5d\"   <%-5d>\n"
               "arg: %d   format string: \"%%5d\"    <%5d>\n"
               "arg: %d   format string: \"%%-5i\"   <%-5i>\n"
               "arg: %d   format string: \"%%5i\"    <%5i>\n"
               "arg: %d   format string: \"%%-5o\"   <%-5o>\n"
               "arg: %d   format string: \"%% 5o\"   <%5o>\n"
               "arg: %d   format string: \"%%-5u\"   <%-5u>\n"
               "arg: %d   format string: \"%%5u\"    <%5u>\n"
               "arg: %d  format string: \"%%-5x\"   <%-5x>\n"
               "arg: %d  format string: \"%%5X\"    <%5X>\n"
               "arg: %.1f format string: \"%%-10a\"  <%-10a>\n"
               "arg: %.1f format string: \"%%10A\"   <%10A>\n"
               "arg: %.1f format string: \"%%-15e\"  <%-15e>\n"
               "arg: %.1f format string: \"%%15E\"   <%15E>\n"
               "arg: %.1f format string: \"%%-10f\"  <%-10f>\n"
               "arg: %.1f format string: \"%%10F\"   <%10F>\n"
               "arg: %.1f format string: \"%%-10g\"  <%-10g>\n"
               "arg: %.1f format string: \"%%10G\"   <%10G>\n\n",
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               10, 10,
               15, 15,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0);



  fprintf(out, "Flag: \"+\"\n"
               "      The result of a signed conversion shall always begin with a sign ('+' or\n"
               "      '-'). The conversion shall begin with a sign only when a negative value\n"
               "      is converted if this flag is not specified.\n"
               "arg: %d   format string: \"%%+d\"   <%+d>\n"
               "arg: %d   format string: \"%%i\"    <%i>\n"
               "arg: %d   format string: \"%%+o\"   <%+o>    WARNING: Sign flag used with unsigned magnitude. Flag ignored.\n"
               "arg: %d   format string: \"%%+u\"   <%+u>    WARNING: Sign flag used with unsigned magnitude. Flag ignored.\n"
               "arg: %d  format string: \"%%+x\"   <%+x>    WARNING: Sign flag used with unsigned magnitude. Flag ignored.\n"
               "arg: %d  format string: \"%%+X\"   <%+X>    WARNING: Sign flag used with unsigned magnitude. Flag ignored.\n"
               "arg: %.1f format string: \"%%+a\"   <%+a>\n"
               "arg: %.1f format string: \"%%A\"    <%A>\n"
               "arg: %.1f format string: \"%%+e\"   <%+e>\n"
               "arg: %.1f format string: \"%%E\"    <%E>\n"
               "arg: %.1f format string: \"%%+f\"   <%+f>\n"
               "arg: %.1f format string: \"%%F\"    <%F>\n"
               "arg: %.1f format string: \"%%+g\"   <%+g>\n"
               "arg: %.1f format string: \"%%G\"    <%G>\n\n",
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               10, 10,
               15, 15,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0);



  fprintf(out, "Flag: \"<space>\"\n"
               "      If the first character of a signed conversion is not a sign or if a signed\n"
               "      conversion results in no characters, a <space> shall be prefixed to the\n"
               "      result. This means that if the <space> and '+' flags both appear, the\n"
               "      <space> flag shall be ignored.\n"
               "arg: %d   format string: \"%% i\"   <% i>\n"
               "arg: %d   format string: \"%%+ i\"  <%+ i>   WARNING: Sign and space flags used simultaneously. Space flag ignored.\n"
               "arg: %d   format string: \"%% d\"   <% d>\n"
               "arg: %d   format string: \"%% +d\"  <% +d>   WARNING: Sign and space flags used simultaneously. Space flag ignored.\n"
               "arg: %d   format string: \"%% o\"   <% o>    WARNING: Space flag used with unsigned magnitude. Flag ignored.\n"
               "arg: %d   format string: \"%%+ o\"  <%+ o>    WARNING: Sign and space flags used with unsigned magnitude. Flags ignored.\n"
               "arg: %d   format string: \"%% u\"   <% u>    WARNING: Space flag used with unsigned magnitude. Flag ignored.\n"
               "arg: %d   format string: \"%%+ u\"  <%+ u>    WARNING: Sign and space flags used with unsigned magnitude. Flags ignored.\n"
               "arg: %d  format string: \"%% x\"   <% x>    WARNING: Space flag used with unsigned magnitude. Flag ignored.\n"
               "arg: %d  format string: \"%%+ X\"  <%+ X>    WARNING: Sign and space flags used with unsigned magnitude. Flags ignored.\n"
               "arg: %.1f format string: \"%% a\"   <% a>\n"
               "arg: %.1f format string: \"%%+ A\"  <%+ A>   WARNING: Sign and space flags used simultaneously. Space flag ignored.\n"
               "arg: %.1f format string: \"%% e\"   <% e>\n"
               "arg: %.1f format string: \"%% +E\"  <% +E>   WARNING: Sign and space flags used simultaneously. Space flag ignored.\n"
               "arg: %.1f format string: \"%% f\"   <% f>\n"
               "arg: %.1f format string: \"%%+ F\"  <%+ F>   WARNING: Sign and space flags used simultaneously. Space flag ignored.\n"
               "arg: %.1f format string: \"%% g\"   <% g>\n"
               "arg: %.1f format string: \"%% +G\"  <% +G>   WARNING: Sign and space flags used simultaneously. Space flag ignored.\n\n",
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               10, 10,
               15, 15,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0);



  fprintf(out, "Flag: \"#\"\n"
               "      Specifies that the value is to be converted to an alternative form. For o\n"
               "      conversion, it increases the precision (if necessary) to force the first\n"
               "      digit of the result to be zero. For x or X conversion specifiers, a non-\n"
               "      zero result shall have 0x (or 0X) prefixed to it. For a, A, e, E, f, F, g,\n"
               "      and G conversion specifiers, the result shall always contain a radix\n"
               "      character, even if no digits follow the radix character. Without this\n"
               "      flag, a radix character appears in the result of these conversions only if\n"
               "      a digit follows it. For g and G conversion specifiers, trailing zeros\n"
               "      shall not be removed from the result as they normally are. For other\n"
               "      conversion specifiers, the behavior is undefined.\n"
               "arg: %d   format string: \"%%#d\"   <%#d>   WARNING: # flag used with decimal magnitude. Flag ignored.\n"
               "arg: %d   format string: \"%%#i\"   <%#i>   WARNING: # flag used with decimal magnitude. Flag ignored.\n"
               "arg: %d   format string: \"%%#u\"   <%#u>   WARNING: # flag used with decimal magnitude. Flag ignored.\n"
               "arg: %d   format string: \"%%#o\"   <%#o>\n"
               "arg: %d   format string: \"%%#o\"   <%#o>\n"
               "arg: %d   format string: \"%%#x\"   <%#x>\n"
               "arg: %d   format string: \"%%#X\"   <%#X>\n"
               "arg: %.1f format string: \"%%a\"    <%a>\n"
               "arg: %.1f format string: \"%%#A\"   <%#A>\n"
               "arg: %.1f format string: \"%%1.e\"  <%1.e>\n"
               "arg: %.1f format string: \"%%#1.E\" <%#1.E>\n"
               "arg: %.1f format string: \"%%1.f\"  <%1.f>\n"
               "arg: %.1f format string: \"%%#1.F\" <%#1.F>\n"
               "arg: %.1f format string: \"%%g\"    <%g>\n"
               "arg: %.1f format string: \"%%#G\"   <%#G>\n\n",
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               0, 0,
               1, 1,
               0, 0,
               1.0, 1.0,
               0.0, 0.0,
               1.0, 1.0,
               0.0, 0.0,
               1.0, 1.0,
               0.0, 0.0,
               1.0, 1.0,
               0.0, 0.0);



  fprintf(out, "Flag: \"0\"\n"
               "      For d, i, o, u, x, X, a, A, e, E, f, F, g, and G conversion specifiers,\n"
               "      leading zeros (following any indication of sign or base) are used to pad\n"
               "      to the field width; no space padding is performed. If the '0' and '-'\n"
               "      flags both appear, the '0' flag is ignored. For d, i, o, u, x, and X\n"
               "      conversion specifiers, if a precision is specified, the '0' flag is\n"
               "      ignored. If the '0' and ''' flags both appear, the grouping characters are\n"
               "      inserted before zero padding. For other conversions, the behavior is\n"
               "      undefined.\n"
               "arg: %d    format string: \"%%05d\"    <%05d>\n"
               "arg: %d    format string: \"%%-05d\"   <%-05d>            WARNING: - and 0 flags used simultaneously. 0 flag ignored.\n"
               "arg: %d    format string: \"%%05.d\"   <%05.d>\n"
               "arg: %d    format string: \"%%05i\"    <%05i>\n"
               "arg: %d    format string: \"%%-05i\"   <%-05i>            WARNING: - and 0 flags used simultaneously. 0 flag ignored.\n"
               "arg: %d    format string: \"%%05.i\"   <%05.i>\n"
               "arg: %d    format string: \"%%05o\"    <%05o>\n"
               "arg: %d    format string: \"%%-05o\"   <%-05o>            WARNING: - and 0 flags used simultaneously. 0 flag ignored.\n"
               "arg: %d    format string: \"%%05.o\"   <%05.o>\n"
               "arg: %d    format string: \"%%05u\"    <%05u>\n"
               "arg: %d    format string: \"%%-05u\"   <%-05u>            WARNING: - and 0 flags used simultaneously. 0 flag ignored.\n"
               "arg: %d    format string: \"%%05.u\"   <%05.u>\n"
               "arg: %d   format string: \"%%05x\"    <%05x>\n"
               "arg: %d   format string: \"%%-05x\"   <%-05x>             WARNING: - and 0 flags used simultaneously. 0 flag ignored.\n"
               "arg: %d   format string: \"%%05.x\"   <%05.x>\n"
               "arg: %d   format string: \"%%05X\"    <%05X>\n"
               "arg: %d   format string: \"%%-05X\"   <%-05X>             WARNING: - and 0 flags used simultaneously. 0 flag ignored.\n"
               "arg: %d   format string: \"%%05.X\"   <%05.X>\n"
               "arg: %.1f  format string: \"%%0-10a\"  <%0-10a>        WARNING: - and 0 flags used simultaneously. 0 flag ignored.\n"
               "arg: %.1f  format string: \"%%0+10A\"  <%0+10A>\n"
               "arg: %.1f  format string: \"%%010A\"   <%010A>\n"
               "arg: %.1f  format string: \"%%0-15e\"  <%0-15e>   WARNING: - and 0 flags used simultaneously. 0 flag ignored.\n"
               "arg: %.1f  format string: \"%%0+15E\"  <%0+15E>\n"
               "arg: %.1f  format string: \"%%015E\"   <%015E>\n"
               "arg: %.1f  format string: \"%%0-10f\"  <%0-10f>        WARNING: - and 0 flags used simultaneously. 0 flag ignored.\n"
               "arg: %.1f  format string: \"%%0+10F\"  <%0+10F>\n"
               "arg: %.1f  format string: \"%%010F\"   <%010F>\n"
               "arg: %.1f  format string: \"%%0-10g\"  <%0-10g>        WARNING: - and 0 flags used simultaneously. 0 flag ignored.\n"
               "arg: %.1f  format string: \"%%0+10G\"  <%0+10G>\n"
               "arg: %.1f  format string: \"%%010G\"   <%010G>\n\n",
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               1, 1,
               10, 10,
               10, 10,
               10, 10,
               15, 15,
               15, 15,
               15, 15,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0,
               1.0, 1.0);
}


void  length_modifiers_test(FILE *out)
{
  char cv, *pcv;
  short int siv, *psiv;
  long int liv, *pliv;
  long long int lliv, *plliv;
  intmax_t jiv, *pjiv;
  size_t ziv, *pziv;
  ptrdiff_t piv, *ppiv;
  pcv = &cv;
  psiv = &siv;
  pliv = &liv;
  plliv = &lliv;
  pjiv = &jiv;
  pziv = &ziv;
  ppiv = &piv;


  fprintf(out, "\n\nTesting all length modifiers.\n"
               "=============================\n");
  fprintf(out, "Length modifier: \"hh\"\n"
               "                 Specifies that a following d, i, o, u, x, or X conversion\n"
               "                 specifier applies to a signed char or unsigned char argument\n"
               "                 (the argument will have been promoted according to the integer\n"
               "                 promotions, but its value shall be converted to signed char or\n"
               "                 unsigned char before printing); or that a following n conversion\n"
               "                 specifier applies to a pointer to a signed char argument.\n"
               "arg: CHAR_MAX    format string: \"%%hhd\"   <%hhd>\n"
               "arg: CHAR_MIN    format string: \"%%hhi\"   <%hhi>\n"
               "arg: UCHAR_MAX   format string: \"%%hho\"   <%hho>\n"
               "arg: UCHAR_MAX   format string: \"%%hhu\"   <%hhu>\n"
               "arg: CHAR_MIN    format string: \"%%hhx\"   <%hhx>\n"
               "arg: CHAR_MAX    format string: \"%%hhX\"   <%hhX>\n"
               "arg: -           format string: \"%%hhn\"   (pointer to cv)%hhn\n",
               CHAR_MAX,
               CHAR_MIN,
               UCHAR_MAX,
               UCHAR_MAX ,
               CHAR_MIN,
               CHAR_MAX,
               pcv);
  fprintf(out, "cv=%i\n\n", *pcv);



  fprintf(out, "Length modifier: \"h\"\n"
               "                 Specifies that a following d, i, o, u, x, or X conversion\n"
               "                 specifier applies to a short or unsigned short argument (the\n"
               "                 argument will have been promoted according to the integer\n"
               "                 promotions, but its value shall be converted to short or\n"
               "                 unsigned short before printing); or that a following n\n"
               "                 conversion specifier applies to a pointer to a short argument.\n"
               "arg: SHRT_MAX    format string: \"%%hd\"   <%hd>\n"
               "arg: SHRT_MIN    format string: \"%%hi\"   <%hi>\n"
               "arg: USHRT_MAX   format string: \"%%ho\"   <%ho>\n"
               "arg: USHRT_MAX   format string: \"%%hu\"   <%hu>\n"
               "arg: SHRT_MIN    format string: \"%%hx\"   <%hx>\n"
               "arg: SHRT_MAX    format string: \"%%hX\"   <%hX>\n"
               "arg: -           format string: \"%%hn\"   (pointer to siv)%hn\n",
               SHRT_MAX,
               SHRT_MIN,
               USHRT_MAX,
               USHRT_MAX,
               SHRT_MIN,
               SHRT_MAX,
               psiv);
  fprintf(out, "siv=%i\n\n", *psiv);



  fprintf(out, "Length modifier: \"l\"\n"
               "                 Specifies that a following d, i, o, u, x, or X conversion\n"
               "                 specifier applies to a long or unsigned long argument; that a\n"
               "                 following n conversion specifier applies to a pointer to a long\n"
               "                 argument; that a following c conversion specifier applies to a\n"
               "                 wint_t argument; that a following s conversion specifier\n"
               "                 applies to a pointer to a wchar_t argument; or has no effect on\n"
               "                 a following a, A, e, E, f, F, g, or G conversion specifier.\n"
               "arg: LONG_MAX    format string: \"%%ld\"   <%ld>\n"
               "arg: LONG_MIN    format string: \"%%li\"   <%li>\n"
               "arg: ULONG_MAX   format string: \"%%lo\"   <%lo>\n"
               "arg: ULONG_MAX   format string: \"%%lu\"   <%lu>\n"
               "arg: LONG_MIN    format string: \"%%lx\"   <%lx>\n"
               "arg: LONG_MAX    format string: \"%%lX\"   <%lX>\n"
               "arg: -           format string: \"%%ln\"   (pointer to liv)%ln\n"
               "arg: %.1f         format string: \"%%la\"   <%la>\n"
               "arg: %.1f         format string: \"%%A\"    <%A>\n"
               "arg: %.1f         format string: \"%%le\"   <%le>\n"
               "arg: %.1f         format string: \"%%E\"    <%E>\n"
               "arg: %.1f         format string: \"%%lf\"   <%lf>\n"
               "arg: %.1f         format string: \"%%F\"    <%F>\n"
               "arg: %.1f         format string: \"%%lg\"   <%lg>\n"
               "arg: %.1f         format string: \"%%G\"    <%G>\n",
               LONG_MAX,
               LONG_MIN,
               ULONG_MAX,
               ULONG_MAX,
               LONG_MIN,
               LONG_MAX,
               pliv,
               1.1, 1.1,
               1.1, 1.1,
               2.2, 2.2,
               2.2, 2.2,
               3.3, 3.3,
               3.3, 3.3,
               4.4, 4.4,
               4.4, 4.4);
  fprintf(out, "liv=%li\n\n", *pliv);



  fprintf(out, "Length modifier: \"ll\"\n"
               "                 Specifies that a following d, i, o, u, x, or X conversion\n"
               "                 specifier applies to a long long or unsigned long long argument;\n"
               "                 or that a following n conversion specifier applies to a pointer\n"
               "                 to a long long argument.\n"
               "arg: LONG_LONG_MAX   format string: \"%%lld\"   <%lld>\n"
               "arg: LONG_LONG_MIN   format string: \"%%lli\"   <%lli>\n"
               "arg: ULONG_LONG_MAX  format string: \"%%llo\"   <%llo>\n"
               "arg: ULONG_LONG_MAX  format string: \"%%llu\"   <%llu>\n"
               "arg: LONG_LONG_MIN   format string: \"%%llx\"   <%llx>\n"
               "arg: LONG_LONG_MAX   format string: \"%%llX\"   <%llX>\n"
               "arg: -               format string: \"%%lln\"   (pointer to lliv)%lln\n",
               LONG_LONG_MAX,
               LONG_LONG_MIN,
               ULONG_LONG_MAX,
               ULONG_LONG_MAX,
               LONG_LONG_MIN,
               LONG_LONG_MAX,
               plliv);
  fprintf(out, "lliv=%lli\n\n", *plliv);



  fprintf(out, "Length modifier: \"j\"\n"
               "                 Specifies that a following d, i, o, u, x, or X conversion\n"
               "                 specifier applies to an intmax_t or uintmax_t argument; or that\n"
               "                 a following n conversion specifier applies to a pointer to an\n"
               "                 intmax_t argument.\n"
               "arg: LONG_LONG_MAX   format string: \"%%jd\"   <%jd>\n"
               "arg: LONG_LONG_MIN   format string: \"%%ji\"   <%ji>\n"
               "arg: ULONG_LONG_MAX  format string: \"%%jo\"   <%jo>\n"
               "arg: ULONG_LONG_MAX  format string: \"%%ju\"   <%ju>\n"
               "arg: LONG_LONG_MIN   format string: \"%%jx\"   <%jx>\n"
               "arg: LONG_LONG_MAX   format string: \"%%jX\"   <%jX>\n"
               "arg: -               format string: \"%%jn\"   (pointer to jiv)%jn\n",
               (intmax_t)LONG_LONG_MAX,
               (intmax_t)LONG_LONG_MIN,
               (uintmax_t)ULONG_LONG_MAX,
               (uintmax_t)ULONG_LONG_MAX,
               (intmax_t)LONG_LONG_MIN,
               (intmax_t)LONG_LONG_MAX,
               pjiv);
  fprintf(out, "jiv=%ji\n\n", *pjiv);



  fprintf(out, "Length modifier: \"z\"\n"
               "                 Specifies that a following d, i, o, u, x, or X conversion\n"
               "                 specifier applies to a size_t or the corresponding signed\n"
               "                 integer type argument; or that a following n conversion\n"
               "                 specifier applies to a pointer to a signed integer type\n"
               "                 corresponding to a size_t argument.\n"
               "arg: LONG_MAX    format string: \"%%zd\"   <%zd>\n"
               "arg: LONG_MIN    format string: \"%%zi\"   <%zi>\n"
               "arg: LONG_MAX    format string: \"%%zo\"   <%zo>\n"
               "arg: LONG_MAX    format string: \"%%zu\"   <%zu>\n"
               "arg: LONG_MIN    format string: \"%%zx\"   <%zx>\n"
               "arg: LONG_MAX    format string: \"%%zX\"   <%zX>\n"
               "arg: -           format string: \"%%zn\"   (pointer to ziv)%zn\n",
               (size_t)LONG_MAX,
               (size_t)LONG_MIN,
               (size_t)LONG_MAX,
               (size_t)LONG_MAX,
               (size_t)LONG_MIN,
               (size_t)LONG_MAX,
               pziv);
  fprintf(out, "ziv=%zi\n\n", *pziv);



  fprintf(out, "Length modifier: \"t\"\n"
               "                 Specifies that a following d, i, o, u, x, or X conversion\n"
               "                 specifier applies to a ptrdiff_t or the corresponding\n"
               "                 unsigned type argument; or that a following n conversion\n"
               "                 specifier applies to a pointer to a ptrdiff_t argument.\n"
               "arg: LONG_MAX    format string: \"%%td\"   <%td>\n"
               "arg: LONG_MIN    format string: \"%%ti\"   <%ti>\n"
               "arg: LONG_MAX    format string: \"%%to\"   <%to>\n"
               "arg: ULONG_MAX   format string: \"%%tu\"   <%tu>\n"
               "arg: LONG_MIN    format string: \"%%tx\"   <%tx>\n"
               "arg: LONG_MAX    format string: \"%%tX\"   <%tX>\n"
               "arg: -           format string: \"%%tn\"   (pointer to piv)%tn\n",
               (ptrdiff_t)LONG_MAX,
               (ptrdiff_t)LONG_MIN,
               (ptrdiff_t)LONG_MAX,
               (ptrdiff_t)ULONG_MAX,
               (ptrdiff_t)LONG_MIN,
               (ptrdiff_t)LONG_MAX,
               ppiv);
  fprintf(out, "piv=%ti\n\n", *ppiv);



  fprintf(out, "Length modifier: \"L\"\n"
               "---------1---------2---------3---------4---------5---------6---------7---------8\n"
               "                 Specifies that a following a, A, e, E, f, F, g, or G conversion\n"
               "                 specifier applies to a long double argument.\n"
               "arg: LDBL_MAX    format string: \"%%La\"   <%La>\n"
               "arg: LDBL_MIN    format string: \"%%LA\"   <%LA>\n"
               "arg: LDBL_MAX    format string: \"%%Le\"   <%Le>\n"
               "arg: LDBL_MIN    format string: \"%%LE\"   <%LE>\n"
               "arg: LDBL_MAX    format string: \"%%Lf\"   <%Lf>\n"
               "arg: LDBL_MIN    format string: \"%%LF\"   <%LF>\n"
               "arg: LDBL_MAX    format string: \"%%Lg\"   <%Lg>\n"
               "arg: LDBL_MIN    format string: \"%%LG\"   <%LG>\n\n",
               LDBL_MAX,
               LDBL_MIN,
               LDBL_MAX,
               LDBL_MIN,
               LDBL_MAX,
               LDBL_MIN,
               LDBL_MAX,
               LDBL_MIN);
}


void numeric_conversion_specifiers_test(FILE *out)
{
  int i, width, precision;
  double darg[] = {1.1, 2.2, 3.3};
  char *title[] = {"En castellano:", "Auf deutsch:", "In english:"};
  char *format[] = {
   "%5$s, %2$d de %1$s, %3$*6$.*7$d:%4$*6$.*7$d\n",
   "%5$s, %2$d. %1$s, %3$*6$.*7$d:%4$*6$.*7$d\n",
   "%5$s, %1$s %2$d, %3$*6$.*7$d:%4$*6$.*7$d\n"
  };
  char *weekday[] = {"Sabado", "Samstag", "Saturday"};
  char *month[] = {"febrero", "Februar", "february"};
  int day = 2;
  int hour = 12;
  int min = 34;


  fprintf(out, "\n\nTesting numeric conversion specifiers.\n"
               "======================================\n");

  width = 10;
  precision = 1;
  fprintf(out, "Printing a sequence of numbers using a given field width and precision\n"
               "accessing the variables a multiple times in different order.\n"
               "The sequence of arguments after the format string is:\n"
               "  width, precision, darg[0], darg[1], darg[2]\n"
               "with the values:\n"
               "  width:     %d\n"
               "  precision: %d\n"
               "  darg[0]:   %f\n"
               "  darg[1]:   %f\n"
               "  darg[2]:   %f\n",
               width, precision, darg[0], darg[1], darg[2]);
  fprintf(out, "Format string: \"%%3$-*1$.*2$f###%%4$*1$.*2$f###%%5$*1$.*2$f\"     <%3$-*1$.*2$f###%4$*1$.*2$f###%5$*1$.*2$f>\n"
               "Printing again but accessing the arguments in inverse order:\n"
               "Format string: \"%%5$-*1$.*2$f###%%4$*1$.*2$f###%%3$*1$.*2$f\"     <%5$-*1$.*2$f###%4$*1$.*2$f###%3$*1$.*2$f>\n\n\n",
               width, precision, darg[0], darg[1], darg[2]);


  width = 2;
  precision = 2;
  fprintf(out, "Printing Language-Independent Date and Time.\n\n");
  for (i = 0; i < 3; i++)
  {
    int len = strlen(format[i]);
    fprintf(out, "%s  ", title[i]);
    fprintf(out, format[i], month[i], day, hour, min, weekday[i], width, precision);
    format[i][--len] = '\0';
    fprintf(out, "Produced with:\n"
                 "  printf(\"%1$s\\n\", month[%2$i], day, hour, min, weekday[%2$i], width, precision);\n\n",
                 format[i], i);
  }
}


void printf_test(FILE *out)
{
  char buf[100], *strbuf;
  union {
    unsigned int word[3];
    long double value;
  } x;
  int strlng;



  fprintf(out, "\n\nGeneral test of the printf familiy of functions.\n"
               "================================================\n");

  fprintf(out, "Infinity.\n");
  sprintf(buf, "%La", 1.0L / 0.0L);
  fprintf(out, "fmt str: \"%%La\"   arg: 1.0L / 0.0L\n"
               "Shall return: \"inf\"    returns: \"%s\"\n", buf);
  sprintf(buf, "%LA", -1.0L / 0.0L);
  fprintf(out, "fmt str: \"%%LA\"   arg: -1.0L / 0.0L\n"
               "Shall return: \"-INF\"   returns: \"%s\"\n", buf);
  sprintf(buf, "%Lf", 1.0L / 0.0L);
  fprintf(out, "fmt str: \"%%Lf\"   arg: 1.0L / 0.0L\n"
               "Shall return: \"inf\"    returns: \"%s\"\n", buf);
  sprintf(buf, "%LF", -1.0L / 0.0L);
  fprintf(out, "fmt str: \"%%LF\"   arg: -1.0L / 0.0L\n"
               "Shall return: \"-INF\"   returns: \"%s\"\n", buf);
  sprintf(buf, "%Le", 1.0L / 0.0L);
  fprintf(out, "fmt str: \"%%Le\"   arg: 1.0L / 0.0L\n"
               "Shall return: \"inf\"    returns: \"%s\"\n", buf);
  sprintf(buf, "%LE", -1.0L / 0.0L);
  fprintf(out, "fmt str: \"%%LE\"   arg: -1.0L / 0.0L\n"
               "Shall return: \"-INF\"   returns: \"%s\"\n", buf);
  sprintf(buf, "%Lg", 1.0L / 0.0L);
  fprintf(out, "fmt str: \"%%Lg\"   arg: 1.0L / 0.0L\n"
               "Shall return: \"inf\"    returns: \"%s\"\n", buf);
  sprintf(buf, "%LG", -1.0L / 0.0L);
  fprintf(out, "fmt str: \"%%LG\"   arg: -1.0L / 0.0L\n"
               "Shall return: \"-INF\"   returns: \"%s\"\n\n", buf);

  fprintf(out, "NaN.\n");
  sprintf(buf, "%La", 0.0L / 0.0L);
  fprintf(out, "fmt str: \"%%La\"   arg: 0.0L / 0.0L\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", buf);
  sprintf(buf, "%LA", -0.0L / 0.0L);
  fprintf(out, "fmt str: \"%%LA\"   arg: -0.0L / 0.0L\n"
               "Shall return: \"NAN\"   returns: \"%s\"\n", buf);
  sprintf(buf, "%Lf", 0.0L / 0.0L);
  fprintf(out, "fmt str: \"%%Lf\"   arg: 0.0L / 0.0L\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", buf);
  sprintf(buf, "%LF", -0.0L / 0.0L);
  fprintf(out, "fmt str: \"%%LF\"   arg: -0.0L / 0.0L\n"
               "Shall return: \"NAN\"   returns: \"%s\"\n", buf);
  sprintf(buf, "%Le", 0.0L / 0.0L);
  fprintf(out, "fmt str: \"%%Le\"   arg: 0.0L / 0.0L\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", buf);
  sprintf(buf, "%LE", -0.0L / 0.0L);
  fprintf(out, "fmt str: \"%%LE\"   arg: -0.0L / 0.0L\n"
               "Shall return: \"NAN\"   returns: \"%s\"\n", buf);
  sprintf(buf, "%Lg", 0.0L / 0.0L);
  fprintf(out, "fmt str: \"%%Lg\"   arg: 0.0L / 0.0L\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", buf);
  sprintf(buf, "%LG", -0.0L / 0.0L);
  fprintf(out, "fmt str: \"%%LG\"   arg: -0.0L / 0.0L\n"
               "Shall return: \"NAN\"   returns: \"%s\"\n\n", buf);


  x.word[2] = 0x7FFF;
  x.word[1] = 0xC000000C;
  x.word[0] = 0x10000001;
  fprintf(out, "Quiet NaN\n");
  sprintf(buf, "%La", x.value);
  fprintf(out, "fmt str: \"%%La\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Le", x.value);
  fprintf(out, "fmt str: \"%%Le\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Lf", x.value);
  fprintf(out, "fmt str: \"%%Lf\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Lg", x.value);
  fprintf(out, "fmt str: \"%%Lg\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n\n", x.word[2], x.word[1], x.word[0], buf);
  x.word[2] = 0x7FFF;
  x.word[1] = 0x80000008;
  x.word[0] = 0x10000001;
  fprintf(out, "Signalling NaN\n");
  sprintf(buf, "%La", x.value);
  fprintf(out, "fmt str: \"%%La\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Le", x.value);
  fprintf(out, "fmt str: \"%%Le\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Lf", x.value);
  fprintf(out, "fmt str: \"%%Lf\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Lg", x.value);
  fprintf(out, "fmt str: \"%%Lg\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n\n", x.word[2], x.word[1], x.word[0], buf);
  x.word[2] = 0x7FFF;
  x.word[1] = 0x40000004;
  x.word[0] = 0x10000001;
  fprintf(out, "Pseudo-NaN\n");
  sprintf(buf, "%La", x.value);
  fprintf(out, "fmt str: \"%%La\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Le", x.value);
  fprintf(out, "fmt str: \"%%Le\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Lf", x.value);
  fprintf(out, "fmt str: \"%%Lf\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Lg", x.value);
  fprintf(out, "fmt str: \"%%Lg\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n\n", x.word[2], x.word[1], x.word[0], buf);
  x.word[2] = 0xFFFF;
  x.word[1] = 0x00000000;
  x.word[0] = 0x00000000;
  fprintf(out, "Pseudo-Infinity\n");
  sprintf(buf, "%La", x.value);
  fprintf(out, "fmt str: \"%%La\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Le", x.value);
  fprintf(out, "fmt str: \"%%Le\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Lf", x.value);
  fprintf(out, "fmt str: \"%%Lf\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Lg", x.value);
  fprintf(out, "fmt str: \"%%Lg\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n\n", x.word[2], x.word[1], x.word[0], buf);
  x.word[2] = 0x4004;
  x.word[1] = 0x00000000;
  x.word[0] = 0x00000000;
  fprintf(out, "Pseudo-Zero\n");
  sprintf(buf, "%La", x.value);
  fprintf(out, "fmt str: \"%%La\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Le", x.value);
  fprintf(out, "fmt str: \"%%Le\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Lf", x.value);
  fprintf(out, "fmt str: \"%%Lf\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Lg", x.value);
  fprintf(out, "fmt str: \"%%Lg\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n\n", x.word[2], x.word[1], x.word[0], buf);
  x.word[2] = 0x0440;
  x.word[1] = 0x60000006;
  x.word[0] = 0x10000001;
  fprintf(out, "Unnormalized number\n");
  sprintf(buf, "%La", x.value);
  fprintf(out, "fmt str: \"%%La\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Le", x.value);
  fprintf(out, "fmt str: \"%%Le\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Lf", x.value);
  fprintf(out, "fmt str: \"%%Lf\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Lg", x.value);
  fprintf(out, "fmt str: \"%%Lg\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n\n", x.word[2], x.word[1], x.word[0], buf);
  x.word[2] = 0x0000;
  x.word[1] = 0x80000008;
  x.word[0] = 0x10000001;
  fprintf(out, "Pseudo-Denormal\n");
  sprintf(buf, "%La", x.value);
  fprintf(out, "fmt str: \"%%La\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Le", x.value);
  fprintf(out, "fmt str: \"%%Le\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Lf", x.value);
  fprintf(out, "fmt str: \"%%Lf\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n", x.word[2], x.word[1], x.word[0], buf);
  sprintf(buf, "%Lg", x.value);
  fprintf(out, "fmt str: \"%%Lg\"   exp: %#.4x  manthi: %#.8x  mantlo: %#.8x\n"
               "Shall return: \"nan\"    returns: \"%s\"\n\n", x.word[2], x.word[1], x.word[0], buf);


  fprintf(out, "Testing asprintf.\n");
  fprintf(out, "Code line:   strlng = asprintf(&strbuf, \"Pi = %%.15Lf\", 3.1415926535897932384626433832795L);\n");
  strlng = asprintf(&strbuf, "Pi = %.15Lf", 3.1415926535897932384626433832795L);
  fprintf(out, "Result:      strbuf: \"%s\"   strlng: %d\n", strbuf, strlng);
  free(strbuf);

  fprintf(out, "Testing asnprintf.\n");
  strbuf = NULL;
  fprintf(out, "Code line:   strlng = asnprintf(&strbuf, 0, \"Pi = %%.15Lf\", 3.1415926535897932384626433832795L);\n");
  strlng = asnprintf(&strbuf, 0, "Pi = %.15Lf", 3.1415926535897932384626433832795L);
  fprintf(out, "Result:      strbuf: %s  strlng: %d\n", strbuf, strlng);
  fprintf(out, "Code line:   strlng = asnprintf(&strbuf, 10, \"Pi = %%.15Lf\", 3.1415926535897932384626433832795L);\n");
  strlng = asnprintf(&strbuf, 10, "Pi = %.15Lf", 3.1415926535897932384626433832795L);
  fprintf(out, "Result:      strbuf: 0x%p  strlng: %d\n", strbuf, strlng);
  fprintf(out, "             strbuf: \"%s\"  mallocated buffer length is %zd chars long plus 1 nul char\n\n", strbuf, strlen(strbuf));
  free(strbuf);

  fprintf(out, "Testing flags in combination with Infinity and NaN.\n");
  fprintf(out, "Code line:   sprintf(buf, \"%%0*Lf\", 10, 1.0L / 0.0L);\n");
  sprintf(buf, "%0*Lf", 10, 1.0L / 0.0L);
  asprintf(&strbuf, "%*s", (int)strlen(buf), "inf");
  fprintf(out, "Shall return: <%s>    returns: <%s>  %s\n", strbuf, buf, strcmp(strbuf, buf) ? "Not OK" : "OK");
  free(strbuf);
  fprintf(out, "Code line:   sprintf(buf, \"%%+*Lf\", 10, 1.0L / 0.0L);\n");
  sprintf(buf, "%+*Lf", 10, 1.0L / 0.0L);
  asprintf(&strbuf, "%*s", (int)strlen(buf), "+inf");
  fprintf(out, "Shall return: <%s>    returns: <%s>  %s\n", strbuf, buf, strcmp(strbuf, buf) ? "Not OK" : "OK");
  free(strbuf);
  fprintf(out, "Code line:   sprintf(buf, \"%%-*Lf\", 10, 1.0L / 0.0L);\n");
  sprintf(buf, "%-*Lf", 10, 1.0L / 0.0L);
  asprintf(&strbuf, "%*s", (int)strlen(buf), "inf       ");
  fprintf(out, "Shall return: <%s>    returns: <%s>  %s\n", strbuf, buf, strcmp(strbuf, buf) ? "Not OK" : "OK");
  free(strbuf);
  fprintf(out, "Code line:   sprintf(buf, \"%% *Lf\", 10, 1.0L / 0.0L);\n");
  sprintf(buf, "% *Lf", 10, 1.0L / 0.0L);
  asprintf(&strbuf, "%*s", (int)strlen(buf), "inf");
  fprintf(out, "Shall return: <%s>    returns: <%s>  %s\n", strbuf, buf, strcmp(strbuf, buf) ? "Not OK" : "OK");
  free(strbuf);
  fprintf(out, "Code line:   sprintf(buf, \"%%#*Lf\", 10, 1.0L / 0.0L);\n");
  sprintf(buf, "%#*Lf", 10, 1.0L / 0.0L);
  asprintf(&strbuf, "%*s", (int)strlen(buf), "inf");
  fprintf(out, "Shall return: <%s>    returns: <%s>  %s\n", strbuf, buf, strcmp(strbuf, buf) ? "Not OK" : "OK");
  free(strbuf);
  fprintf(out, "Code line:   sprintf(buf, \"%%0*Lf\", 10, 0.0L / 0.0L);\n");
  sprintf(buf, "%0*Lf", 10, 0.0L / 0.0L);
  asprintf(&strbuf, "%*s", (int)strlen(buf), "nan");
  fprintf(out, "Shall return: <%s>    returns: <%s>  %s\n", strbuf, buf, strcmp(strbuf, buf) ? "Not OK" : "OK");
  free(strbuf);
  fprintf(out, "Code line:   sprintf(buf, \"%%+*Lf\", 10, 0.0L / 0.0L);\n");
  sprintf(buf, "%+*Lf", 10, 0.0L / 0.0L);
  asprintf(&strbuf, "%*s", (int)strlen(buf), "+nan");
  fprintf(out, "Shall return: <%s>    returns: <%s>  %s\n", strbuf, buf, strcmp(strbuf, buf) ? "Not OK" : "OK");
  free(strbuf);
  fprintf(out, "Code line:   sprintf(buf, \"%%-*Lf\", 10, 0.0L / 0.0L);\n");
  sprintf(buf, "%-*Lf", 10, 0.0L / 0.0L);
  asprintf(&strbuf, "%*s", (int)strlen(buf), "nan       ");
  fprintf(out, "Shall return: <%s>    returns: <%s>  %s\n", strbuf, buf, strcmp(strbuf, buf) ? "Not OK" : "OK");
  free(strbuf);
  fprintf(out, "Code line:   sprintf(buf, \"%% *Lf\", 10, 0.0L / 0.0L);\n");
  sprintf(buf, "% *Lf", 10, 0.0L / 0.0L);
  asprintf(&strbuf, "%*s", (int)strlen(buf), "nan");
  fprintf(out, "Shall return: <%s>    returns: <%s>  %s\n", strbuf, buf, strcmp(strbuf, buf) ? "Not OK" : "OK");
  free(strbuf);
  fprintf(out, "Code line:   sprintf(buf, \"%%#*Lf\", 10, 0.0L / 0.0L);\n");
  sprintf(buf, "%#*Lf", 10, 0.0L / 0.0L);
  asprintf(&strbuf, "%*s", (int)strlen(buf), "nan");
  fprintf(out, "Shall return: <%s>    returns: <%s>  %s\n", strbuf, buf, strcmp(strbuf, buf) ? "Not OK" : "OK");
  free(strbuf);
}


int main(void)
{
  FILE *out;

  out = fopen("printf5.txt", "w");
  if (out == NULL)
  {
    printf("Can not open test.txt.  Test failed.\n");
    return 1;
  }

  printf("Testing:\n"
         "  printf family of functions...\n");
  flags_test(out);
  length_modifiers_test(out);
  numeric_conversion_specifiers_test(out);
  printf_test(out);
  fclose(out);

  printf("The test output is in printf5.txt\n");
  return 0;
}
