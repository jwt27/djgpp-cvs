/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define V2DBG 1

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
