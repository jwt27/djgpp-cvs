/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <sys/exceptn.h>
#include <dpmi.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>

#include <libc/dosio.h>
#include <libc/ttyprvt.h>
#include <libc/getdinfo.h>

void (*__setmode_stdio_hook)(int fd, int mode); /* BSS to zero */

int
setmode(int handle, int mode)
{
  __dpmi_regs regs;
  int oldmode, newmode;

  oldmode = newmode = _get_dev_info(handle);
  if (oldmode == -1)
     return -1;

  if (mode & O_BINARY)
    newmode |= _DEV_RAW;
  else
    newmode &= ~_DEV_RAW;

  if (oldmode & _DEV_CDEV)	/* Only for character dev */
  {
    regs.x.ax = 0x4401;
    regs.x.bx = handle;
    regs.x.dx = newmode & 0xff;   		/* Force upper byte zero */
    __dpmi_int(0x21, &regs);
    if (regs.x.flags & 1)
    {
      errno = __doserr_to_errno(regs.x.ax);
      return -1;
    }
    if (handle == 0)
      __djgpp_set_ctrl_c(!(mode & O_BINARY));
  }

  oldmode = __file_handle_modes[handle];
  newmode = (oldmode & ~(O_BINARY|O_TEXT)) | (mode & (O_BINARY|O_TEXT));
  __file_handle_set (handle, newmode);
  oldmode &= (O_BINARY|O_TEXT);

  return oldmode;
}
