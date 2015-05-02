/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2010 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

asm(".long ___libemu_ident_string");

#define EXP_BIAS 16383
#define EXP_MAX 32766

#define SIGN_POS	0
#define SIGN_NEG	1

#define TW_V	0	// valid
#define TW_Z	1	// zero
#define TW_S	2	// special
#define TW_E	3	// empty

#define EX_SO	0x02C1	// stack overflow
#define EX_SU	0x00C1	// stack underflow
#define EX_P	0x00A0	// loss of precision
#define EX_U	0x0090	// underflow
#define EX_O	0x0088	// overflow
#define EX_Z	0x0084	// divide by zero
#define EX_D	0x0082	// denormalized operand
#define EX_I	0x0081	// invalid operation

#define SW_B	0x8000	// backward compatibility (=ES)
#define SW_C3	0x4000	// condition bit 3
#define SW_TOP	0x3800	// top of stack
#define SW_TOPS 0x0800	// scale for TOS
#define SW_C2	0x0400	// condition bit 2
#define SW_C1	0x0200	// condition bit 1
#define SW_C0	0x0100	// condition bit 0
#define SW_ES	0x0080	// exception summary
#define SW_SF	0x0040	// stack fault
#define SW_PE	0x0020	// loss of precision
#define SW_UE	0x0010	// underflow
#define SW_OE	0x0008	// overflow
#define SW_ZE	0x0004	// divide by zero
#define SW_DE	0x0002	// denormalized operand
#define SW_IE	0x0001	// invalid operation

#define CW_RC	0x0C00	// rounding control
#define CW_PC	0x0300	// precision control
#define CW_PM	0x0020	// precision mask
#define CW_UM	0x0010	// underflow mask
#define CW_OM	0x0008	// overflow mask
#define CW_ZM	0x0004	// divide by zero mask
#define CW_DM	0x0002	// denormalized operand mask
#define CW_IM	0x0001	// invalid operation mask
#define CW_EXM	0x007f	// all masks

#define RC_RND	0x0000
#define RC_DOWN	0x0400
#define RC_UP	0x0800
#define RC_CHOP	0x0C00

#define COMP_A_GT_B	1
#define COMP_A_EQ_B	2
#define COMP_A_LT_B	3
#define COMP_NOCOMP	4
#define COMP_NAN	0x40
#define COMP_SNAN	0x80

#define NAN_NONE 0
#define NAN_SNAN 1
#define NAN_QNAN 2

struct reg {
  char sign;
  char tag;
  short exp;
  unsigned sigl;
  unsigned sigh;
};

typedef void (*FUNC)();

static int modrm;
static unsigned eax;
static unsigned ebx;
static unsigned ecx;
static unsigned edx;
static unsigned esi;
static unsigned edi;
static unsigned ebp;
static unsigned esp;
static unsigned char *eip;

static jmp_buf jumpbuf;

static int status_word = 0;
static int control_word = 0x37f;
static reg regs[8] = {
 { SIGN_POS, TW_E, 0, 0x0, 0x0 },
 { SIGN_POS, TW_E, 0, 0x0, 0x0 },
 { SIGN_POS, TW_E, 0, 0x0, 0x0 },
 { SIGN_POS, TW_E, 0, 0x0, 0x0 },
 { SIGN_POS, TW_E, 0, 0x0, 0x0 },
 { SIGN_POS, TW_E, 0, 0x0, 0x0 },
 { SIGN_POS, TW_E, 0, 0x0, 0x0 },
 { SIGN_POS, TW_E, 0, 0x0, 0x0 },
};
static int top = 0;

static reg CONST_1 = { SIGN_POS, TW_V, EXP_BIAS, 0x00000000, 0x80000000 };
static reg CONST_L2T = { SIGN_POS, TW_V, EXP_BIAS+1, 0xcd1b8afe, 0xd49a784b };
static reg CONST_L2E = { SIGN_POS, TW_V, EXP_BIAS, 0x5c17f0bc, 0xb8aa3b29 };
static reg CONST_PI = { SIGN_POS, TW_V, EXP_BIAS+1, 0x2168c235, 0xc90fdaa2 };
static reg CONST_PI2 = { SIGN_POS, TW_V, EXP_BIAS, 0x2168c235, 0xc90fdaa2 };
static reg CONST_LG2 = { SIGN_POS, TW_V, EXP_BIAS-2, 0xfbcff799, 0x9a209a84 };
static reg CONST_LN2 = { SIGN_POS, TW_V, EXP_BIAS-1, 0xd1cf79ac, 0xb17217f7 };
static reg CONST_Z = { SIGN_POS, TW_Z, 0, 0x0, 0x0 };

static reg CONST_PINF = { SIGN_POS, TW_S, EXP_MAX+1, 0x00000000, 0x80000000 };
static reg CONST_NINF = { SIGN_NEG, TW_S, EXP_MAX+1, 0x00000000, 0x80000000 };
static reg CONST_NAN = { SIGN_NEG, TW_S, EXP_MAX+1, 0x00000000, 0xC0000000 };

inline reg& st(int which=0) { return regs[(top+which)&7]; }

extern "C" int _write(int,void*,int);

static inline int val_same(reg& a, reg& b)
{
  if (a.sign != b.sign || a.exp != b.exp || a.sigl != b.sigl || a.sigh != b.sigh)
    return 0;
  return 1;
}

static inline int is_zero(reg a)
{
  return (a.exp == 0 && a.sigl == 0 && a.sigh == 0);
}

#ifndef eprintf
static void eprintf(const char *f, ...)
{
  va_list args;
  char buf[1000];

  va_start(args, f);
  vsnprintf(buf, sizeof(buf), f, args);
  va_end(args);

  _write(1, buf, strlen(buf));
}
#endif

/* extern "C" void djshld(void *); */
#define djshld(q) ((*(long long unsigned *)(void *)(q)) <<= 1)
#define djshrd(q) ((*(long long unsigned *)(void *)(q)) >>= 1)

static int nan_type(reg& r)
{
  if (r.exp != EXP_MAX+1)
    return NAN_NONE;
  if (r.sigh & 0x40000000)
    return NAN_QNAN;
  if ((r.sigh & 0x3fffffff) || r.sigl)
    return NAN_SNAN;
  return NAN_NONE;	// Inf
}

static int compare(reg& a, reg& b)
{
  int a_inf, b_inf; // 0=no, 1=pos, -1=neg
  a_inf = 0;
  if (val_same(a, CONST_PINF))
    a_inf = 1;
  else if (val_same(a, CONST_NINF))
    a_inf = -1;
  b_inf = 0;
  if (val_same(b, CONST_PINF))
    b_inf = 1;
  else if (val_same(b, CONST_NINF))
    b_inf = -1;
  if (a_inf || b_inf)
  {
    if (a_inf == 1)
    {
      if (b_inf == 1)
	return COMP_A_EQ_B;
      else
        return COMP_A_GT_B;
    }
    if (b_inf == 1)
      return COMP_A_LT_B;

    if (a_inf == -1)
    {
      if (b_inf == -1)
        return COMP_A_EQ_B;
      else
        return COMP_A_LT_B;
    }
    if (b_inf == -1)
      return COMP_A_GT_B;
  }
  int a_nan = nan_type(a);
  int b_nan = nan_type(b);
  if (a_nan || b_nan)
  {
    if ((a_nan == NAN_SNAN) || (b_nan == NAN_SNAN))
      return COMP_NOCOMP | COMP_SNAN | COMP_NAN;
    return COMP_NOCOMP | COMP_NAN;
  }
  if (a.sign != b.sign)
  {
    if ((a.tag == TW_Z) && (b.tag == TW_Z))
      return COMP_A_EQ_B;
    if (a.sign == SIGN_POS)
      return COMP_A_GT_B;
    return COMP_A_LT_B;
  }
  while (!(a.sigh & 0x80000000))
  {
    if (!a.exp)
      break;
    djshld(&a.sigl);
    a.exp--;
  }
  while (!(b.sigh & 0x80000000))
  {
    if (!b.exp)
      break;
    djshld(&b.sigl);
    b.exp--;
  }
  int diff = a.exp - b.exp;
  if (diff == 0) diff = a.sigh > b.sigh ? 1 : (a.sigh < b.sigh ? -1 : 0);
  if (diff == 0) diff = a.sigl > b.sigl ? 1 : (a.sigl < b.sigl ? -1 : 0);
  if (a.sign == SIGN_NEG)
    diff = -diff;
  if (diff > 0) return COMP_A_GT_B;
  if (diff < 0) return COMP_A_LT_B;
  return COMP_A_EQ_B;
}

static void emu_printall()
{
  static const char *tag_desc[] = { "Valid", "Zero", "Special", "Empty" };
  status_word = status_word & ~SW_TOP;
  status_word += (top&7) * SW_TOPS;
  eprintf("  SW: 0x%04x  top=%d cc=%d%d%d%d    ", status_word, top&7,
    status_word & SW_C3?1:0, status_word & SW_C2?1:0,
    status_word & SW_C1?1:0, status_word & SW_C0?1:0);
  eprintf("CW: 0x%04x\r\n", control_word);
  for (int i=0; i<8; i++)
  {
    reg *r = &st(i);
    switch (r->tag)
    {
      case TW_E:
        continue;
        eprintf("st(%d)                                ", i);
        break;
      case TW_Z:
        eprintf("st(%d) %c .0000 0000 0000 0000         ",
                i, r->sign ? '-' : '+');
        break;
      case TW_S:
      case TW_V:
        eprintf("st(%d) %c .%04x %04x %04x %04x e%+-6d ", i,
          r->sign ? '-' : '+',
          (long)(r->sigh >> 16),
          (long)(r->sigh & 0xFFFF),
          (long)(r->sigl >> 16),
          (long)(r->sigl & 0xFFFF),
          r->exp - EXP_BIAS + 1);
    }
    eprintf("%s\r\n", tag_desc[(int)r->tag]);
  }
}

static struct {
  int type;
  const char *name;
} ex_names[] = {
  { EX_SO, "stack overflow" },
  { EX_SU, "stack underflow" },
  { EX_P, "loss of precision" },
  { EX_U, "underflow" },
  { EX_O, "overflow" },
  { EX_Z, "divide by zero" },
  { EX_D, "denormalized operand" },
  { EX_I, "invalid operation" },
  { 0,0 }
};

static void exception(int n)
{
  int i;
  status_word |= n;
  if (n == EX_SU)
    status_word &= ~SW_C1;
  for (i=0; ex_names[i].type; i++)
    if (ex_names[i].type == n)
      break;
  if (~control_word & n & CW_EXM)
  {
    if (ex_names[i].type)
      eprintf("80387 Exception: %s!\r\n", ex_names[i].name);
    else
      eprintf("80387 Exception: 0x%04x!\r\n", n);
    emu_printall();
    
    longjmp(jumpbuf,1);
  }
}

static void emu_bad()
{
  eprintf("Unimplemented 80387 Opcode at eip=0x%08x : %02x", eip-2, eip[-2]);
  if (eip[-1] > 0277)
    eprintf(" %02x", eip[-1]);
  else
    eprintf(" /%d", (eip[-1]>>3)&7);
  eprintf(" - e%d%d", eip[-2]&7, (eip[-1]>>3)&7);
  if (eip[-1] > 0277)
    eprintf(" s%d", eip[-1]&7);
  eprintf("\r\n");
  exception(EX_I);
}

static void
setcc(int cc)
{
  status_word &= ~(SW_C0|SW_C1|SW_C2|SW_C3);
  status_word |= cc & (SW_C0|SW_C1|SW_C2|SW_C3);
}

static int full()
{
  if (st(7).tag != TW_E)
  {
    exception(EX_SO);
    top--;
    st(0) = CONST_NAN;
    return 1;
  }
  return 0;
}

static int empty(int i=0)
{
  if (st(i).tag == TW_E)
  {
    exception(EX_SU);
    return 1;
  }
  return 0;
}

static int sregval(int reg1, int mod)
{
  switch (reg1)
  {
    case 0: return eax;
    case 1: return ecx;
    case 2: return edx;
    case 3: return ebx;
    case 4: return (mod==-1) ? 0 : esp;
    case 5: return mod ? ebp : 0; // data
    case 6: return esi;
    case 7: return edi;
  }
  return 0;
}

static int scale[] = { 1, 2, 4, 8 };

static int getsib()
{
  int mod = modrm >> 6;
  int sib = *eip++;
  int ss = sib>>6;
  int s_index = (sib>>3) & 7;
  int base = sib & 7;
  int rv = sregval(base, mod) + sregval(s_index, -1) * scale[ss];
  int rv2;
  switch (mod)
  {
    case 1:
      rv2 = *(signed char *)eip++;
      rv += rv2;
      break;
    case 0:
      if (base != 5)
        break;
    case 2:
      ((unsigned char *)&rv2)[0] = *eip++;
      ((unsigned char *)&rv2)[1] = *eip++;
      ((unsigned char *)&rv2)[2] = *eip++;
      ((unsigned char *)&rv2)[3] = *eip++;
      rv += rv2;
      break;
  }
  return rv;
}

static int regval(int reg1, int mod)
{
  switch (reg1)
  {
    case 0: return eax;
    case 1: return ecx;
    case 2: return edx;
    case 3: return ebx;
    case 4: return getsib();
    case 5: return mod ? ebp : 0; // data
    case 6: return esi;
    case 7: return edi;
  }
  return 0;
}

static void *get_modrm()
{
  int mod = modrm>>6;
  int rm = modrm & 7;
  int rv = 0;
  switch (mod)
  {
    case 0:
      if (rm == 5)
      {
        ((unsigned char *)&rv)[0] = *eip++;
        ((unsigned char *)&rv)[1] = *eip++;
        ((unsigned char *)&rv)[2] = *eip++;
        ((unsigned char *)&rv)[3] = *eip++;
      }
      else
        rv = regval(rm, mod);
      break;
    case 1:
      if (rm != 4)
        rv = (*(signed char *)eip++) + regval(rm, mod);
      else
        rv = regval(rm, mod);
      break;
    case 2:
      if (rm != 4)
      {
        ((unsigned char *)&rv)[0] = *eip++;
        ((unsigned char *)&rv)[1] = *eip++;
        ((unsigned char *)&rv)[2] = *eip++;
        ((unsigned char *)&rv)[3] = *eip++;
        rv += regval(rm, mod);
      }
      else
        rv = regval(rm, mod);
      break;
    case 3:
      eprintf("Attempt to get address from mod = 3\r\n");
      longjmp(jumpbuf,1);
  }
//  eprintf("modrm returning 0x%x\r\n", rv);
  return (void *)rv;
}



static void r_uadd(reg& a, reg& b, reg& s) // signs ignored
{
  reg t;
  int dif = a.exp - b.exp;
  if (!dif) dif = a.sigh > b.sigh ? 1 : (a.sigh < b.sigh ? -1 : 0);
  if (!dif) dif = a.sigl > b.sigl ? 1 : (a.sigl < b.sigl ? -1 : 0);
  if (dif > 0)
  {
    s = a;
    t = b;
  }
  else
  {
    s = b;
    t = a;
  }
  if (s.exp - t.exp > 64)
    return;
  while (t.exp < s.exp)
  {
    t.exp ++;
    djshrd(&t.sigl);
  }
  unsigned short *ss, *ts;
  unsigned long tmp;
  ss = (unsigned short *)(void *)&s.sigl;
  ts = (unsigned short *)(void *)&t.sigl;
  tmp = 0;
  for (int i=4; i>0; i--)
  {
    tmp += (unsigned long)*ss + (unsigned long)*ts;
    *ss = tmp;
    ss++;
    ts++;
    tmp >>= 16;
  }
  if (tmp)
  {
    djshrd(&s.sigl);
    s.exp++;
    s.sigh |= 0x80000000;
  }
  if (!(s.sigh | s.sigl))
  {
    s.exp = 0;
    s.tag = TW_Z;
  }
  else
  {
    while (!(s.sigh & 0x80000000))
    {
      if (s.exp == 0)
        return;
      djshld(&s.sigl);
      s.exp--;
    }
  }
}

static void r_usub(reg& a, reg& b, reg& d) // a > b
{
  reg t;
  d = a;
  t = b;

  if (d.exp - t.exp > 64)
    return;
  while (t.exp < d.exp)
  {
    t.exp ++;
    djshrd(&t.sigl);
  }
  unsigned short *ss, *ts;
  long tmp;
  ss = (unsigned short *)(void *)&d.sigl;
  ts = (unsigned short *)(void *)&t.sigl;
  tmp = 0;
  for (int i=4; i>0; i--)
  {
    tmp += (long)*ss - (long)*ts;
    *ss = tmp;
    ss++;
    ts++;
    tmp >>= 16;
  }
  if (!(d.sigh | d.sigl))
  {
    d.exp = 0;
    d.tag = TW_Z;
  }
  else
  {
    while (!(d.sigh & 0x80000000))
    {
      if (d.exp == 0)
        return;
      djshld(&d.sigl);
      d.exp--;
    }
  }
}

static void r_sub(reg& a, reg& b, reg& d)
{
  if (b.tag == TW_Z)
  {
    d = a;
    return;
  }
  if (a.tag == TW_Z)
  {
    d = b;
    if (b.tag != TW_Z || b.sign)
      d.sign ^= SIGN_POS^SIGN_NEG;
    return;
  }
  if (a.tag == TW_S)
  {
    if (b.tag == TW_S && nan_type(b) == NAN_NONE && a.sign == b.sign)
    {
      exception(EX_I);
      d = CONST_NAN;
    }
    else
      d = a;
    return;
  }
  if (b.tag == TW_S)
  {
    d = b;
    d.sign ^= SIGN_POS^SIGN_NEG;
    return;
  }

  int mdif;
  mdif = a.exp - b.exp;
  if (!mdif)
    mdif = a.sigh > b.sigh ? 1 : (a.sigh < b.sigh ? -1 : 0);
  if (!mdif)
    mdif = a.sigl > b.sigl ? 1 : (a.sigl < b.sigl ? -1 : 0);

  switch (a.sign*2 + b.sign)
  {
    case 0: // P - P
    case 3: // N - N
      if (mdif > 0)
      {
        r_usub(a, b, d);
        d.sign = a.sign;
      }
      else
      {
        r_usub(b, a, d);
        d.sign = a.sign ^ SIGN_POS^SIGN_NEG;
      }
      break;
    case 1: // P - N
      r_uadd(a, b, d);
      d.sign = SIGN_POS;
      break;
    case 2: // N - P
      r_uadd(a, b, d);
      d.sign = SIGN_NEG;
      break;
  }
}

static void r_add(reg& a, reg& b, reg& s)
{
  char old_sign;
  if (a.tag == TW_Z)
  {
    s = b;
    if (b.tag == TW_Z && a.sign != b.sign)
      s.sign = 0;
    return;
  }
  if (b.tag == TW_Z)
  {
    s = a;
    return;
  }
  if (a.tag == TW_S)
  {
    // "If both operands are infinities of opposite signs, an
    // invalid-operation exception is generated."--Intel manual
    if (a.sign != b.sign
	&& nan_type(a) == NAN_NONE && nan_type(b) == NAN_NONE)
    {
      exception(EX_I);
      s = CONST_NAN;
    }
    else
      s = a;
    return;
  }
  if (b.tag == TW_S)
  {
    s = b;
    return;
  }

  switch (a.sign*2 + b.sign)
  {
    case 0: // P + P
    case 3: // N + N
      r_uadd(a, b, s);
      s.sign = a.sign;
      break;
    case 1: // P + N
      old_sign = b.sign;
      b.sign ^= SIGN_POS^SIGN_NEG;
      r_sub(a, b, s);
      b.sign = old_sign;
      break;
    case 2: // N + P
      old_sign = a.sign;
      a.sign ^= SIGN_POS^SIGN_NEG;
      r_sub(b, a, s);
      a.sign = old_sign;
      break;
  }
}

static void normalize(reg& r)
{
  if (!(r.sigl | r.sigh))
  {
    r.exp = 0;
    r.tag = TW_Z;
    return;
  }
  if (r.exp > EXP_MAX)
  {
    r.tag = TW_S;
    return;
  }
  while (!(r.sigh & 0x80000000))
  {
    if (r.exp == 0)
      return;
    djshld(&r.sigl);
    r.exp--;
  }
}

static void r_mul(reg& a, reg& b, reg& s)
{
  if (a.tag == TW_Z)
  {
    if (b.tag == TW_S
      && (val_same(b, CONST_PINF) || val_same(b, CONST_NINF)))
    {
      exception(EX_I);
      s = CONST_NAN;
    }
    else
      s = CONST_Z;
  }
  else if (b.tag == TW_Z)
  {
    if (a.tag == TW_S
      && (val_same(a, CONST_PINF) || val_same(a, CONST_NINF)))
    {
      exception(EX_I);
      s = CONST_NAN;
    }
    else
      s = CONST_Z;
  }
  else if (a.tag == TW_S)
  {
    s = a;
  }
  else if (b.tag == TW_S)
  {
    s = b;
  }
  else
  {
    unsigned short sl[9], carry[10];
    unsigned short *as = (unsigned short *)(void *)(&a.sigl);
    unsigned short *bs = (unsigned short *)(void *)(&b.sigl);
    unsigned long l, sum;
    int ai, bi;
    for (ai=0; ai<8; ai++)
      sl[ai] = carry[ai] = 0;
    for (ai = 0; ai < 4; ai++)
      for (bi = 0; bi < 4; bi++)
      {
        l = as[ai] * bs[bi];

        sum = sl[ai+bi] + (l & 0xffff);
        sl[ai+bi] = sum & 0xffff;

        sum = sl[ai+bi+1] + (l>>16) + (sum>>16);
        sl[ai+bi+1] = sum & 0xffff;

        carry[ai+bi+2] += sum>>16;
      }
    for (ai=0; ai<8; ai++)
    {
      if (carry[ai])
      {
        sum = sl[ai] + carry[ai];
        sl[ai] = sum & 0xffff;
        carry[ai+1] += sum>>16;
      }
    }
    s.sigl = *(long *)(sl+4);
    s.sigh = *(long *)(sl+6);
    long biased_exp = a.exp;
    biased_exp += b.exp - EXP_BIAS + 1;
    if (biased_exp > EXP_MAX)
      s.exp = EXP_MAX + 1;
    else
      s.exp = biased_exp;
    s.tag = TW_V;
  }
  if (a.sign == b.sign)
    s.sign = SIGN_POS;
  else
    s.sign = SIGN_NEG;
  normalize(s);
}

static void r_div(reg& a, reg& b, reg& q)
{
  if (a.tag == TW_S)
  {
    if (val_same(a, CONST_PINF) || val_same(a, CONST_NINF))
    {
      if (val_same(b, CONST_PINF) || val_same(b, CONST_NINF))
      {
	exception(EX_I);
	q = CONST_NAN;
      }
      else
	q = a;
    }
    else if (a.exp > EXP_MAX)
      q = CONST_NAN;
  }
  else if (b.tag == TW_S)
  {
    if (val_same(b, CONST_PINF))
      q = CONST_Z;
    else if (val_same(b, CONST_NINF))
      q = CONST_Z;
    else if (a.exp > EXP_MAX)
      q = CONST_NAN;
  }
  else if (a.tag == TW_Z)
  {
    if (b.tag == TW_Z)
    {
      exception(EX_I);
      q = CONST_NAN;
    }
    else
      q = a;
  }
  else if (b.tag == TW_Z)
  {
    exception(EX_Z);
    q = CONST_PINF;
  }
  else
  {
    q.exp = a.exp - b.exp + EXP_BIAS;
    if (q.exp > EXP_MAX)
      q = CONST_PINF;
    else if (q.exp <= 0)
      q = CONST_Z;
    else
    {
      unsigned long long al, bl, ql, f;
      int i;
      al = *(unsigned long long *)(void *)(&a.sigl);
      bl = *(unsigned long long *)(void *)(&b.sigl);
      ql = 0;
      f = (unsigned long long)1 << 63;
      for (i=0; i<64; i++)
      {
        if (al >= bl)
        {
          al -= bl;
          ql += f;
        }
        bl >>= 1;
        f >>= 1;
      }
      *(unsigned long long *)(void *)(&q.sigl) = ql;
      q.tag = TW_V;
    }
  }
  if (a.sign == b.sign)
    q.sign = SIGN_POS;
  else
    q.sign = SIGN_NEG;
  normalize(q);
}

static void r_mov(long double *s, reg& d)
{
  unsigned long *sp = (unsigned long *)s;
  if (sp[2] & 0x8000)
    d.sign = SIGN_NEG;
  else
    d.sign = SIGN_POS;
  d.exp = sp[2] & 0x7fff;
  d.sigh = sp[1];
  d.sigl = sp[0];
  d.tag = TW_V;
  normalize(d);
}

static void r_mov(double *s, reg& d)
{
  unsigned m64 = ((unsigned *)s)[1];
  unsigned l64 = ((unsigned *)s)[0];
  if (m64 & 0x80000000)
    d.sign = SIGN_NEG;
  else
    d.sign = SIGN_POS;
  if (!((m64 & 0x7fffffff) | (l64)))
  {
    int c = d.sign;
    d = CONST_Z;
    d.sign = c;
    return;
  }
  d.exp = (int)((m64>>20)&0x7ff) - 1023 + EXP_BIAS;
  d.sigh = ((m64 & 0xfffff)<<11) | 0x80000000;
  d.sigh |= l64 >> 21;
  d.sigl = l64 << 11;
  d.tag = TW_V;
  if ((m64 & 0x7ff00000) == 0x7ff00000)
    d.exp = EXP_MAX+1;
  normalize(d);
}

static void r_mov(float *s, reg& d)
{
  unsigned m32 = *(unsigned *)s;
  if (m32 & 0x80000000)
    d.sign = SIGN_NEG;
  else
    d.sign = SIGN_POS;
  if (!(m32 & 0x7fffffff))
  {
    int c = d.sign;
    d = CONST_Z;
    d.sign = c;
    return;
  }
  d.exp = (int)((m32>>23)&0xff) - 127 + EXP_BIAS;
  d.sigh = ((m32 & 0x7fffff)<<8) | 0x80000000;
  d.sigl = 0;
  d.tag = TW_V;
  if ((m32 & 0x7f800000) == 0x7f800000)
    d.exp = EXP_MAX+1;
  normalize(d);
}

static void r_mov(long long *_s, reg& d)
{
  long long s = *_s;
  if (s == 0)
  {
    d = CONST_Z;
    return;
  }

  if (s > 0)
    d.sign = SIGN_POS;
  else
  {
    s = -s;
    d.sign = SIGN_NEG;
  }

  int e = EXP_BIAS + 63;
  while (s >= 0)
  {
    djshld(&s);
    e -= 1;
  }
  d.sigh = s >> 32;
  d.sigl = s;
  d.exp = e;
  d.tag = TW_V;
  normalize(d);
}

static void r_mov(long *_s, reg& d)
{
  long s = *_s;
  if (s == 0)
  {
    d = CONST_Z;
    return;
  }

  if (s > 0)
    d.sign = SIGN_POS;
  else
  {
    s = -s;
    d.sign = SIGN_NEG;
  }

  int e = EXP_BIAS + 31;
  while (!(s & 0x80000000))
  {
    s <<= 1;
    e -= 1;
  }
  d.sigh = s;
  d.sigl = 0;
  d.exp = e;
  d.tag = TW_V;
  normalize(d);
}

static void r_mov(short *_s, reg& d)
{
  int s = *_s;
  if (s == 0)
  {
    d = CONST_Z;
    return;
  }

  if (s > 0)
    d.sign = SIGN_POS;
  else
  {
    s = -s;
    d.sign = SIGN_NEG;
  }

  int e = EXP_BIAS + 15;
  while (!(s & 0x8000))
  {
    s <<= 1;
    e -= 1;
  }
  d.sigh = s << 16;
  d.sigl = 0;
  d.exp = e;
  d.tag = TW_V;
  normalize(d);
}

static void r_mov(char *s, reg& d)
{
  int side=1, pos=8;
  long long l;
  l = 0;
  for (int i=0; i<18; i++)
  {
    l *= 10;
    switch (side)
    {
      case 0:
        l += s[pos] & 0x0f;
        side = 1;
        pos--;
        break;
      case 1:
        l += s[pos] >> 4;
        side = 0;
        break;
    }
  }
  r_mov(&l, d);
  if (s[9] & 0x80)
    d.sign = SIGN_NEG;
}

//=============================================================================

static void round_to_int(reg& r) // r gets mangled such that sig is int, sign
{
  int more_than_half = 0;
  int half_or_more = 0;
  if (r.tag == TW_Z)
  {
    return;
  }
  while (r.exp < EXP_BIAS+62)
  {
    if (r.sigl & 1)
      more_than_half = 1;
    djshrd(&r.sigl);
    r.exp++;
  }
  while (r.exp < EXP_BIAS+63)
  {
    if (r.sigl & 1)
      half_or_more = 1;
    djshrd(&r.sigl);
    r.exp++;
  }
  if (r.exp > EXP_BIAS+63)
  {
    r.sigl = r.sigh = ~0;
    return;
  }
  switch (control_word & CW_RC)
  {
    case RC_RND:
      if (half_or_more)
      {
        if (more_than_half) // nearest
          (*(long long *)(void *)(&r.sigl)) ++;
        else
          if (r.sigl & 1) // odd?
            (*(long long *)(void *)(&r.sigl)) ++;
      }
      break;
    case RC_DOWN:
      if ((half_or_more||more_than_half) && r.sign)
        (*(long long *)(void *)(&r.sigl)) ++;
      break;
    case RC_UP:
      if ((half_or_more||more_than_half) && !r.sign)
        (*(long long *)(void *)(&r.sigl)) ++;
      break;
    case RC_CHOP:
      break;
  }
}

static void r_mov(reg& s, long double *d)
{
  ((short *)d)[4] = s.exp + s.sign*0x8000;
  ((long *)d)[0] = s.sigl;
  ((long *)d)[1] = s.sigh;
}

static void r_mov(reg& s, double *d)
{
  unsigned long *l = (unsigned long *)d;
  if (s.tag == TW_Z)
  {
    l[0] = 0;
    l[1] = 0;
  }
  else
  {
    reg t = s;
    int rmode = (control_word & CW_RC);
    short unbiased_exp = t.exp - EXP_BIAS;
    unsigned lbit = 1 << 10; // mask for highest bit shifted out of sigl
    unsigned lbits = 0x000003ff; // mask for rest of bits shifted out of sigl
    reg ulp = {t.sign, TW_V, t.exp, lbit, 0}; // the ULP
    // Round the number if necessary
    if (t.exp <= EXP_MAX && unbiased_exp <= 1023)
    {
      if ((rmode == RC_RND
	   && (t.sigl & lbit) && (t.sigl & lbits))
	  || (rmode == RC_UP && t.sign == SIGN_POS
	      && (t.sigl & (lbit | lbits)))
	  || (rmode == RC_DOWN && t.sign == SIGN_NEG
	      && (t.sigl & (lbit | lbits))))
      {
	r_add(s, ulp, t);
	unbiased_exp = t.exp - EXP_BIAS;
      }
    }
    l[0] = (t.sigl >> 11) | (t.sigh << 21);
    l[1] = (t.sigh >> 11) & 0xfffff;
    if (t.exp == 0x7fff)	// NaN or Inf
      l[1] |= 0x7ff00000;
    else if (unbiased_exp > 1023) // overflow
    {
      // If Overflow is unmasked, longjmp without storing anything.
      exception(EX_O);
      if ((rmode == RC_DOWN && t.sign == 0)
	  || (rmode == RC_UP && t.sign)
	  || rmode == RC_CHOP)
      {
	l[0] = 0xffffffff;	// max finite number
	l[1] = 0x7fefffff;
      }
      else
      {
	l[0] = 0;		// Inf
	l[1] = 0x7ff00000;
      }
    }
    else if (unbiased_exp < -1022) // underflow
    {
      exception(EX_U);
      if (unbiased_exp < -1074)
	l[0] = l[1] = 0;
      else
      {	// Gradual underflow
	int to_shift = -1023 - unbiased_exp;
	if (to_shift >= 32)
	{
	  l[0] = l[1] >> (to_shift - 32);
	  l[1] = 0;
	}
	else
	{
	  l[0] >>= to_shift;
	  l[0] |= (l[1] << (32 - to_shift));
	  l[1] >>= to_shift;
	}
      }
    }
    else
      l[1] |= (((unbiased_exp + 1023) & 0x7ff) << 20);
  }
  if (s.sign)
    l[1] |= 0x80000000;
}

static void r_mov(reg& s, float *d)
{
  long f;
  if (s.tag == TW_Z)
  {
    f = 0;
  }
  else
  {
    reg t = s;
    int rmode = (control_word & CW_RC);
    short unbiased_exp = t.exp - EXP_BIAS;
    unsigned lbit = 1 << 7; // mask for highest bit shifted out of sigh
    unsigned lbits = 0x0000007f; // mask for rest of bits shifted out of sigh
    reg ulp = {t.sign, TW_V, t.exp, 0, lbit}; // the ULP
    // Round the number if necessary
    if (t.exp <= EXP_MAX && unbiased_exp <= 127)
    {
      if ((rmode == RC_RND
	   && (t.sigh & lbit) && ((t.sigh & lbits) || t.sigl))
	  || (rmode == RC_UP && t.sign == SIGN_POS
	      && ((t.sigh & (lbit | lbits)) || t.sigl))
	  || (rmode == RC_DOWN && t.sign == SIGN_NEG
	      && ((t.sigh & (lbit | lbits)) || t.sigl)))
      {
	r_add(s, ulp, t);
	unbiased_exp = t.exp - EXP_BIAS;
      }
    }
    f = (t.sigh >> 8) & 0x007fffff;
    if (t.exp == 0x7fff)	// NaN or Inf
      f |= 0x7f800000;
    else if (unbiased_exp > 127) // overflow
    {
      // If Overflow is unmasked, longjmp without storing anything.
      exception(EX_O);
      if ((rmode == RC_DOWN && t.sign == 0)
	  || (rmode == RC_UP && t.sign)
	  || rmode == RC_CHOP)
	f = 0x7f7fffff;		// max finite number
      else
	f = 0x7f800000;		// Inf
    }
    else if (unbiased_exp < -126) // underflow
    {
      exception(EX_U);
      if (unbiased_exp < -149)
	f = 0;
      else
	// Gradual underflow
	f >>= -127 - unbiased_exp;
    }
    else
      f |= ((unbiased_exp + 127) & 0xff) << 23;
  }
  if (s.sign)
    f |= 0x80000000;
  *(long *)d = f;
}

static void r_mov(reg& s, long long *d)
{
  reg t;
  t = s;
  if (t.tag == TW_S
      && (val_same(t, CONST_PINF) || val_same(t, CONST_NINF)))
  {
    exception(EX_I);
    ((long *)d)[0] = 0;
    ((long *)d)[1] = 0x80000000;
    return;
  }
  round_to_int(t);
  ((long *)d)[0] = t.sigl;
  ((long *)d)[1] = t.sigh;
  if (t.sign)
    *d = - *d;
}

static void r_mov(reg& s, long *d)
{
  reg t;
  t = s;
  if (t.tag == TW_S
      && (val_same(t, CONST_PINF) || val_same(t, CONST_NINF)))
  {
    exception(EX_I);
    *d = 0x80000000;
    return;
  }
  round_to_int(t);
  if (t.sigh || (t.sigl & 0x80000000))
    *d = -1;
  else
    *d = s.sign ? -t.sigl : t.sigl;
}

static void r_mov(reg& s, short *d)
{
  reg t;
  t = s;
  if (t.tag == TW_S
      && (val_same(t, CONST_PINF) || val_same(t, CONST_NINF)))
  {
    exception(EX_I);
    *d = -32767;
    return;
  }
  round_to_int(t);
  if (t.sigh || (t.sigl & 0xFFFF8000))
    *d = -1;
  else
    *d = s.sign ? -t.sigl : t.sigl;
}

static void r_mov(reg& s, char *d)
{
  reg t;
  t = s;
  round_to_int(t);
  long long ll = *(long long *)(void *)(&t.sigl);
  int side = 0;
  int r, i;
  for (i=0; i<10; i++)
    d[i] = 0;
  int pos=0;
  for (i=0; i<18; i++)
  {
    r = ll % 10;
    ll /= 10;
    if (side)
    {
      d[pos] |= r << 4;
      side = 0;
      pos++;
    }
    else
    {
      d[pos] |= r;
      side = 1;
    }
  }
  if (s.sign == SIGN_NEG)
    d[9] = 0x80;
}

static void emu_00()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fadd st,st(i)
    int i = modrm & 7;
    if (empty(i))
      return;
    reg tmp;
    r_add(st(), st(i), tmp);
    st() = tmp;
    return;
  }
  else
  {
    // fadd m32real
    reg t1, t2;
    r_mov((float *)get_modrm(), t1);
    r_add(t1, st(), t2);
    st() = t2;
  }
}

static void emu_01()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fmul st,st(i)
    int i = modrm & 7;
    if (empty(i))
      return;
    reg t;
    r_mul(st(), st(i), t);
    st() = t;
  }
  else
  {
    // fmul m32real
    reg t, t2;
    r_mov((float *)get_modrm(), t);
    r_mul(st(), t, t2);
    st() = t2;
  }
}

static void emu_02()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fcom st(i)
    if (empty(modrm&7))
    {
      setcc(SW_C3|SW_C2|SW_C0);
      return;
    }
    int c = compare(st(), st(modrm&7));
    int f=0;
    if (c & COMP_NAN)
    {
      setcc(SW_C3|SW_C2|SW_C0);
      exception(EX_I);
    }
    else
      switch (c)
      {
        case COMP_A_LT_B:
          f = SW_C0;
          break;
        case COMP_A_EQ_B:
          f = SW_C3;
          break;
        case COMP_A_GT_B:
          f = 0;
          break;
        case COMP_NOCOMP:
          f = SW_C3 | SW_C2 | SW_C0;
          break;
      }
    setcc(f);
    
  }
  else
  {
    // fcom m32real
    reg t;
    r_mov((float *)get_modrm(), t);
    int c = compare(st(), t);
    int f=0;
    if (c & COMP_NAN)
    {
      setcc(SW_C3|SW_C2|SW_C0);
      exception(EX_I);
    }
    else
      switch (c)
      {
        case COMP_A_LT_B:
          f = SW_C0;
          break;
        case COMP_A_EQ_B:
          f = SW_C3;
          break;
        case COMP_A_GT_B:
          f = 0;
          break;
        case COMP_NOCOMP:
          f = SW_C3 | SW_C2 | SW_C0;
          break;
      }
    setcc(f);
  }
}

static void emu_03()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fcomp st(i)
    if (empty(modrm&7))
    {
      setcc(SW_C3|SW_C2|SW_C0);
      return;
    }
    int c = compare(st(), st(modrm&7));
    st().tag = TW_E;
    top++;
    int f=0;
    if (c & COMP_NAN)
    {
      setcc(SW_C3|SW_C2|SW_C0);
      exception(EX_I);
    }
    else
      switch (c)
      {
        case COMP_A_LT_B:
          f = SW_C0;
          break;
        case COMP_A_EQ_B:
          f = SW_C3;
          break;
        case COMP_A_GT_B:
          f = 0;
          break;
        case COMP_NOCOMP:
          f = SW_C3 | SW_C2 | SW_C0;
          break;
      }
    setcc(f);
    
  }
  else
  {
    // fcom m32real
    reg t;
    r_mov((float *)get_modrm(), t);
    int c = compare(st(), t);
    st().tag = TW_E;
    top++;
    int f=0;
    if (c & COMP_NAN)
    {
      setcc(SW_C3|SW_C2|SW_C0);
      exception(EX_I);
    }
    else
      switch (c)
      {
        case COMP_A_LT_B:
          f = SW_C0;
          break;
        case COMP_A_EQ_B:
          f = SW_C3;
          break;
        case COMP_A_GT_B:
          f = 0;
          break;
        case COMP_NOCOMP:
          f = SW_C3 | SW_C2 | SW_C0;
          break;
      }
    setcc(f);
  }
}

static void emu_04()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fsub st,st(i)
    int i = modrm & 7;
    if (empty(i))
      return;
    reg tmp;
    r_sub(st(), st(i), tmp);
    st() = tmp;
    return;
  }
  else
  {
    // fsub m32real
    reg t1, t2;
    r_mov((float *)get_modrm(), t1);
    r_sub(st(), t1, t2);
    st() = t2;
  }
}

static void emu_05()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fsubr st,st(i)
    int i = modrm & 7;
    if (empty(i))
      return;
    reg tmp;
    r_sub(st(i), st(), tmp);
    st() = tmp;
    return;
  }
  else
  {
    // fsubr m32real
    reg t1, t2;
    r_mov((float *)get_modrm(), t1);
    r_sub(t1, st(), t2);
    st() = t2;
  }
}

static void emu_06()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fdiv st,st(i)
    int i = modrm & 7;
    if (empty(i))
      return;
    reg t;
    r_div(st(), st(i), t);
    st() = t;
  }
  else
  {
    // fdiv m32real
    reg t, t2;
    r_mov((float *)get_modrm(), t);
    r_div(st(), t, t2);
    st() = t2;
  }
}

static void emu_07()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fdivr st,st(i)
    int i = modrm & 7;
    if (empty(i))
      return;
    reg t;
    r_div(st(i), st(), t);
    st() = t;
  }
  else
  {
    // fdivr m32real
    reg t, t2;
    r_mov((float *)get_modrm(), t);
    r_div(t, st(), t2);
    st() = t2;
  }
}

static void emu_10()
{
  if (full())
    return;
  if (modrm > 0277)
  {
    // fld st(i)
    int i = modrm & 7;
    if (empty(i))
      return;
    st(7) = st(i);
    top--;
    return;
  }
  else
  {
    // fld m32real
    top--;
    r_mov((float *)get_modrm(), st());
  }
}

static void emu_11()
{
  if (modrm > 0277)
  {
    if (empty())
      st() = CONST_NAN;
    // fxch st(i)
    int i = modrm & 7;
    if (empty(i))
      st(i) = CONST_NAN;
    reg t;
    t = st();
    st() = st(i);
    st(i) = t;
  }
  else
  {
    emu_bad();
  }
}

static void fnop()
{
}

static FUNC emu_12_table[] = {
  fnop, emu_bad, emu_bad, emu_bad, emu_bad, emu_bad, emu_bad, emu_bad
};

static void emu_12()
{
  if (modrm > 0277)
  {
    (emu_12_table[modrm&7])();
  }
  else
  {
    // fst m32real
    if (empty())
      return;
    r_mov(st(), (float *)get_modrm());
  }
}

static void emu_13()
{
  if (modrm > 0277)
  {
    emu_bad();
  }
  else
  {
    // fstp m32real
    if (empty())
      return;
    r_mov(st(), (float *)get_modrm());
    st().tag = TW_E;
    top++;
  }
}

static void fchs()
{
  if (empty())
    return;
  st().sign ^= SIGN_POS^SIGN_NEG;
  status_word &= ~SW_C1;
}

static void fabs_()
{
  if (empty())
    return;
  st().sign = SIGN_POS;
  status_word &= ~SW_C1;
}

static void ftst()
{
  switch (st().tag)
  {
    case TW_Z:
      setcc(SW_C3);
      break;
    case TW_V:
      if (st().sign == SIGN_POS)
        setcc(0);
      else
        setcc(SW_C0);
      break;
    case TW_S:
      if (val_same(st(), CONST_PINF))
      {
        setcc(0);
        break;
      }
      else if (val_same(st(), CONST_NINF))
      {
        setcc(SW_C0);
        break;
      }
      setcc(SW_C0|SW_C2|SW_C3);
      exception(EX_I);
      break;
    case TW_E:
      setcc(SW_C0|SW_C2|SW_C3);
      exception(EX_SU);
      break;
  }
}

static void fxam()
{
  int c=0;
  switch (st().tag)
  {
    case TW_E:
      c = SW_C3|SW_C0;
      break;
    case TW_Z:
      c = SW_C3;
      break;
    case TW_V:
      if (st().sigh & 0x80000000)
        c = SW_C2;
      else
        c = SW_C3|SW_C2;
      break;
    case TW_S:
      if (val_same(st(), CONST_NAN))
        c = SW_C0;
      else if (val_same(st(), CONST_PINF))
        c = SW_C2|SW_C0;
      else if (val_same(st(), CONST_NINF))
        c = SW_C2|SW_C0;
      break;
  }
  if (st().sign == SIGN_NEG)
    c |= SW_C1;
  setcc(c);
}

static FUNC emu_14_table[] = {
  fchs, fabs_, emu_bad, emu_bad, ftst, fxam, emu_bad, emu_bad
};

static void emu_14()
{
  if (modrm > 0277)
  {
    (emu_14_table[modrm&7])();
  }
  else
  {
    //
    emu_bad();
  }
}

static void fld_const(reg &c)
{
  if (full())
    return;
  top--;
  st() = c;
  status_word &= ~SW_C1;
}

static void fld1()
{
  fld_const(CONST_1);
}

static void fldl2t()
{
  fld_const(CONST_L2T);
}

static void fldl2e()
{
  fld_const(CONST_L2E);
}

static void fldpi()
{
  fld_const(CONST_PI);
}

static void fldlg2()
{
  fld_const(CONST_LG2);
}

static void fldln2()
{
  fld_const(CONST_LN2);
}

static void fldz()
{
  fld_const(CONST_Z);
}

static FUNC emu_15_table[] = {
  fld1, fldl2t, fldl2e, fldpi, fldlg2, fldln2, fldz, emu_bad
};

static void emu_15()
{
  if (modrm > 0277)
  {
    (emu_15_table[modrm&7])();
  }
  else
  {
    // fldcw
    control_word = *(short *)get_modrm();
  }
}

static void f2xm1()
{
  if (empty())
    return;
  reg xloga, val, rv, bottom, tmp;
  long i;

  r_mul(CONST_LN2, st(), xloga);
  val = xloga;
  rv = xloga;

  for (i=2; i<19; i++)
  {
    r_mov(&i, bottom);
    r_mul(val, xloga, tmp);
    r_div(tmp, bottom, val);
    r_add(val, rv, tmp);
    rv = tmp;
  }
  st() = rv;
}

// logb(x) = loga(x) / loga(b)
// log2(x) = loge(x) / loge(2)

static void fyl2x()
{
  if (empty() || empty(1))
    return;
  reg z, x, nom, denom, xsquare, term, temp, sum, pow;
  long exponent;
  reg CONST_SQRT2 = { SIGN_POS, TW_V, EXP_BIAS, 0xf9de6000, 0xb504f333 };

  z = st();
  if ((z.tag != TW_V) || (z.sign != SIGN_POS)
      || (val_same(z, CONST_1)
	  && (val_same(st(1), CONST_NINF) || val_same(st(1), CONST_PINF))))
  {
    char sign = st(1).sign;
    if (val_same(z, CONST_NINF)
	|| (z.sign == SIGN_NEG && z.tag == TW_V)
	|| (z.tag == TW_Z && st(1).tag == TW_Z)
	|| (val_same(z, CONST_PINF) && st(1).tag == TW_Z)
	|| (val_same(z, CONST_1)
	    && (val_same(st(1), CONST_NINF) || val_same(st(1), CONST_PINF))))
    {
      exception(EX_I);
      st(1) = CONST_NAN;
    }
    else if (z.tag == TW_Z && st(1).tag == TW_Z)
    {
      exception(EX_Z);
      st(1) = CONST_PINF;
      if (sign == SIGN_POS)
	st(1).sign = SIGN_NEG;
    }
    else if (nan_type(z) != NAN_NONE)
      st(1) = z;
    else if (nan_type(st(1)) == NAN_NONE && z.tag == TW_S)
    {
      st(1) = CONST_PINF;
      st(1).sign = sign;
    }
    st().tag = TW_E;
    top++;
    return;
  }  
  exponent = (long)(z.exp - EXP_BIAS);
  z.exp=EXP_BIAS;
  if (compare(z, CONST_SQRT2) == COMP_A_GT_B)
  {
    (z.exp)--;
    exponent++;
  }

  r_sub(z, CONST_1, nom);
  r_add(z, CONST_1, denom);
  r_div(nom, denom, x);
  if (x.tag == TW_S)
  {
    st().tag = TW_E;
    top++;
    int sign = st().sign;
    st() = CONST_PINF;
    if (!sign)
      st().sign = 1;
    return;
  }
  pow = x;
  sum = x;
  r_mul(x, x, xsquare);
  
  for (long i=3; i<25; i+=2)
  {
    r_mul(pow, xsquare, temp);
    pow = temp;

    r_mov(&i, denom);
    r_div(pow, denom, term);

    r_add(term, sum, temp);
    sum = temp;
  }
  r_div(sum, CONST_LN2, temp);
  temp.exp++;
  if (exponent) {
    r_mov(&exponent, term);
    r_add(term, temp, sum);
  } else {
    sum = temp;
  }

  r_mul(sum, st(1), temp);
  st(1) = temp;
  st().tag = TW_E;
  top++;
}

static int fprem_do(reg& quot, reg& div1, int round) // remainder of st() / st(1)
{
  int rv;
  int old_cw = control_word;
  control_word &= ~CW_RC;
  control_word |= round;
  int expdif = quot.exp - div1.exp;
  if (expdif < 64)
  {
    reg tmp, tmp2;
    r_div(quot, div1, tmp);
    long long q;
    r_mov(tmp, &q);
    r_mov(&q, tmp);
    r_mul(div1, tmp, tmp2);
    r_sub(quot, tmp2, tmp);
    quot = tmp;
    rv = q & 7;
  }
  else
  {
    reg tmp, tmp2;
    setcc(SW_C2);
    r_div(st(), div1, tmp);
    int old_exp = tmp.exp;
    // 62 below is "an implementation-dependent number between 32
    // and 63", as required by the Intel manual.
    tmp.exp = 62 + EXP_BIAS;
    long long q;
    r_mov(tmp, &q);
    r_mov(&q, tmp);
    tmp.exp = old_exp;
    r_mul(div1, tmp, tmp2);
    r_sub(quot, tmp2, tmp);
    quot = tmp;
    rv = -1;
  }
  control_word = old_cw;
  return rv;;
}

static void fsincos()
{
  if (empty() || full())
    return;
  if (st().exp > EXP_MAX)
  {
    if (nan_type(st()) != NAN_NONE)
    {
      top--;
      st() = st(1);
    }
    else
      exception(EX_I);
    return;
  }
  if (st().exp > 63 + EXP_BIAS)
  {
    /* Source operand is out of range.  */
    setcc(SW_C2);
    return;
  }
  int q = fprem_do(st(), CONST_PI2, RC_CHOP);

  if (q & 1)
  {
    reg tmp;
    r_sub(CONST_PI2, st(), tmp);
    st() = tmp;
  }

  reg x2, val, rv, tmp, t2;
  reg valc, rvc, tmpc;
  val = st();
  r_mul(st(), val, x2);
  rv = val;
  valc = CONST_1;
  rvc = valc;


  for (int i=0; i<11; i++)
  {
    val.sign ^= SIGN_POS ^ SIGN_NEG;
    valc.sign ^= SIGN_POS ^ SIGN_NEG;
    r_mul(x2, val, tmp);
    r_mul(x2, valc, tmpc);
    long c = ((i<<1)+2) * ((i<<1)+3);
    r_mov(&c, t2);
    r_div(tmp, t2, val);
    c = ((i<<1)+1) * ((i<<1)+2);
    r_mov(&c, t2);
    r_div(tmpc, t2, valc);
    r_add(val, rv, tmp);
    rv = tmp;
    r_add(valc, rvc, tmpc);
    rvc = tmpc;
  }
  setcc(0);

  if (q & 2)
    rv.sign ^= SIGN_POS ^ SIGN_NEG;
  st() = rv;

  top--;
  register int qq = q & 3;
  if ((qq == 1) || (qq == 2))
    rvc.sign ^= SIGN_POS ^ SIGN_NEG;
  st() = rvc;
}

static void fptan()
{
  fsincos();
  if ((status_word & SW_C2) == SW_C2 || empty(1))
    return;
  reg tmp;
  r_div(st(1), st(), tmp);
  st(1) = tmp;
  st() = CONST_1;
}

static void fpatan()
{
  if (empty(1))
    return;
  if (nan_type(st()) != NAN_NONE || nan_type(st(1)) != NAN_NONE)
  {
    st(1) = CONST_NAN;
    st().tag = TW_E;
    top++;
    return;
  }
  if (is_zero(st()))
  {
    // Propagate sign of numerator
    char num_sign = st(1).sign;
    if (is_zero(st(1)))
      st(1) = CONST_PI;
    else
      st(1) = CONST_PI2;
    st(1).sign = num_sign;
    st().tag = TW_E;
    top++;
    return;
  }

  if (is_zero(st(1)))
  {
    // Check for sign of denominator
    // st(1) = CONST_Z;
    char st1_sign = st(1).sign;
    st(1) = (st(0).sign == SIGN_NEG) ? CONST_PI : CONST_Z;
    st(1).sign = st1_sign;
    st().tag = TW_E;
    top++;
    return;
  }
  if (st().tag == TW_S
      && (val_same(st(), CONST_NINF) || val_same(st(), CONST_PINF)))
  {
    char sign = st(1).sign;
    char tag  = st(1).tag;
    reg tmp, pi_4, two;
    long i = 2;
    st(1) = (st().sign == SIGN_NEG) ? CONST_PI : CONST_Z;
    if (tag != TW_V)
    {
      r_mov(&i, two);
      r_div(CONST_PI2, two, pi_4);
      if (st().sign == SIGN_NEG)
	r_sub(st(1), pi_4, tmp);
      else
	r_add(st(1), pi_4, tmp);
      st(1) = tmp;
    }
    st(1).sign = sign;
    st().tag = TW_E;
    top++;
    return;
  }
  if (st(1).tag == TW_S
      && (val_same(st(1), CONST_NINF) || val_same(st(1), CONST_PINF)))
  {
    char nsign = st(1).sign;
    st(1) = CONST_PI2;
    st(1).sign = nsign;
    st().tag = TW_E;
    top++;
    return;
  }
  reg x2, na, da, nb, db, temp, mx2, sum, m;
  int quadrant = 0;
  if (st(1).sign == SIGN_NEG)
    quadrant |= 1;
  if (st(0).sign == SIGN_NEG)
    quadrant |= 2;
  st(1).sign = st().sign = SIGN_POS;
  if (compare(st(1), st()) == COMP_A_GT_B)
  {
    quadrant |= 4;
    temp = st(1);
    st(1) = st();
    st() = temp;
  }

  r_div(st(1), st(), nb);
  r_mul(nb, nb, x2);
  da = db = m = CONST_1;
  na = CONST_Z;

  for (long i=3; i<53; i+=2)
  {
    reg t1, t2, ti;

    r_mul(m, x2, mx2);
    r_mov(&i, ti);

    r_mul(na, mx2, t1);
    r_mul(ti, nb, t2);
    r_add(t2, t1, temp);
    na = nb;
    nb = temp;

    r_mul(da, mx2, t1);
    r_mul(ti, db, t2);
    r_add(t2, t1, temp);
    da = db;
    db = temp;

    r_add(m, ti, t1);
    m = t1;
  }

  r_div(na, da, sum);

  if (quadrant & 4)
  {
    r_sub(CONST_PI2, sum, temp);
    sum = temp;
  }
  if (quadrant & 2)
  {
    r_sub(CONST_PI, sum, temp);
    sum = temp;
  }
  if (quadrant & 1)
    sum.sign ^= SIGN_POS^SIGN_NEG;

  st(1) = sum;
  st().tag = TW_E;
  top++;
}

static void fxtract()
{
  if (empty())
    return;
  if (full())
    return;
  top--;
  st() = st(1);
  st().exp = EXP_BIAS;
  long e = st(1).exp - EXP_BIAS;
  r_mov(&e, st(1));
}

static void fprem1()
{
  if (empty(1))
    return;
  reg t = st();
  int q = fprem_do(st(), st(1), RC_RND);
  // If the result is zero, its sign must be the same as the sign of
  // the dividend.
  if (st().tag == TW_Z)
    st().sign = t.sign;
  if (q == -1)
    setcc(SW_C2);
  else
  {
    int c = 0;
    if (q&4) c |= SW_C0;
    if (q&2) c |= SW_C3;
    if (q&1) c |= SW_C1;
    setcc(c);
  }
}

static void fdecstp()
{
  status_word &= ~SW_C1;
  top--;
}

static void fincstp()
{
  status_word &= ~SW_C1;
  top++;
}

static FUNC emu_16_table[] = {
  f2xm1, fyl2x, fptan, fpatan, fxtract, fprem1, fdecstp, fincstp
};

static void emu_16()
{
  if (modrm > 0277)
  {
    (emu_16_table[modrm&7])();
  }
  else
  {
    emu_bad();
  }
}

static void fprem()
{
  if (empty(1))
    return;
  // Intel Manual says: "The sign of the remainder is the same as the
  // sign of the dividend."
  int sign = st().sign;
  int q = fprem_do(st(), st(1), RC_CHOP);
  st().sign = sign;
  if (q == -1)
    setcc(SW_C2);
  else
  {
    int c = 0;
    if (q&4) c |= SW_C0;
    if (q&2) c |= SW_C3;
    if (q&1) c |= SW_C1;
    setcc(c);
  }
}

void fyl2x();

static void fyl2xp1()
{
  reg newx;
  // FIXME: this is inaccurate when the argument is near zero, which
  // defeats the purpose of the FYL2XP1 instruction.  Instead, compute
  // the same series as in FYL2X, but for x/(x+2) instead of
  // (x-1)/(x+1).
  r_add(st(), CONST_1, newx);
  st() = newx;
  fyl2x();
}

static void fsqrt()
{
  if (empty())
    return;
  if (st().tag == TW_Z)
    return;
  if (st().exp > EXP_MAX
      && (nan_type(st()) != NAN_NONE || st().sign == SIGN_POS))
    return;
  if (st().sign == SIGN_NEG)
  {
    exception(EX_I);
    st() = CONST_NAN;
    return;
  }

  unsigned long long val = *(unsigned long long *)(void *)(&st().sigl);
  unsigned long long result = 0;
  unsigned long long side = 0;
  unsigned long long left = 0;
  if (st().exp & 1)
  {
    djshrd(&val);
    st().exp++;
  }
  int exp = (st().exp - EXP_BIAS - 1)/2 - 64;
  while (!(((__dj_long_a *)&result)[1] & 0x80000000))
  {
    /* GCC between 2.8 and EGCS 1.1.1 optimizes this loop
       all wrong; the asm works around it. */
    asm volatile("" : : : "memory");
    left = (left << 2) + (((__dj_long_a *)&val)[1] >> 30);
    djshld(&val);
    djshld(&val);
    if (left >= side*2 + 1)
    {
      left -= side*2+1;
      side = (side+1)*2;
      djshld(&result);
      result |= 1;
    }
    else
    {
      side *= 2;
      djshld(&result);
    }
    exp++;
  }
  st().exp = exp + EXP_BIAS;
  st().sigl = result & 0xffffffff;
  st().sigh = result >> 32;
  st().tag = TW_V;
}

static void frndint()
{
  if (empty())
    return;
  long long tmp;
  if (st().exp > EXP_BIAS+62)
    return;
  reg t = st();
  // We need to remember the sign to return -0.0 if the
  // argument was negative.
  int sign = t.sign;
  r_mov(t, &tmp);
  r_mov(&tmp, t);
  t.sign = sign;
  st() = t;
}

static void fscale()
{
  long long scale1;
  int old_cw = control_word;
  if (empty(1) || st().exp > EXP_MAX || st(1).exp > EXP_MAX)
    return;
  control_word &= ~CW_RC;
  control_word |= RC_CHOP;
  r_mov(st(1), &scale1);
  control_word = old_cw;
  if (scale1 > EXP_MAX - st().exp)
  {
    int sign = st().sign;
    exception(EX_O);
    st() = CONST_PINF;	// FIXME: the result should depend on rounding mode
    st().sign = sign;
  }
  else
    st().exp += scale1;
}

static void fsin()
{
  if (empty())
    return;
  if (st().exp > EXP_MAX)
  {
    if (nan_type(st()) == NAN_NONE)
      exception(EX_I);
    setcc(SW_C2);
    return;
  }
  if (st().exp > 63 + EXP_BIAS)
  {
    /* Source operand is out of range.  */
    setcc(SW_C2);
    return;
  }
  int q = fprem_do(st(), CONST_PI2, RC_CHOP);

  if (q & 1)
  {
    reg tmp;
    r_sub(CONST_PI2, st(), tmp);
    st() = tmp;
  }

  reg x2, val, rv, tmp, t2;
  val = st();
  r_mul(st(), val, x2);
  rv = val;


  for (int i=0; i<11; i++)
  {
    long c = ((i<<1)+2) * ((i<<1)+3);
    val.sign ^= SIGN_POS ^ SIGN_NEG;
    r_mul(x2, val, tmp);
    r_mov(&c, t2);
    r_div(tmp, t2, val);
    r_add(val, rv, tmp);
    rv = tmp;
  }
  setcc(0);
  if (q & 2)
    rv.sign ^= SIGN_POS ^ SIGN_NEG;
  st() = rv;
}

static void fcos()
{
  if (empty())
    return;
  if (st().exp > EXP_MAX)
  {
    if (nan_type(st()) == NAN_NONE)
      exception(EX_I);
    setcc(SW_C2);
    return;
  }
  if (st().exp > 63 + EXP_BIAS)
  {
    /* Source operand is out of range.  */
    setcc(SW_C2);
    return;
  }
  int q = fprem_do(st(), CONST_PI2, RC_CHOP);

  if (q & 1)
  {
    reg tmp;
    r_sub(CONST_PI2, st(), tmp);
    st() = tmp;
  }

  reg x2, val, rv, tmp, t2;
  val = st();
  r_mul(st(), val, x2);
  val = CONST_1;
  rv = val;


  for (int i=0; i<11; i++)
  {
    long c = ((i<<1)+1) * ((i<<1)+2);
    val.sign ^= SIGN_POS ^ SIGN_NEG;
    r_mul(x2, val, tmp);
    r_mov(&c, t2);
    r_div(tmp, t2, val);
    r_add(val, rv, tmp);
    rv = tmp;
  }
  setcc(0);
  register int qq = q & 3;
  if ((qq == 1) || (qq == 2))
    rv.sign ^= SIGN_POS ^ SIGN_NEG;
  st() = rv;
}

static FUNC emu_17_table[] = {
  fprem, fyl2xp1, fsqrt, fsincos, frndint, fscale, fsin, fcos
};

static void emu_17()
{
  if (modrm > 0277)
  {
    (emu_17_table[modrm&7])();
  }
  else
  {
    // fstcw m16int
    *(short *)get_modrm() = control_word;
  }
}

static void emu_20()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    emu_bad();
  }
  else
  {
    // fiadd m32int
    reg t1, t2;
    r_mov((long *)get_modrm(), t1);
    r_add(st(), t1, t2);
    st() = t2;
  }
}

static void emu_21()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    //
    emu_bad();
  }
  else
  {
    // fimul m32int
    reg t, t2;
    r_mov((long *)get_modrm(), t);
    r_mul(st(), t, t2);
    st() = t2;
  }
}

static void emu_22()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    emu_bad();
  }
  else
  {
    // ficom m32int
    reg t;
    r_mov((long *)get_modrm(), t);
    int c = compare(st(), t);
    int f=0;
    if (c & COMP_NAN)
    {
      setcc(SW_C3|SW_C2|SW_C0);
      exception(EX_I);
    }
    else
      switch (c)
      {
        case COMP_A_LT_B:
          f = SW_C0;
          break;
        case COMP_A_EQ_B:
          f = SW_C3;
          break;
        case COMP_A_GT_B:
          f = 0;
          break;
        case COMP_NOCOMP:
          f = SW_C3 | SW_C2 | SW_C0;
          break;
      }
    setcc(f);
  }
}

static void emu_23()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    emu_bad();
  }
  else
  {
    // ficomp m32int
    reg t;
    r_mov((long *)get_modrm(), t);
    int c = compare(st(), t);
    st().tag = TW_E;
    top++;
    int f=0;
    if (c & COMP_NAN)
    {
      setcc(SW_C3|SW_C2|SW_C0);
      exception(EX_I);
    }
    else
      switch (c)
      {
        case COMP_A_LT_B:
          f = SW_C0;
          break;
        case COMP_A_EQ_B:
          f = SW_C3;
          break;
        case COMP_A_GT_B:
          f = 0;
          break;
        case COMP_NOCOMP:
          f = SW_C3 | SW_C2 | SW_C0;
          break;
      }
    setcc(f);
  }
}

static void emu_24()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    emu_bad();
  }
  else
  {
    // fisub m32int
    reg t1, t2;
    r_mov((long *)get_modrm(), t1);
    r_sub(st(), t1, t2);
    st() = t2;
  }
}

static void emu_25()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fucompp
    if ((modrm&7) != 1)
      emu_bad();
    else
    {
    if (empty(modrm&7))
    {
      setcc(SW_C3|SW_C2|SW_C0);
      return;
    }
    int c = compare(st(), st(1));
    st().tag = TW_E;
    top++;
    st().tag = TW_E;
    top++;
    int f=0;
    if (c & COMP_SNAN)
    {
      setcc(SW_C3|SW_C2|SW_C0);
      exception(EX_I);
    }
    else
      switch (c)
      {
        case COMP_A_LT_B:
          f = SW_C0;
          break;
        case COMP_A_EQ_B:
          f = SW_C3;
          break;
        case COMP_A_GT_B:
          f = 0;
          break;
        case COMP_NOCOMP:
          f = SW_C3 | SW_C2 | SW_C0;
          break;
      }
    setcc(f);
    }
  }
  else
  {
    // fisubr m32int
    reg t1, t2;
    r_mov((long *)get_modrm(), t1);
    r_sub(t1, st(), t2);
    st() = t2;
  }
}

static void emu_26()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    //
    emu_bad();
  }
  else
  {
    // fidiv m32int
    reg t, t2;
    r_mov((long *)get_modrm(), t);
    r_div(st(), t, t2);
    st() = t2;
  }
}

static void emu_27()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    //
    emu_bad();
  }
  else
  {
    // fidivr m32int
    reg t, t2;
    r_mov((long *)get_modrm(), t);
    r_div(t, st(), t2);
    st() = t2;
  }
}

static void emu_30()
{
  if (full())
    return;
  if (modrm > 0277)
  {
    emu_bad();
  }
  else
  {
    // fild m32int
    top--;
    r_mov((long *)get_modrm(), st());
  }
}

static void emu_31()
{
  void emu_bad();
  emu_bad();
}

static void emu_32()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    //
    emu_bad();
  }
  else
  {
    // fist m32int
    r_mov(st(), (long *)get_modrm());
  }
}

static void emu_33()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    //
    emu_bad();
  }
  else
  {
    // fistp m32int
    r_mov(st(), (long *)get_modrm());
    st().tag = TW_E;
    top++;
  }
}

static void fclex()
{
  status_word &= ~(SW_B|SW_ES|SW_SF|SW_PE|SW_UE|SW_OE|SW_ZE|SW_DE|SW_IE);
}

static void finit()
{
  control_word = 0x037f;
  status_word = 0;
  top = 0;
  for (int r=0; r<8; r++)
  {
    regs[r].sign = 0;
    regs[r].tag = TW_E;
    regs[r].exp = 0;
    regs[r].sigh = 0;
    regs[r].sigl = 0;
  }
}

static FUNC emu_34_table[] = {
  emu_bad, emu_bad, fclex, finit, emu_bad, emu_bad, emu_bad, emu_bad
};

static void emu_34()
{
  if (modrm > 0277)
  {
    (emu_34_table[modrm&7])();
  }
  else
  {
    //
    emu_bad();
  }
}

static void emu_35()
{
  if (full())
    return;
  if (modrm > 0277)
  {
    emu_bad();
  }
  else
  {
    // fld m80real
    top--;
    r_mov((long double *)get_modrm(), st());
  }
}

static void emu_36()
{
  void emu_bad();
  emu_bad();
}

static void emu_37()
{
  if (modrm > 0277)
  {
    emu_bad();
  }
  else
  {
    // fstp m80real
    if (empty())
      return;
    r_mov(st(), (long double *)get_modrm());
    st().tag = TW_E;
    top++;
  }
}

static void emu_40()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fadd st(i),st
    int i = modrm & 7;
    if (empty(i))
      return;
    reg tmp;
    r_add(st(), st(i), tmp);
    st(i) = tmp;
    return;
  }
  else
  {
    // fadd m64real
    reg t1, t2;
    r_mov((double *)get_modrm(), t1);
    r_add(t1, st(), t2);
    st() = t2;
  }
}

static void emu_41()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fmul st(i),st
    int i = modrm & 7;
    if (empty(i))
      return;
    reg t;
    r_mul(st(i), st(), t);
    st(i) = t;
  }
  else
  {
    // fmul m64real
    reg t, t2;
    r_mov((double *)get_modrm(), t);
    r_mul(st(), t, t2);
    st() = t2;
  }
}

static void emu_42()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // 
    emu_bad();
  }
  else
  {
    // fcom m64real
    reg t;
    r_mov((double *)get_modrm(), t);
    int c = compare(st(), t);
    int f=0;
    if (c & COMP_NAN)
    {
      setcc(SW_C3|SW_C2|SW_C0);
      exception(EX_I);
    }
    else
      switch (c)
      {
        case COMP_A_LT_B:
          f = SW_C0;
          break;
        case COMP_A_EQ_B:
          f = SW_C3;
          break;
        case COMP_A_GT_B:
          f = 0;
          break;
        case COMP_NOCOMP:
          f = SW_C3 | SW_C2 | SW_C0;
          break;
      }
    setcc(f);
  }
}

static void emu_43()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    emu_bad();
  }
  else
  {
    // fcomp m64real
    reg t;
    r_mov((double *)get_modrm(), t);
    int c = compare(st(), t);
    st().tag = TW_E;
    top++;
    int f=0;
    if (c & COMP_NAN)
    {
      setcc(SW_C3|SW_C2|SW_C0);
      exception(EX_I);
    }
    else
      switch (c)
      {
        case COMP_A_LT_B:
          f = SW_C0;
          break;
        case COMP_A_EQ_B:
          f = SW_C3;
          break;
        case COMP_A_GT_B:
          f = 0;
          break;
        case COMP_NOCOMP:
          f = SW_C3 | SW_C2 | SW_C0;
          break;
      }
    setcc(f);
  }
}

static void emu_44()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fsub st(i),st
    int i = modrm & 7;
    if (empty(i))
      return;
    reg tmp;
    r_sub(st(), st(i), tmp);
    st(i) = tmp;
    return;
  }
  else
  {
    // fsub m64real
    reg t1, t2;
    r_mov((double *)get_modrm(), t1);
    r_sub(st(), t1, t2);
    st() = t2;
  }
}

static void emu_45()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fsubr st(i),st
    int i = modrm & 7;
    if (empty(i))
      return;
    reg tmp;
    r_sub(st(i), st(), tmp);
    st(i) = tmp;
    return;
  }
  else
  {
    // fsubr m64real
    reg t1, t2;
    r_mov((double *)get_modrm(), t1);
    r_sub(t1, st(), t2);
    st() = t2;
  }
}

static void emu_46()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fdivr st(i),st
    int i = modrm&7;
    if (empty(i))
      return;
    reg t;
    r_div(st(), st(i), t);
    st(i) = t;
  }
  else
  {
    // fdiv m64real
    reg t, t2;
    r_mov((double *)get_modrm(), t);
    r_div(st(), t, t2);
    st() = t2;
  }
}

static void emu_47()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fdiv st(i),st
    reg t;
    int i = modrm & 7;
    if (empty(i))
      return;
    r_div(st(i), st(0), t);
    st(i) = t;
  }
  else
  {
    // fdivr m64real
    reg t, t2;
    r_mov((double *)get_modrm(), t);
    r_div(t, st(), t2);
    st() = t2;
  }
}

static void emu_50()
{
  if (full())
    return;
  if (modrm > 0277)
  {
    // ffree st(i)
    int i = modrm & 7;
    st(i).tag = TW_E;
  }
  else
  {
    // fld m64real
    top--;
    r_mov((double *)get_modrm(), st());
  }
}

static void emu_51()
{
  void emu_bad();
  emu_bad();
}

static void emu_52()
{
  if (modrm > 0277)
  {
    st(modrm&7) = st();
  }
  else
  {
    // fst m64real
    if (empty())
      return;
    r_mov(st(), (double *)get_modrm());
  }
}

static void emu_53()
{
  if (modrm > 0277)
  {
    // fstp st(i)
    st(modrm&7) = st();
    st().tag = TW_E;
    top++;
  }
  else
  {
    // fstp m64real
    if (empty())
      return;
    r_mov(st(), (double *)get_modrm());
    st().tag = TW_E;
    top++;
  }
}

static void emu_54()
{
  if (modrm > 0277)
  {
    // fucom st(i)
    if (empty())
      return;
    if (empty(modrm&7))
    {
      setcc(SW_C3|SW_C2|SW_C0);
      return;
    }
    int c = compare(st(), st(modrm&7));
    int f=0;
    if (c & COMP_SNAN)
    {
      setcc(SW_C3|SW_C2|SW_C0);
      exception(EX_I);
    }
    else
      switch (c)
      {
        case COMP_A_LT_B:
          f = SW_C0;
          break;
        case COMP_A_EQ_B:
          f = SW_C3;
          break;
        case COMP_A_GT_B:
          f = 0;
          break;
        case COMP_NOCOMP:
          f = SW_C3 | SW_C2 | SW_C0;
          break;
      }
    setcc(f);
    
  }
  else
  {
    // frestor
    char *addr = (char *)get_modrm();
    int i, tag_word;

    control_word = *(int *)(addr+0) & 0xffff;
    status_word = *(int *)(addr+4) & 0xffff;
    tag_word = *(int *)(addr+8) & 0xffff;
    top = (status_word / SW_TOPS) & 3;
    for (i=0; i<8; i++)
    {
      r_mov((long double *)(addr + 0x1c + 10), st(i));
      st(i).tag = (tag_word >> (((i+top)&7)*2)) & 3;
    }
  }
}

static void emu_55()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fucomp st(i)
    if (empty(modrm&7))
    {
      setcc(SW_C3|SW_C2|SW_C0);
      return;
    }
    int c = compare(st(), st(modrm&7));
    st().tag = TW_E;
    top++;
    int f=0;
    if (c & COMP_SNAN)
    {
      setcc(SW_C3|SW_C2|SW_C0);
      exception(EX_I);
    }
    else
      switch (c)
      {
        case COMP_A_LT_B:
          f = SW_C0;
          break;
        case COMP_A_EQ_B:
          f = SW_C3;
          break;
        case COMP_A_GT_B:
          f = 0;
          break;
        case COMP_NOCOMP:
          f = SW_C3 | SW_C2 | SW_C0;
          break;
      }
    setcc(f);
    
  }
  else
  {
    emu_bad();
  }
}

static void emu_56() /* fsave */
{
  char *addr = (char *)get_modrm();
  int i, tag_word=0;

  status_word = status_word & ~SW_TOP;
  status_word += (top&7) * SW_TOPS;

  *(int *)(addr+0) = control_word & 0xffff;
  *(int *)(addr+4) = status_word & 0xffff;
  for (i=0; i<8; i++)
  {
    tag_word |= (st(i).tag << (((i+top)&7)*2));
    r_mov(st(i), (long double *)(addr + 0x1c + i*10));
  }
  *(int *)(addr+8) = tag_word & 0xffff;
}

static void emu_57()
{
  if (modrm > 0277)
  {
    //
    emu_bad();
  }
  else
  {
    // fstsw m2byte
    status_word &= ~SW_TOP;
    status_word |= (top&7) * SW_TOPS;
    *(short *)get_modrm() = status_word;
  }
}

static void emu_60()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // faddp st(i),st
    int i = modrm & 7;
    if (empty(i))
      return;
    reg tmp;
    r_add(st(), st(i), tmp);
    st(i) = tmp;
    st().tag = TW_E;
    top++;
    return;
  }
  else
  {
    // fiadd m16int
    reg t1, t2;
    r_mov((short *)get_modrm(), t1);
    r_add(st(), t1, t2);
    st() = t2;
  }
}

static void emu_61()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fmulp st(i),st
    int i = modrm & 7;
    if (empty(i))
      return;
    reg t;
    r_mul(st(i), st(), t);
    st(i) = t;
    st().tag = TW_E;
    top++;
  }
  else
  {
    // fimul m16int
    reg t, t2;
    r_mov((short *)get_modrm(), t);
    r_mul(st(), t, t2);
    st() = t2;
  }
}

static void emu_62()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    emu_bad();
  }
  else
  {
    // ficom m16int
    reg t;
    r_mov((short *)get_modrm(), t);
    int c = compare(st(), t);
    int f=0;
    if (c & COMP_NAN)
    {
      setcc(SW_C3|SW_C2|SW_C0);
      exception(EX_I);
    }
    else
      switch (c)
      {
        case COMP_A_LT_B:
          f = SW_C0;
          break;
        case COMP_A_EQ_B:
          f = SW_C3;
          break;
        case COMP_A_GT_B:
          f = 0;
          break;
        case COMP_NOCOMP:
          f = SW_C3 | SW_C2 | SW_C0;
          break;
      }
    setcc(f);
  }
}

static void emu_63()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fcompp
    if ((modrm&7) != 1)
    {
      emu_bad();
      return;
    }
    if (empty(1))
    {
      setcc(SW_C3|SW_C2|SW_C0);
      return;
    }
    int c = compare(st(), st(1));
    int f=0;
    st().tag = TW_E;
    top++;
    st().tag = TW_E;
    top++;
    if (c & COMP_NAN)
    {
      setcc(SW_C3|SW_C2|SW_C0);
      exception(EX_I);
    }
    else
      switch (c)
      {
        case COMP_A_LT_B:
          f = SW_C0;
          break;
        case COMP_A_EQ_B:
          f = SW_C3;
          break;
        case COMP_A_GT_B:
          f = 0;
          break;
        case COMP_NOCOMP:
          f = SW_C3 | SW_C2 | SW_C0;
          break;
      }
    setcc(f);
    
  }
  else
  {
    // ficomp m16int
    reg t;
    r_mov((short *)get_modrm(), t);
    int c = compare(st(), t);
    st().tag = TW_E;
    top++;
    int f=0;
    if (c & COMP_NAN)
    {
      setcc(SW_C3|SW_C2|SW_C0);
      exception(EX_I);
    }
    else
      switch (c)
      {
        case COMP_A_LT_B:
          f = SW_C0;
          break;
        case COMP_A_EQ_B:
          f = SW_C3;
          break;
        case COMP_A_GT_B:
          f = 0;
          break;
        case COMP_NOCOMP:
          f = SW_C3 | SW_C2 | SW_C0;
          break;
      }
    setcc(f);
  }
}

static void emu_64()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fsubp st(i),st
    int i = modrm & 7;
    if (empty(i))
      return;
    reg tmp;
    r_sub(st(), st(i), tmp);
    st(i) = tmp;
    st().tag = TW_E;
    top++;
    return;
  }
  else
  {
    // fisub m16int
    reg t1, t2;
    r_mov((short *)get_modrm(), t1);
    r_sub(st(), t1, t2);
    st() = t2;
  }
}

static void emu_65()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fsubr st(i),st
    int i = modrm & 7;
    if (empty(i))
      return;
    reg tmp;
    r_sub(st(i), st(), tmp);
    st(i) = tmp;
    st().tag = TW_E;
    top++;
    return;
  }
  else
  {
    // fisubr m16int
    reg t1, t2;
    r_mov((short *)get_modrm(), t1);
    r_sub(t1, st(), t2);
    st() = t2;
  }
}

static void emu_66()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fdivrp st(i),st
    int i = modrm & 7;
    if (empty(i))
      return;
    reg t;
    r_div(st(), st(i), t);
    st(i) = t;
    st().tag = TW_E;
    top++;
  }
  else
  {
    // fidiv m16int
    reg t, t2;
    r_mov((short *)get_modrm(), t);
    r_div(st(), t, t2);
    st() = t2;
  }
}

static void emu_67()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    // fdivp st(i),st
    reg t;
    int i = modrm & 7;
    if (empty(i))
      return;
    r_div(st(i), st(), t);
    st(i) = t;
    st().tag = TW_E;
    top++;
  }
  else
  {
    // fidiv m16int
    reg t, t2;
    r_mov((short *)get_modrm(), t);
    r_div(t, st(), t2);
    st() = t2;
  }
}

static void emu_70()
{
  if (full())
    return;
  if (modrm > 0277)
  {
    emu_bad();
  }
  else
  {
    // fild m16int
    top--;
    r_mov((short *)get_modrm(), st());
  }
}

static void emu_71()
{
  void emu_bad();
  emu_bad();
}


static void emu_72()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    //
    emu_bad();
  }
  else
  {
    // fist m16int
    r_mov(st(), (short *)get_modrm());
  }
}

static void emu_73()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    //
    emu_bad();
  }
  else
  {
    // fistp m16int
    r_mov(st(), (short *)get_modrm());
    st().tag = TW_E;
    top++;
  }
}

static void fstsw_ax()
{
  status_word &= ~SW_TOP;
  status_word |= (top&7) * SW_TOPS;
  eax &= 0xffff0000;
  eax |= status_word;
}

static FUNC emu_74_table[] = {
  fstsw_ax, emu_bad, emu_bad, emu_bad, emu_bad, emu_bad, emu_bad, emu_bad
};

static void emu_74()
{
  if (modrm > 0277)
  {
    (emu_74_table[modrm&7])();
  }
  else
  {
    // fbld m80dec
    if (full())
      return;
    top--;
    r_mov((char *)get_modrm(), st());
  }
}

static void emu_75()
{
  if (full())
    return;
  if (modrm > 0277)
  {
    emu_bad();
  }
  else
  {
    // fild m80int
    top--;
    r_mov((long long *)get_modrm(), st());
  }
}

static void emu_76()
{
  if (st().tag == TW_E)
    exception(EX_SU);
  if (modrm > 0277)
  {
    emu_bad();
  }
  else
  {
    // fbstp
    r_mov(st(), (char *)get_modrm());
    st().tag = TW_E;
    top++;
  }
}

static void emu_77()
{
  if (empty())
    return;
  if (modrm > 0277)
  {
    //
    emu_bad();
  }
  else
  {
    // fistp m32int
    r_mov(st(), (long long *)get_modrm());
    st().tag = TW_E;
    top++;
  }
}

#ifndef TEST
#define TEST 0
#endif

extern void emu_install();
extern void emu_printall();

#if TEST

double a=10, b=16;
float f=3.3;
int i=3;

static void test()
{
  asm("fldl _b");
  asm("fldl _a");
  emu_printall();
  asm("fdivr %st,%st(1)");
  emu_printall();
}

#endif

#if 0
main()
{
#if TEST
  test();
#endif
  emu_install();
#if TEST
  test();
#endif
}
#endif

#if 0
char saw[256*8];

/* zero here means invalid.  If first entry starts with '*', use st(i) */
/* no assumed %EFs here.  Indexed by rm(modrm()) */
char *f0[] = {0, 0, 0, 0, 0, 0, 0, 0};
char *fop_9[]  = { "*fxch st,%GF" };
char *fop_10[] = { "fnop", 0, 0, 0, 0, 0, 0, 0 };
char *fop_12[] = { "fchs", "fabs", 0, 0, "ftst", "fxam", 0, 0 };
char *fop_13[] = { "fld1", "fldl2t", "fldl2e", "fldpi",
                   "fldlg2", "fldln2", "fldz", 0 };
char *fop_14[] = { "f2xm1", "fyl2x", "fptan", "fpatan",
                   "fxtract", "fprem1", "fdecstp", "fincstp" };
char *fop_15[] = { "fprem", "fyl2xp1", "fsqrt", "fsincos",
                   "frndint", "fscale", "fsin", "fcos" };
char *fop_21[] = { 0, "fucompp", 0, 0, 0, 0, 0, 0 };
char *fop_28[] = { 0, 0, "fclex", "finit", 0, 0, 0, 0 };
char *fop_32[] = { "*fadd %GF,st" };
char *fop_33[] = { "*fmul %GF,st" };
char *fop_36[] = { "*fsubr %GF,st" };
char *fop_37[] = { "*fsub %GF,st" };
char *fop_38[] = { "*fdivr %GF,st" };
char *fop_39[] = { "*fdiv %GF,st" };
char *fop_40[] = { "*ffree %GF" };
char *fop_42[] = { "*fst %GF" };
char *fop_43[] = { "*fstp %GF" };
char *fop_44[] = { "*fucom %GF" };
char *fop_45[] = { "*fucomp %GF" };
char *fop_48[] = { "*faddp %GF,st" };
char *fop_49[] = { "*fmulp %GF,st" };
char *fop_51[] = { 0, "fcompp", 0, 0, 0, 0, 0, 0 };
char *fop_52[] = { "*fsubrp %GF,st" };
char *fop_53[] = { "*fsubp %GF,st" };
char *fop_54[] = { "*fdivrp %GF,st" };
char *fop_55[] = { "*fdivp %GF,st" };
char *fop_60[] = { "fstsw ax", 0, 0, 0, 0, 0, 0, 0 };

char **fspecial[] = { /* 0=use st(i), 1=undefined 0 in fop_* means undefined */
  0, 0, 0, 0, 0, 0, 0, 0,
  0, fop_9, fop_10, 0, fop_12, fop_13, fop_14, fop_15,
  f0, f0, f0, f0, f0, fop_21, f0, f0,
  f0, f0, f0, f0, fop_28, f0, f0, f0,
  fop_32, fop_33, f0, f0, fop_36, fop_37, fop_38, fop_39,
  fop_40, f0, fop_42, fop_43, fop_44, fop_45, f0, f0,
  fop_48, fop_49, f0, fop_51, fop_52, fop_53, fop_54, fop_55,
  f0, f0, f0, f0, fop_60, f0, f0, f0,
  };

char *floatops[] = { /* assumed " %EF" at end of each.  mod != 3 only */
/*00*/ "fadd", "fmul", "fcom", "fcomp",
       "fsub", "fsubr", "fdiv", "fdivr",
/*08*/ "fld", 0, "fst", "fstp",
       "fldenv", "fldcw", "fstenv", "fstcw",
/*16*/ "fiadd", "fimul", "ficomw", "ficompw",
       "fisub", "fisubr", "fidiv", "fidivr",
/*24*/ "fild", 0, "fist", "fistp",
       0, "fldt", 0, "fstpt",
/*32*/ "faddq", "fmulq", "fcomq", "fcompq",
       "fsubq", "fsubrq", "fdivq", "fdivrq",
/*40*/ "fldq", 0, "fstq", "fstpq",
       "frstor", 0, "fsave", "fstsww",
/*48*/ "fiaddw", "fimulw", "ficomw", "ficompw",
       "fisubw", "fisubrw", "fidivw", "fidivr",
/*56*/ "fildw", 0, "fistw", "fistpw",
       "fbldt", "fildq", "fbstpt", "fistpq"
  };
#endif

static FUNC esc_table[64] = {
  emu_00, emu_10, emu_20, emu_30, emu_40, emu_50, emu_60, emu_70,
  emu_01, emu_11, emu_21, emu_31, emu_41, emu_51, emu_61, emu_71,
  emu_02, emu_12, emu_22, emu_32, emu_42, emu_52, emu_62, emu_72,
  emu_03, emu_13, emu_23, emu_33, emu_43, emu_53, emu_63, emu_73,
  emu_04, emu_14, emu_24, emu_34, emu_44, emu_54, emu_64, emu_74,
  emu_05, emu_15, emu_25, emu_35, emu_45, emu_55, emu_65, emu_75,
  emu_06, emu_16, emu_26, emu_36, emu_46, emu_56, emu_66, emu_76,
  emu_07, emu_17, emu_27, emu_37, emu_47, emu_57, emu_67, emu_77,
};

extern "C" int _emu_entry(jmp_buf _exception_state);

int _emu_entry(jmp_buf _exception_state)
{
  int jmpval;
  eip = (unsigned char *) _exception_state->__eip;
  eax = _exception_state->__eax;
  ebx = _exception_state->__ebx;
  ecx = _exception_state->__ecx;
  edx = _exception_state->__edx;
  esi = _exception_state->__esi;
  edi = _exception_state->__edi;
  ebp = _exception_state->__ebp;
  esp = _exception_state->__esp;
  if (*eip == 0x66) // operand size - we know what size we need
    eip++;
  if (*eip == 0x9b) // fwait
  {
    eip++;
    _exception_state->__eip = (unsigned long) eip;
    return 0;
  }
  jmpval = setjmp(jumpbuf);
  if(jmpval)
  {
    _exception_state->__signum = 16; // simulate SIGFPE
    return 1;           /* Emulator failed for some reason */
  }
#if 0
  int see = ((int)(eip[0] & 7) << 8) | eip[1];
  if (saw[see] != 42)
  {
    eprintf("EMU387: %02x %02x %02x %02x - e%d%d", eip[0], eip[1], eip[2], eip[3], eip[0]&7, (eip[1]>>3)&7);
    eprintf(" s%d  ", eip[1]&7);
    int esc = ((eip[0]<<3)&070) | ((eip[1]>>3)&007);
    int modrm = eip[1];
    if ((modrm>>6) == 3)
      if (fspecial[esc])
        if (fspecial[esc][0] && (fspecial[esc][0][0] == '*'))
            eprintf("%s\r\n", fspecial[esc][0]+1);
        else if (fspecial[esc][modrm&7])
            eprintf("%s\r\n", fspecial[esc][modrm&7]);
        else
          eprintf("<invalid>\r\n");
      else
        eprintf("%s st(i)\r\n", floatops[esc]);
    else
      eprintf("%s st(i)\r\n", floatops[esc]);
    saw[see] = 42;
  }
#endif
  int esc_value = *eip++ & 7;
  modrm = *eip++;
  esc_value |= (modrm & 070);
  (esc_table[esc_value])();
//  eprintf("EIP: 0x%x TOP: %d\r\n", eip, top);
//  emu_printall();
  _exception_state->__eip = (unsigned long) eip;
  _exception_state->__eax = eax;
  return 0;
}
