/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */

#include <libc/stubs.h>
#include <libc/symlink.h>
#include <stdio.h>
#include <unistd.h>

#include "xsymlink.h"

int readlink(const char * filename, char * buffer, size_t size)
{
   char real_name[FILENAME_MAX];
   if (!__solve_dir_symlinks(filename, real_name))
      return -1;
   return __internal_readlink(real_name, 0, buffer, size);
}

