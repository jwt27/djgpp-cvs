/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <libc/symlink.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <sys/fsext.h>
#include <libc/fsexthlp.h>
 
int
chmod(const char *filename, int pmode)
{
  int dmode;
  char real_name[FILENAME_MAX];
  int attr;
  int rv;

  if (!__solve_symlinks(filename, real_name))
     return -1;

  /* See if a file system extension has a hook for this file. */
  if (__FSEXT_call_open_handlers_wrapper(__FSEXT_chmod, &rv,
					 real_name, pmode))
    return rv;
  
  attr = _chmod(real_name, 0, 0);

  if (attr == -1)
    return -1;
 
  if(pmode & S_IWUSR)           /* Only implemented toggle is write/nowrite */
    dmode = 0;                  /* Normal file */
  else
    dmode = 1;                  /* Readonly file */
 
  /* Must clear the directory and volume bits, otherwise 214301 fails.
     Unused bits left alone (some network redirectors use them).  */
  if (_chmod(real_name, 1, (attr & 0xffe6) | dmode) == -1)
    return -1;
  return 0;
}

