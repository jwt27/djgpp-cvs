/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */

#include <libc/stubs.h>
#include <libc/symlink.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int __solve_dir_symlinks(const char * __symlink_path, char * __real_path)
{
   char   path_copy[FILENAME_MAX];
   char * last_part;

   /* Reject NULLs... */
   if (!__symlink_path || !__real_path)
   {
      errno = EINVAL;
      return -1;
   }
                                  
   /* ...and empty strings */
   if (__symlink_path[0] == '\0')
   {
      errno = ENOENT;
      return -1;
   }

   strcpy(path_copy, __symlink_path);
   last_part = basename(path_copy);
   if (*last_part == '\0')
   {
      /* If basename() returned pointer to the end of string, cut the last 
       * dir separator and try again. Exception: for paths like 'C:', just
       * copy it to the result and return.
       */ 
      if (*(last_part - 1) == ':')
      {
         strcpy(__real_path, path_copy);
         return 1;
      }
      *(last_part - 1) = '\0'; 
      last_part = basename(path_copy);
   }
   if (last_part == path_copy)
   {
      /* The path is made from single part */
      strcpy(__real_path, path_copy);
      return 1;
   }
   /* If the have path like c:/file or c:file, just copy it to the result
    * and return.
    */
   if (*(last_part - 1) == ':')
   {
      strcpy(__real_path, path_copy);
      return 1;
   }
   *(last_part - 1) = '\0';
   if (!__solve_symlinks(path_copy, __real_path))
      return 0;
   strcat(__real_path, "/");
   strcat(__real_path, last_part);
   return 1;
}
