/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <libc/symlink.h>
#include <sys/fsext.h>
#include <libc/fsexthlp.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <io.h>
#include <stdio.h>
#include <string.h>

#include "xsymlink.h"

/* Emulate symlinks for all files */
int symlink(const char *source, const char *dest)
{
   int         symlink_file;
   char        real_dest[FILENAME_MAX];
   int         ret;
   static char fill_buf[_SYMLINK_FILE_LEN + 1] =
                             "This is just a text to force symlink file to "
                             "be 510 bytes long. Do not delete it nor spaces "
                             "following it.";
   memset(fill_buf + strlen(fill_buf), ' ',
          _SYMLINK_FILE_LEN - strlen(fill_buf));

   /* Common error conditions */
   if (!source || !dest)
   {
      errno = EINVAL;
      return -1;
   }

   /* Provide ability to hook symlink support */
   if (__FSEXT_call_open_handlers_wrapper(__FSEXT_symlink, &ret, source, dest))
      return ret;


   /* The ``dest'' may have symlinks somewhere in the path itself.  */
   if (!__solve_symlinks(dest, real_dest))
      return -1; /* Errno (ELOOP) from __solve_symlinks() call.  */

   /* Check if there already is file with symlink's name */
   if (__file_exists(real_dest))
   {
      errno = EEXIST;
      return -1;
   }

   symlink_file = _creat(real_dest, 0);
   if (symlink_file < 0)
      return -1; /* Return errno from creat() call */
   write(symlink_file, _SYMLINK_PREFIX, _SYMLINK_PREFIX_LEN);
   write(symlink_file, source, strlen(source));
   write(symlink_file, "\n", 1);
   write(symlink_file, fill_buf, _SYMLINK_FILE_LEN - _SYMLINK_PREFIX_LEN - strlen(source) - 1);
   _close(symlink_file);

   return 0;
}
