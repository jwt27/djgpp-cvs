/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */

#include <libc/stubs.h>
#include <libc/symlink.h>
#include <unistd.h>

#include "xsymlink.h"

int readlink(const char * filename, char * buffer, size_t size)
{
   return __internal_readlink(filename, 0, buffer, size);
}

