/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <errno.h>
#include <io.h>
#include <dpmi.h>
#include <unistd.h>

off_t
tell(int _file)
{
  return lseek(_file, 0, SEEK_CUR);
}
