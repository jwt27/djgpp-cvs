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

/* Symlink files have fixed length - 510 bytes. Why this value? Why not? */
/* It is big enough to hold longest possible path                        */
#define _SYMLINK_FILE_LEN 510

#endif /* #ifndef __XSYMLINK_H_ */

