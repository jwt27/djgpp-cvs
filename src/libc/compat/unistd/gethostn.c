/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <dpmi.h>
#include <go32.h>

static char pc_n[]= "pc";

int
gethostname (char *buf, int size)
{
  char *h, dosbuf[16];
  int len;
  __dpmi_regs r;

  /* Try asking [a lan extension of] dos for a name.  */
  r.x.ax = 0x5e00;
  r.h.ch = 0;  /* Try to detect overloading of 0x5e00  */
  r.x.dx = __tb & 15;
  r.x.ds = __tb / 16;
  __dpmi_int (0x21, &r);
  if ((r.x.flags & 1) || r.h.ch == 0)
  {
    /* Failed.  Try $HOSTNAME, then "pc".  */
    h = getenv ("HOSTNAME");
    if (h == 0)
      h = pc_n;
  }
  else
  {
    dosmemget (__tb, sizeof (dosbuf), dosbuf);
    h = dosbuf + strlen (dosbuf);
    while (h > dosbuf && h[-1] == ' ') h--;
    *h = 0;
    h = dosbuf;
  }

  len = strlen (h);
  if (len + 1 > size)
  {
    errno = ERANGE;
    return -1;
  }
  strcpy (buf, h);
  return 0;
}
