/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <errno.h>
#include <dpmi.h>
#include <go32.h>
#include <dos.h>

int uname(struct utsname *u)
{
  __dpmi_regs r;
  unsigned short dos_version;

  if (!u)
  {
    errno = EFAULT;
    return -1;
  }

  dos_version = _get_dos_version(1);
  strncpy(u->sysname, _os_flavor, sizeof(u->sysname) - 1);
  u->sysname[sizeof(u->sysname) - 1] = '\0';
  sprintf(u->release, "%d", dos_version >> 8);
  sprintf(u->version, "%02d", dos_version & 0xff);
  strcpy(u->machine, "pc");

  r.x.ax = 0x5e00;
  r.x.ds = __tb >> 4;
  r.x.dx = __tb & 15;
  __dpmi_int(0x21, &r);
  if ((r.x.flags & 1) || (r.h.ch == 0))
    strcpy(u->nodename, "pc");
  else
  {
    int i = 8;
    dosmemget(__tb, 8, u->nodename);
    do {
      u->nodename[i--] = 0;
    } while (i && u->nodename[i] <= ' ');
  }
  return 0;
}
