/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define V2DBG 1

typedef struct {
  word16 sig0;
  word16 sig1;
  word16 sig2;
  word16 sig3;
  word16 exponent:15;
  word16 sign:1;
} NPXREG;

typedef struct {
  word32 control;
  word32 status;
  word32 tag;
  word32 eip;
  word32 cs;
  word32 dataptr;
  word32 datasel;
  NPXREG reg[8];
} NPX;

typedef struct GDT_S {
word16 lim0;
word16 base0;
word8 base1;
word8 stype;	/* type, DT, DPL, present */
word8 lim1;	/* limit, granularity */
word8 base2;
} GDT_S;

typedef struct IDT_S {
  word16 offset0;
  word16 selector;
  word16 stype;
  word16 offset1;
} IDT;
