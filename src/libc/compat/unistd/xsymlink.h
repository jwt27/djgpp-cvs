/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Symlink support by Laurynas Biveinis                      */

/* Internal header containing symlink file format specifiers     */
/* I decided not to put it in public header, because this prefix */
/* isn't guaranteed not to change.                               */
#ifndef __XSYMLINK_H_
#define __XSYMLINK_H_

/* Current prefix is for being compatible with CygWin symlinks */
#define _SYMLINK_PREFIX "!<symlink>"
#define _SYMLINK_PREFIX_LEN (sizeof(_SYMLINK_PREFIX) - 1)

#endif /* #ifndef __XSYMLINK_H_ */

