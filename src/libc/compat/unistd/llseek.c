/*
 * File llseek.c.
 *
 * Copyright (C) 2000 Martin Str@"omberg <ams@ludd.luth.se>.
 *
 * This software may be used freely so long as this copyright notice is
 * left intact. There is no warranty on this software.
 *
 */

#include <libc/stubs.h>
#include <unistd.h>
#include <dpmi.h>
#include <errno.h>
#include <libc/dosio.h>
#include <sys/fsext.h>


offset_t
llseek( int handle, offset_t offset, int whence )
{
  __dpmi_regs r;

  __FSEXT_Function *func = __FSEXT_get_function(handle);
  if( func )
  {
    int rv;
    if( func(__FSEXT_llseek, &rv, &handle) )
    {
      return rv;
    }
  }

  r.h.ah = 0x42;
  r.h.al = whence;
  r.x.bx = handle;
  r.x.cx = offset >> 16;
  r.x.dx = offset & 0xffff;
  __dpmi_int(0x21, &r);
  if( r.x.flags & 1 )
  {
    errno = __doserr_to_errno(r.x.ax);
    return -1;
  }
  return( ( ( (unsigned)r.x.dx ) << 16) + r.x.ax );
}

