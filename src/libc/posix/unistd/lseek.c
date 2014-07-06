/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <go32.h>
#include <dpmi.h>
#include <sys/fsext.h>
#include <libc/fsexthlp.h>
#include <libc/dosio.h>
#include <libc/fd_props.h>

off_t
lseek(int handle, off_t offset, int whence)
{
  offset_t llseek_offset;

  __FSEXT_Function *func = __FSEXT_get_function(handle);
  if (func)
  {
    int rv;
    if (__FSEXT_func_wrapper(func, __FSEXT_lseek, &rv, handle, offset, whence))
      return rv;
  }

  llseek_offset = llseek(handle, offset, whence);
  if (llseek_offset == -1)
  {
    return -1; /* llseek sets errno. */
  }

  /* Here we rely on that llseek()'s return range is [-1, 2^32-2]. 
   * All this removal of the bits 31-63 (of which only bit 31
   * potentially could be set) in the long long (offset_t) and then
   * merging it into the long (off_t) is because if a value is
   * (de)promoted and it doesn't fit in the target variable
   * implementation defined behaviour is invoked. So we make sure it
   * temporarily fits. After that it's ok to or in the sign bit
   * again. 
   */
  if (llseek_offset & 0x80000000)
  {
    return ((off_t)(llseek_offset & 0x7fffffff)) | 0x80000000;
  }
  else
  {
    return llseek_offset & 0x7fffffff;
  }
}
