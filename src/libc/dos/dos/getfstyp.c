/*
 * File getfstyp.c.
 *
 * Copyright (C) 2001 Martin Str@"omberg <ams@ludd.luth.se>.
 * Copyright (C) 2000 Martin Str@"omberg <ams@ludd.luth.se>.
 *
 * This software may be used freely so long as this copyright notice is
 * left intact. There is no warranty on this software.
 *
 */

#include <dos.h>
#include <errno.h>
#include <go32.h>
#include <dpmi.h>
#include <libc/dosio.h>
#include <libc/farptrgs.h>

/* Returns: -1 == error; 0 == result_str filled in. */
int 
_get_fs_type( const int drive /* drive number (1=A:). */
	    , char *const result_str  /* String to put result in. At least 9 chars long. */
	     )
{
  int n;
  __dpmi_regs r;

  /* Check DOZE version and return -1 if too low. */
  if( _osmajor < 4 )
  {
    errno = ENOSYS;
    return( -1 );
  }

  /* Call INT21, ax==0x6900 i.e. Get Disk Serial Number (sic!). */
  r.x.ax = 0x6900;
  r.h.bl = drive; 
  r.h.bh = 0;
  r.x.ds = __tb >> 4;
  r.x.dx = __tb & 0x0f;
  __dpmi_int(0x21, &r);
  if( (r.x.flags & 1) == 0 )
  {
    /* Get the file system type. */
    for(n = 0; n < 8; n++)
    {
      result_str[n] = _farpeekb( _dos_ds, __tb + 0x11 + n);
    }
    result_str[8] = 0;

    /* Remove terminating spaces. */
    for(n = 7; n && result_str[n] == ' '; n-- )
    {
      result_str[n] = 0;
    }

    return( 0 );
  }

  errno = __doserr_to_errno(r.x.ax);
  return( -1 );

}
