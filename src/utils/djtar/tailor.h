/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* tailor.h -- target dependent definitions
 * Copyright (C) 1992-1993 Jean-loup Gailly.
 *
 * Severely butchered by Eli Zaretskii to only define things we need
 * when compiling DJTAR under DJGPP v2.0 and later.
 *
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License, see the file COPYING.
 */

/* The target dependent definitions should be defined here only.
 * The target dependent functions should be defined in tailor.c.
 */

/* $Id: tailor.h,v 1.1 1995/11/17 01:01:20 dj Exp $ */

#if defined(__MSDOS__) && !defined(MSDOS)
#  define MSDOS
#endif

#define MAX_PATH_LEN  128

	/* Common defaults */

#ifndef RECORD_IO
#  define RECORD_IO 0
#endif

#ifndef OPEN
#  define OPEN(name, flags, mode) open(name, flags, mode)
#endif

#ifndef get_char
#  define get_char() get_byte()
#endif

#ifndef put_char
#  define put_char(c) put_byte(c)
#endif
