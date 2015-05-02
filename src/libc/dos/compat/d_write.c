/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2007 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_WRITE.C.
 *
 * Written by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <libc/stubs.h>
#include <libc/dosio.h>
#include <go32.h>
#include <dpmi.h>
#include <errno.h>
#include <dos.h>

unsigned int _dos_write(int handle, const void *buffer, unsigned int count, unsigned int *result)
{
  __dpmi_regs r;
  int dos_segment;
  int dos_selector;
  unsigned int dos_buffer_size, write_size;
  const unsigned char *p_buffer;

  /* Allocates ~64K or less transfer buffer from DOS */
  dos_buffer_size = ( count < 0xFFE0 ) ? count : 0xFFE0;
  if ( (dos_segment=__dpmi_allocate_dos_memory((dos_buffer_size + 15) >> 4, &dos_selector)) == -1 )
  {
    errno = ENOMEM;
    return 8;
  }

  /* Reads blocks from user buffer and writes to file. */
  p_buffer = (const unsigned char *)buffer;
  *result  = 0;
  while( count )
  {
    write_size = ( count < dos_buffer_size ) ? count : dos_buffer_size;
    movedata(_my_ds(), (unsigned int)p_buffer, dos_selector, 0, write_size);
    r.h.ah = 0x40;
    r.x.bx = handle;
    r.x.cx = write_size;
    r.x.ds = dos_segment;
    r.x.dx = 0;
    __dpmi_int(0x21, &r);
    if ( r.x.flags & 1 )
    {
      __dpmi_free_dos_memory(dos_selector);
      errno = __doserr_to_errno(r.x.ax);
      return r.x.ax;
    }
    count    -= write_size;
    p_buffer += write_size;
    *result  += r.x.ax;
  }

  /* Frees allocated DOS transfer buffer. */
  __dpmi_free_dos_memory(dos_selector);
  return 0;
}
