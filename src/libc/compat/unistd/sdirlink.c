/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
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
   int    is_root = 0;

   /* Reject NULLs... */
   if (!__symlink_path || !__real_path)
   {
      errno = EINVAL;
      return 0;
   }
                                  
   /* ...and empty strings */
   if (__symlink_path[0] == '\0')
   {
      errno = ENOENT;
      return 0;
   }

   /* Handle root directories */
   if (   ((__symlink_path[0] == '/') || (__symlink_path[0] == '\\'))
       && (__symlink_path[1] == '\0'))
     is_root = 1;

   if (   __symlink_path[0]
       && (__symlink_path[1] == ':')
       && ((__symlink_path[2] == '/') || (__symlink_path[2] == '\\'))
       && (__symlink_path[3] == '\0'))
     is_root = 1;

   if (is_root)
   {
      strcpy(__real_path, __symlink_path);
      return 1;
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

#ifdef TEST

#include <stdlib.h>

int main(int argc, char *argv[])
{
  char buf[FILENAME_MAX];
  int i;

  puts("User requested tests:");
  for (i = 1; i < argc; i++) {
    __solve_dir_symlinks(argv[i], buf);
    printf("%s -> %s\n", argv[i], buf);
  }

  return EXIT_SUCCESS;
}
#endif
