/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef _ED_H_
#define _ED_H_

#include <debug/v2load.h>
#include <debug/dbgcom.h>

typedef unsigned long word32;
typedef unsigned short word16;
typedef unsigned char word8;
typedef signed long int32;
typedef signed short int16;
typedef signed char int8;

#define A_black		0
#define A_blue		1
#define A_green		2
#define A_cyan		3
#define A_red		4
#define	A_purple	5
#define A_brown		6
#define A_grey		7
#define A_bold		8
#define A_yellow	14
#define A_white		15

void ansi(int fg);
void ansidetect(void);

#endif
