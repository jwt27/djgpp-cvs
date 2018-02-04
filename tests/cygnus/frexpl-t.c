/* Copyright (C) 2018 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2017 DJ Delorie, see COPYING.DJ for details */


#include "main-t.h"

typedef struct {
  const _longdouble_union_t value;      /* test value */
  const _longdouble_union_t mantissa;   /* result */
  const int exponent;                   /* result */
} entry_t;

static const entry_t tests_long_double[] =
{
  /* test value */
  /*     value                              mantissa                                 exponent   */

  /* Zeros. */
  {{.ldt = {0x0U, 0x0U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x00000000U, 0x0000U, 0}}, 0}, /* 0.0 */
  {{.ldt = {0x0U, 0x0U, 0x0U, 1}},  {.ldt = {0x00000000U, 0x00000000U, 0x0000U, 1}}, 0}, /* -0.0 */

  /* Subnormals aka denormals. */
  {{.ldt = {0x00000001U, 0x0U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, -16444}, /* Very small number. */
  {{.ldt = {0x00000008U, 0x0U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, -16441}, /* Very small number. */
  {{.ldt = {0x00000080U, 0x0U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, -16437}, /* Very small number. */
  {{.ldt = {0x00000800U, 0x0U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, -16433}, /* Very small number. */
  {{.ldt = {0x00008000U, 0x0U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, -16429}, /* Very small number. */
  {{.ldt = {0x00080000U, 0x0U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, -16425}, /* Very small number. */
  {{.ldt = {0x00800000U, 0x0U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, -16421}, /* Very small number. */
  {{.ldt = {0x08000000U, 0x0U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, -16417}, /* Very small number. */
  {{.ldt = {0x80000000U, 0x0U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, -16413}, /* Very small number. */
  {{.ldt = {0x0U, 0x00000008U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, -16409}, /* Very small number. */
  {{.ldt = {0x0U, 0x00000080U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, -16405}, /* Very small number. */
  {{.ldt = {0x0U, 0x00000800U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, -16401}, /* Very small number. */
  {{.ldt = {0x0U, 0x00008000U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, -16397}, /* Very small number. */
  {{.ldt = {0x0U, 0x00080000U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, -16393}, /* Very small number. */
  {{.ldt = {0x0U, 0x00800000U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, -16389}, /* Very small number. */
  {{.ldt = {0x0U, 0x08000000U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, -16385}, /* Very small number. */
  {{.ldt = {0x0U, 0x40000000U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, -16382}, /* Very small number. */

  {{.ldt = {0x00000001U, 0x0U, 0x0U, 1}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 1}}, -16444}, /* Very small number. */
  {{.ldt = {0x00000007U, 0x0U, 0x0U, 1}},  {.ldt = {0x00000000U, 0xE0000000U, 0x3FFEU, 1}}, -16442}, /* Very small number. */
  {{.ldt = {0x00000070U, 0x0U, 0x0U, 1}},  {.ldt = {0x00000000U, 0xE0000000U, 0x3FFEU, 1}}, -16438}, /* Very small number. */
  {{.ldt = {0x00000700U, 0x0U, 0x0U, 1}},  {.ldt = {0x00000000U, 0xE0000000U, 0x3FFEU, 1}}, -16434}, /* Very small number. */
  {{.ldt = {0x00007000U, 0x0U, 0x0U, 1}},  {.ldt = {0x00000000U, 0xE0000000U, 0x3FFEU, 1}}, -16430}, /* Very small number. */
  {{.ldt = {0x00070000U, 0x0U, 0x0U, 1}},  {.ldt = {0x00000000U, 0xE0000000U, 0x3FFEU, 1}}, -16426}, /* Very small number. */
  {{.ldt = {0x00700000U, 0x0U, 0x0U, 1}},  {.ldt = {0x00000000U, 0xE0000000U, 0x3FFEU, 1}}, -16422}, /* Very small number. */
  {{.ldt = {0x07000000U, 0x0U, 0x0U, 1}},  {.ldt = {0x00000000U, 0xE0000000U, 0x3FFEU, 1}}, -16418}, /* Very small number. */
  {{.ldt = {0x70000000U, 0x0U, 0x0U, 1}},  {.ldt = {0x00000000U, 0xE0000000U, 0x3FFEU, 1}}, -16414}, /* Very small number. */
  {{.ldt = {0x0U, 0x00000007U, 0x0U, 1}},  {.ldt = {0x00000000U, 0xE0000000U, 0x3FFEU, 1}}, -16410}, /* Very small number. */
  {{.ldt = {0x0U, 0x00000070U, 0x0U, 1}},  {.ldt = {0x00000000U, 0xE0000000U, 0x3FFEU, 1}}, -16406}, /* Very small number. */
  {{.ldt = {0x0U, 0x00000700U, 0x0U, 1}},  {.ldt = {0x00000000U, 0xE0000000U, 0x3FFEU, 1}}, -16402}, /* Very small number. */
  {{.ldt = {0x0U, 0x00007000U, 0x0U, 1}},  {.ldt = {0x00000000U, 0xE0000000U, 0x3FFEU, 1}}, -16398}, /* Very small number. */
  {{.ldt = {0x0U, 0x00070000U, 0x0U, 1}},  {.ldt = {0x00000000U, 0xE0000000U, 0x3FFEU, 1}}, -16394}, /* Very small number. */
  {{.ldt = {0x0U, 0x00700000U, 0x0U, 1}},  {.ldt = {0x00000000U, 0xE0000000U, 0x3FFEU, 1}}, -16390}, /* Very small number. */
  {{.ldt = {0x0U, 0x07000000U, 0x0U, 1}},  {.ldt = {0x00000000U, 0xE0000000U, 0x3FFEU, 1}}, -16386}, /* Very small number. */
  {{.ldt = {0x0U, 0x70000000U, 0x0U, 1}},  {.ldt = {0x00000000U, 0xE0000000U, 0x3FFEU, 1}}, -16382}, /* Very small number. */

  /* Pseudo denormals. */
  {{.ldt = {0x00000001U, 0x80000000U, 0x0U, 0}},  {.ldt = {0x00000001U, 0x80000000U, 0x3FFEU, 0}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000008U, 0x80000000U, 0x0U, 0}},  {.ldt = {0x00000008U, 0x80000000U, 0x3FFEU, 0}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000080U, 0x80000000U, 0x0U, 0}},  {.ldt = {0x00000080U, 0x80000000U, 0x3FFEU, 0}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000800U, 0x80000000U, 0x0U, 0}},  {.ldt = {0x00000800U, 0x80000000U, 0x3FFEU, 0}}, -16381}, /* Very small number. */
  {{.ldt = {0x00008000U, 0x80000000U, 0x0U, 0}},  {.ldt = {0x00008000U, 0x80000000U, 0x3FFEU, 0}}, -16381}, /* Very small number. */
  {{.ldt = {0x00080000U, 0x80000000U, 0x0U, 0}},  {.ldt = {0x00080000U, 0x80000000U, 0x3FFEU, 0}}, -16381}, /* Very small number. */
  {{.ldt = {0x00800000U, 0x80000000U, 0x0U, 0}},  {.ldt = {0x00800000U, 0x80000000U, 0x3FFEU, 0}}, -16381}, /* Very small number. */
  {{.ldt = {0x08000000U, 0x80000000U, 0x0U, 0}},  {.ldt = {0x08000000U, 0x80000000U, 0x3FFEU, 0}}, -16381}, /* Very small number. */
  {{.ldt = {0x80000000U, 0x80000000U, 0x0U, 0}},  {.ldt = {0x80000000U, 0x80000000U, 0x3FFEU, 0}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000000U, 0x80000008U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000008U, 0x3FFEU, 0}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000000U, 0x80000080U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000080U, 0x3FFEU, 0}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000000U, 0x80000800U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80000800U, 0x3FFEU, 0}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000000U, 0x80008000U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80008000U, 0x3FFEU, 0}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000000U, 0x80080000U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80080000U, 0x3FFEU, 0}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000000U, 0x80800000U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x80800000U, 0x3FFEU, 0}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000000U, 0x88000000U, 0x0U, 0}},  {.ldt = {0x00000000U, 0x88000000U, 0x3FFEU, 0}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000000U, 0xC0000000U, 0x0U, 0}},  {.ldt = {0x00000000U, 0xC0000000U, 0x3FFEU, 0}}, -16381}, /* Very small number. */

  {{.ldt = {0x00000001U, 0x80000000U, 0x0U, 1}},  {.ldt = {0x00000001U, 0x80000000U, 0x3FFEU, 1}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000007U, 0x80000000U, 0x0U, 1}},  {.ldt = {0x00000007U, 0x80000000U, 0x3FFEU, 1}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000070U, 0x80000000U, 0x0U, 1}},  {.ldt = {0x00000070U, 0x80000000U, 0x3FFEU, 1}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000700U, 0x80000000U, 0x0U, 1}},  {.ldt = {0x00000700U, 0x80000000U, 0x3FFEU, 1}}, -16381}, /* Very small number. */
  {{.ldt = {0x00007000U, 0x80000000U, 0x0U, 1}},  {.ldt = {0x00007000U, 0x80000000U, 0x3FFEU, 1}}, -16381}, /* Very small number. */
  {{.ldt = {0x00070000U, 0x80000000U, 0x0U, 1}},  {.ldt = {0x00070000U, 0x80000000U, 0x3FFEU, 1}}, -16381}, /* Very small number. */
  {{.ldt = {0x00700000U, 0x80000000U, 0x0U, 1}},  {.ldt = {0x00700000U, 0x80000000U, 0x3FFEU, 1}}, -16381}, /* Very small number. */
  {{.ldt = {0x07000000U, 0x80000000U, 0x0U, 1}},  {.ldt = {0x07000000U, 0x80000000U, 0x3FFEU, 1}}, -16381}, /* Very small number. */
  {{.ldt = {0x70000000U, 0x80000000U, 0x0U, 1}},  {.ldt = {0x70000000U, 0x80000000U, 0x3FFEU, 1}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000000U, 0x80000007U, 0x0U, 1}},  {.ldt = {0x00000000U, 0x80000007U, 0x3FFEU, 1}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000000U, 0x80000070U, 0x0U, 1}},  {.ldt = {0x00000000U, 0x80000070U, 0x3FFEU, 1}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000000U, 0x80000700U, 0x0U, 1}},  {.ldt = {0x00000000U, 0x80000700U, 0x3FFEU, 1}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000000U, 0x80007000U, 0x0U, 1}},  {.ldt = {0x00000000U, 0x80007000U, 0x3FFEU, 1}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000000U, 0x80070000U, 0x0U, 1}},  {.ldt = {0x00000000U, 0x80070000U, 0x3FFEU, 1}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000000U, 0x80700000U, 0x0U, 1}},  {.ldt = {0x00000000U, 0x80700000U, 0x3FFEU, 1}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000000U, 0x87000000U, 0x0U, 1}},  {.ldt = {0x00000000U, 0x87000000U, 0x3FFEU, 1}}, -16381}, /* Very small number. */
  {{.ldt = {0x00000000U, 0xF0000000U, 0x0U, 1}},  {.ldt = {0x00000000U, 0xF0000000U, 0x3FFEU, 1}}, -16381}, /* Very small number. */

  /* Normals. */
  {{.ldt = {0x0U, 0x80000000U, 0x1U, 0}},  {.ldt = {00000000U, 0X80000000U, 0X3FFEU, 0}}, -16381}, /* Small number. */
  {{.ldt = {0x0U, 0x80000000U, 0x1U, 1}},  {.ldt = {00000000U, 0X80000000U, 0X3FFEU, 1}}, -16381}, /* Small -number. */
  {{.ldt = {0xFFFFFFFFU, 0xFFFFFFFFU, 0x7FFEU, 0}},  {.ldt = {0XFFFFFFFFU, 0XFFFFFFFFU, 0X3FFEU, 0}}, 16384}, /* Big number. */
  {{.ldt = {0xFFFFFFFFU, 0xFFFFFFFFU, 0x7FFEU, 1}},  {.ldt = {0XFFFFFFFFU, 0XFFFFFFFFU, 0X3FFEU, 1}}, 16384}, /* Big -number. */

  /* Infs. */
  {{.ldt = {0x0U, 0x80000000U, 0x7FFFU, 0}},  {.ldt = {0x0U, 0x80000000U, 0x7FFFU, 0}}, 0}, /* Inf */
  {{.ldt = {0x0U, 0x80000000U, 0x7FFFU, 1}},  {.ldt = {0x0U, 0x80000000U, 0x7FFFU, 1}}, 0}, /* -Inf */

  /* NaNs. */
  {{.ldt = {0x1U, 0x80000000U, 0x7FFFU, 0}},  {.ldt = {0x1U, 0x80000000U, 0x7FFFU, 0}}, 0}, /* SNaN */
  {{.ldt = {0x1U, 0x80000000U, 0x7FFFU, 1}},  {.ldt = {0x1U, 0x80000000U, 0x7FFFU, 0}}, 0}, /* -SNaN */
  {{.ldt = {0x0U, 0xFFFFFFFFU, 0x7FFFU, 0}},  {.ldt = {0x0U, 0xFFFFFFFFU, 0x7FFFU, 0}}, 0}, /* QNaN */
  {{.ldt = {0x0U, 0xFFFFFFFFU, 0x7FFFU, 1}},  {.ldt = {0x0U, 0xFFFFFFFFU, 0x7FFFU, 0}}, 0}, /* -QNaN */

  /* Number. */
  {{.ldt = {0x2168C000U, 0xC90FDAA2U, 0x3FFFU + 0x0001U, 0}},  {.ldt = {0x2168C000U, 0xC90FDAA2U, 0x3FFEU, 0}}, 2}, /* PI */
  {{.ldt = {0x2168C000U, 0xC90FDAA2U, 0x3FFFU + 0x0001U, 1}},  {.ldt = {0x2168C000U, 0xC90FDAA2U, 0x3FFEU, 1}}, 2}, /* -PI */


  {{.ldt = {0x00000000U, 0xF0000000U, 0x3FFFU + 0x0U, 0}},  {.ldt = {0x00000000U, 0xF0000000U, 0x3FFEU, 0}}, 1}, /* 1.875000 */
  {{.ldt = {0x00000000U, 0xF0000000U, 0x3FFFU + 0x0U, 1}},  {.ldt = {0x00000000U, 0xF0000000U, 0x3FFEU, 1}}, 1}, /* -1.875000 */
  {{.ldt = {0x00000000U, 0xD0000000U, 0x3FFFU + 0x0U, 0}},  {.ldt = {0x00000000U, 0xD0000000U, 0x3FFEU, 0}}, 1}, /* 1.625000 */
  {{.ldt = {0x00000000U, 0xD0000000U, 0x3FFFU + 0x0U, 1}},  {.ldt = {0x00000000U, 0xD0000000U, 0x3FFEU, 1}}, 1}, /* -1.625000 */
  {{.ldt = {0xF7A0B800U, 0xC00010C6U, 0x3FFFU + 0x0U, 0}},  {.ldt = {0xF7A0B800U, 0xC00010C6U, 0x3FFEU, 0}}, 1}, /* 1.500002 */
  {{.ldt = {0xF7A0B800U, 0xC00010C6U, 0x3FFFU + 0x0U, 1}},  {.ldt = {0xF7A0B800U, 0xC00010C6U, 0x3FFEU, 1}}, 1}, /* -1.500002 */
  {{.ldt = {0x00000000U, 0xC0000000U, 0x3FFFU + 0x0U, 0}},  {.ldt = {0x00000000U, 0xC0000000U, 0x3FFEU, 0}}, 1}, /* 1.500000 */
  {{.ldt = {0x00000000U, 0xC0000000U, 0x3FFFU + 0x0U, 1}},  {.ldt = {0x00000000U, 0xC0000000U, 0x3FFEU, 1}}, 1}, /* -1.500000 */
  {{.ldt = {0x085F4800U, 0xBFFFEF39U, 0x3FFFU + 0x0U, 0}},  {.ldt = {0x085F4800U, 0xBFFFEF39U, 0x3FFEU, 0}}, 1}, /* 1.499998 */
  {{.ldt = {0x085F4800U, 0xBFFFEF39U, 0x3FFFU + 0x0U, 1}},  {.ldt = {0x085F4800U, 0xBFFFEF39U, 0x3FFEU, 1}}, 1}, /* -1.499998 */
  {{.ldt = {0x00000000U, 0xB0000000U, 0x3FFFU + 0x0U, 0}},  {.ldt = {0x00000000U, 0xB0000000U, 0x3FFEU, 0}}, 1}, /* 1.375000 */
  {{.ldt = {0x00000000U, 0xB0000000U, 0x3FFFU + 0x0U, 1}},  {.ldt = {0x00000000U, 0xB0000000U, 0x3FFEU, 1}}, 1}, /* -1.375000 */
  {{.ldt = {0x00000000U, 0x90000000U, 0x3FFFU + 0x0U, 0}},  {.ldt = {0x00000000U, 0x90000000U, 0x3FFEU, 0}}, 1}, /* 1.125000 */
  {{.ldt = {0x00000000U, 0x90000000U, 0x3FFFU + 0x0U, 1}},  {.ldt = {0x00000000U, 0x90000000U, 0x3FFEU, 1}}, 1}, /* -1.125000 */

  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0016U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, 23}, /* 4194304.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0016U, 1}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 1}}, 23}, /* -4194304.000000 */ 
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0017U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, 24}, /* 8388608.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0017U, 1}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 1}}, 24}, /* -8388608.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0018U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, 25}, /* 16777216.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0018U, 1}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 1}}, 25}, /* -16777216.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x001EU, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, 31}, /* 1073741824.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x001EU, 1}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 1}}, 31}, /* -1073741824.000000 */
  {{.ldt = {0x00000000U, 0xFFFFFFFEU, 0x3FFFU + 0x001EU, 0}},  {.ldt = {0x00000000U, 0xFFFFFFFEU, 0x3FFEU, 0}}, 31}, /* 2147483647.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x001FU, 1}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 1}}, 32}, /* -2147483648.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0020U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, 33}, /* 4294967296.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0020U, 1}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 1}}, 33}, /* -4294967296.000000 */

  {{.ldt = {0x89A00000U, 0x81234567U, 0x3FFFU + 0x0033U, 0}},  {.ldt = {0x89A00000U, 0x81234567U, 0x3FFEU, 0}}, 52}, /* 2271815812028928.000000 */
  {{.ldt = {0x89A00000U, 0x81234567U, 0x3FFFU + 0x0033U, 1}},  {.ldt = {0x89A00000U, 0x81234567U, 0x3FFEU, 1}}, 52}, /* -2271815812028928.000000 */
  {{.ldt = {0xC4D58000U, 0x8091A2B3U, 0x3FFFU + 0x0034U, 0}},  {.ldt = {0xC4D58000U, 0x8091A2B3U, 0x3FFEU, 0}}, 53}, /* 4523615625714352.000000 */
  {{.ldt = {0xC4D58000U, 0x8091A2B3U, 0x3FFFU + 0x0034U, 1}},  {.ldt = {0xC4D58000U, 0x8091A2B3U, 0x3FFEU, 1}}, 53}, /* -4523615625714352.000000 */
  {{.ldt = {0x3B2A1800U, 0xFF6E5D4CU, 0x3FFFU + 0x0034U, 0}},  {.ldt = {0x3B2A1800U, 0xFF6E5D4CU, 0x3FFEU, 0}}, 53}, /* 8987183256397123.000000 */
  {{.ldt = {0x3B2A1800U, 0xFF6E5D4CU, 0x3FFFU + 0x0034U, 1}},  {.ldt = {0x3B2A1800U, 0xFF6E5D4CU, 0x3FFEU, 1}}, 53}, /* -8987183256397123.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0035U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, 54}, /* 9007199254740992.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0035U, 1}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 1}}, 54}, /* -9007199254740992.000000 */

  {{.ldt = {0x4D5E7000U, 0x891A2B3CU, 0x3FFFU + 0x0035U, 0}},  {.ldt = {0x4D5E7000U, 0x891A2B3CU, 0x3FFEU, 0}}, 54}, /* 9647711201744796.000000 */
  {{.ldt = {0x4D5E7000U, 0x891A2B3CU, 0x3FFFU + 0x0035U, 1}},  {.ldt = {0x4D5E7000U, 0x891A2B3CU, 0x3FFEU, 1}}, 54}, /* -9647711201744796.000000 */
  {{.ldt = {0x76543000U, 0xFEDCBA98U, 0x3FFFU + 0x0041U, 0}},  {.ldt = {0x76543000U, 0xFEDCBA98U, 0x3FFEU, 0}}, 66}, /* 73459034177972256768.000000 */
  {{.ldt = {0x76543000U, 0xFEDCBA98U, 0x3FFFU + 0x0041U, 1}},  {.ldt = {0x76543000U, 0xFEDCBA98U, 0x3FFEU, 1}}, 66}, /* -73459034177972256768.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x003FU, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, 64}, /* 9223372036854775808.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x003FU, 1}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 1}}, 64}, /* -9223372036854775808.000000 */
  {{.ldt = {0x00000000U, 0xC0000000U, 0x3FFFU + 0x003FU, 0}},  {.ldt = {0x00000000U, 0xC0000000U, 0x3FFEU, 0}}, 64}, /* 13835058055282163712.000000 */
  {{.ldt = {0x00000000U, 0xC0000000U, 0x3FFFU + 0x003FU, 1}},  {.ldt = {0x00000000U, 0xC0000000U, 0x3FFEU, 1}}, 64}, /* -13835058055282163712.000000 */
  {{.ldt = {0x00000000U, 0xF0000000U, 0x3FFFU + 0x0042U, 0}},  {.ldt = {0x00000000U, 0xF0000000U, 0x3FFEU, 0}}, 67}, /* 138350580552821637120.000000 */
  {{.ldt = {0x00000000U, 0xF0000000U, 0x3FFFU + 0x0042U, 1}},  {.ldt = {0x00000000U, 0xF0000000U, 0x3FFEU, 1}}, 67}, /* -138350580552821637120.000000 */
  {{.ldt = {0xBA987800U, 0xF7FFFEDCU, 0x3FFFU + 0x0042U, 0}},  {.ldt = {0xBA987800U, 0xF7FFFEDCU, 0x3FFEU, 0}}, 67}, /* 142962256563249856512.000000 */
  {{.ldt = {0xBA987800U, 0xF7FFFEDCU, 0x3FFFU + 0x0042U, 1}},  {.ldt = {0xBA987800U, 0xF7FFFEDCU, 0x3FFEU, 1}}, 67}, /* -142962256563249856512.000000 */
  {{.ldt = {0x00000000U, 0xF8000000U, 0x3FFFU + 0x0042U, 0}},  {.ldt = {0x00000000U, 0xF8000000U, 0x3FFEU, 0}}, 67}, /* 142962266571249025024.000000 */
  {{.ldt = {0x00000000U, 0xF8000000U, 0x3FFFU + 0x0042U, 1}},  {.ldt = {0x00000000U, 0xF8000000U, 0x3FFEU, 1}}, 67}, /* -142962266571249025024.000000 */
  {{.ldt = {0x00012000U, 0xF8000000U, 0x3FFFU + 0x0042U, 0}},  {.ldt = {0x00012000U, 0xF8000000U, 0x3FFEU, 0}}, 67}, /* 142962266571249614848.000000 */
  {{.ldt = {0x00012000U, 0xF8000000U, 0x3FFFU + 0x0042U, 1}},  {.ldt = {0x00012000U, 0xF8000000U, 0x3FFEU, 1}}, 67}, /* -142962266571249614848.000000 */
  {{.ldt = {0x76543000U, 0xFEDCBA98U, 0x3FFFU + 0x0042U, 0}},  {.ldt = {0x76543000U, 0xFEDCBA98U, 0x3FFEU, 0}}, 67}, /* 146918068355944513536.000000 */
  {{.ldt = {0x76543000U, 0xFEDCBA98U, 0x3FFFU + 0x0042U, 1}},  {.ldt = {0x76543000U, 0xFEDCBA98U, 0x3FFEU, 1}}, 67}, /* 147573952589676396544.000000 */
  {{.ldt = {0xFFFFF800U, 0xFFFFFFFFU, 0x3FFFU + 0x0042U, 0}},  {.ldt = {0xFFFFF800U, 0xFFFFFFFFU, 0x3FFEU, 0}}, 67}, /* -147573952589676396544.000000 */
  {{.ldt = {0xFFFFF800U, 0xFFFFFFFFU, 0x3FFFU + 0x0042U, 1}},  {.ldt = {0xFFFFF800U, 0xFFFFFFFFU, 0x3FFEU, 1}}, 67}, /* -147573952589676396544.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0040U, 0}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 0}}, 65}, /* 18446744073709551616.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0040U, 1}},  {.ldt = {0x00000000U, 0x80000000U, 0x3FFEU, 1}}, 65},  /* -18446744073709551616.000000 */

  /* Number. */
  {{.ldt = {0x80000F0CU, 0xFFFFFFFFU, 0x3FFFU + 0x001FU, 0}},  {.ldt = {0x80000F0CU, 0xFFFFFFFFU, 0x3FFEU, 0}}, 32}, /* 4294967295.500001 */
  {{.ldt = {0x8FFFFF00U, 0xFFFFFFFEU, 0x3FFFU + 0x001FU, 0}},  {.ldt = {0x8FFFFF00U, 0xFFFFFFFEU, 0x3FFEU, 0}}, 32}, /* 4294967294.562500 */
  {{.ldt = {0x8000AA00U, 0xFFFFFFFEU, 0x3FFFU + 0x001FU, 0}},  {.ldt = {0x8000AA00U, 0xFFFFFFFEU, 0x3FFEU, 0}}, 32}, /* 4294967294.500010 */
  {{.ldt = {0x707CD001U, 0xFFFFFFFEU, 0x3FFFU + 0x001FU, 0}},  {.ldt = {0x707CD001U, 0xFFFFFFFEU, 0x3FFEU, 0}}, 32}, /* 4294967294.439404 */
  {{.ldt = {0x7FFEB00DU, 0xFFFFFFFEU, 0x3FFFU + 0x001FU, 0}},  {.ldt = {0x7FFEB00DU, 0xFFFFFFFEU, 0x3FFEU, 0}}, 32}, /* 4294967294.499980 */
  {{.ldt = {0x90123007U, 0xFFFFFFFFU, 0x3FFFU + 0x001FU, 0}},  {.ldt = {0x90123007U, 0xFFFFFFFFU, 0x3FFEU, 0}}, 32}, /* 4294967295.562778 */

  {{.ldt = {0x0000FFFFU, 0xFFFFFFFFU, 0x3FFFU + 0x003EU, 0}},  {.ldt = {0x0000FFFFU, 0xFFFFFFFFU, 0x3FFEU, 0}}, 63}, /* 9223372034707324927.500000 */
  {{.ldt = {0x7777777FU, 0xFFFFFFFEU, 0x3FFFU + 0x003EU, 0}},  {.ldt = {0x7777777FU, 0xFFFFFFFEU, 0x3FFEU, 0}}, 63}, /* 9223372033561967551.500000 */
  {{.ldt = {0x9EEEEEE0U, 0xFFFFFFFEU, 0x3FFFU + 0x003EU, 0}},  {.ldt = {0x9EEEEEE0U, 0xFFFFFFFEU, 0x3FFEU, 0}}, 63}, /* 9223372033893037936.000000 */
  {{.ldt = {0xEEEEEE07U, 0xFFFFFFFEU, 0x3FFFU + 0x003EU, 0}},  {.ldt = {0xEEEEEE07U, 0xFFFFFFFEU, 0x3FFEU, 0}}, 63}, /* 9223372034564126467.500000 */
  {{.ldt = {0xEEEEEE0EU, 0xFFFFFFFEU, 0x3FFFU + 0x003EU, 0}},  {.ldt = {0xEEEEEE0EU, 0xFFFFFFFEU, 0x3FFEU, 0}}, 63}, /* 9223372034564126471.000000 */
  {{.ldt = {0xFFFFFF0FU, 0xFFFFFFFFU, 0x3FFFU + 0x003EU, 0}},  {.ldt = {0xFFFFFF0FU, 0xFFFFFFFFU, 0x3FFEU, 0}}, 63}, /* 9223372036854775687.500000 */

  {{.ldt = {0xAAAAAAAAU, 0xAAAAAAAAU, 0x3FFFU + 0x1AABU, 0}},  {.ldt = {0xAAAAAAAAU, 0xAAAAAAAAU, 0x3FFEU, 0}}, 6828}, /* 1.806005774585392e+2055 */
  {{.ldt = {0xCCCCCCCCU, 0xCCCCCCCCU, 0x3FFFU + 0x2CCDU, 0}},  {.ldt = {0xCCCCCCCCU, 0xCCCCCCCCU, 0x3FFEU, 0}}, 11470}, /* 5.213630550621597e+3452 */
  {{.ldt = {0xEEEEEEEEU, 0xEEEEEEEEU, 0x3FFFU + 0x3EEFU, 0}},  {.ldt = {0xEEEEEEEEU, 0xEEEEEEEEU, 0x3FFEU, 0}}, 16112} /* 1.463278241057685e+4850 */
};

static const size_t n_tests_long_double = sizeof(tests_long_double) / sizeof(tests_long_double[0]);

#undef  IS_EQUAL
#define IS_EQUAL(a, b) (((a).ldt.sign == (b).ldt.sign) && ((a).ldt.exponent == (b).ldt.exponent) && ((a).ldt.mantissah == (b).ldt.mantissah) && ((a).ldt.mantissal == (b).ldt.mantissal))
#define IS_EQUAL_NAN(a, b) ((((a).ldt.sign == (b).ldt.sign) || ((a).ldt.sign != (b).ldt.sign))&& ((a).ldt.exponent == (b).ldt.exponent) && ((a).ldt.mantissah == (b).ldt.mantissah) && ((a).ldt.mantissal == (b).ldt.mantissal))


int frexpl_test(void)
{
  unsigned int i, counter;
  int exponent, result;

  for (counter = i = 0; i < n_tests_long_double; i++)
  {
    _longdouble_union_t mantissa;
    mantissa.ld = frexpl(tests_long_double[i].value.ld, &exponent);

    if (IS_EQUAL(tests_long_double[i].mantissa, mantissa) && tests_long_double[i].exponent == exponent)
      counter++;
    else if(IS_EQUAL_NAN(tests_long_double[i].mantissa, mantissa) && tests_long_double[i].exponent == exponent)
      counter++;
    else
      printf("frexpl test failed:  value to frexpl = %.12Lf\n"
             "mantissa result = %.12Lf  exponent result = %d\n"
             "mantissa should be = %.12Lf  exponent should be = %d\n",
             tests_long_double[i].value.ld, mantissa.ld, exponent, tests_long_double[i].mantissa.ld, tests_long_double[i].exponent);
  }

  result = counter < n_tests_long_double ? 1 : 0;
  printf("%s\n", result ? "frexpl test failed." : "frexpl test succeded.");

  return result;
}
