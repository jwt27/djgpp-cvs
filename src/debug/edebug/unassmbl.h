/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1993 DJ Delorie, see COPYING.DJ for details */
/* This is file UNASSMBL.H */
/*
** Copyright (C) 1993 DJ Delorie, 334 North Rd, Deerfield NH 03037-1110
**
** This file is distributed under the terms listed in the document
** "copying.dj", available from DJ Delorie at the address above.
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef _UNASSMBL_H_
#define _UNASSMBL_H_

word32 unassemble(word32 v, int showregs);
extern int last_unassemble_unconditional;
extern int last_unassemble_jump;
extern int last_unassemble_extra_lines;

#endif
