/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
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

/* Modified by Morten Welinder, terra@diku.dk, for use with full screen
   debugger.  These changes are copyright 1994 by Morten Welinder.  */

#ifndef _UNASSMBL_H_
#define _UNASSMBL_H_

#ifdef FULLSCR
char *unassemble_proper (word32, int *);
char *unassemble_source (word32 v);
void put_source_line(int, char *, int, char *);
int file_line_count (char *);
FILE *cache_fopen(char *name);
char *source_path;
#else
word32 unassemble(word32 v, int showregs);
#endif
extern int last_unassemble_unconditional;
extern int last_unassemble_jump;
extern int last_unassemble_extra_lines;

#endif
