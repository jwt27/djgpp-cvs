/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

/* Dummy routines to enable building of emu387.dxe from libemu.a.
 *
 * Since we can't do I/O, dummy the I/O routines out. Since we can't link
 * against a profiled libc, dummy the profiling functions out.
 */

#include <stdio.h>
#include <stdarg.h>
#include <io.h>
#include <libc/dosio.h>
   
int
vsnprintf (char *buf, size_t buflen, const char *fmt, va_list args)
{
  return 0;
}

ssize_t
_write (int fd, const void *buf, size_t nbyte)
{
  return 0;
}

void mcount (int _to);

void
mcount (int _to)
{
}

int __djgpp_exception_state_ptr;
   
/* The _emu_dummy is just there to bring in __emu_entry from the library. */
/* void _emu_dummy(void) { _emu_entry(); } */
