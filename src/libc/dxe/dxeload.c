/* Copyright (C) 1995 Charles Sandmann (sandmann@clio.rice.edu)
   This software may be freely distributed with above copyright, no warranty.
   Based on code by DJ Delorie, it's really his, enhanced, bugs fixed. */

#include <io.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/dxe.h>

void *_dxe_load(char *name)
{
  dxe_header dh;
  char *data;
  int h;

  h = _open(name, 0);
  if (h < 0)
    return 0;
  _read(h, &dh, sizeof(dh));
  if (dh.magic != DXE_MAGIC)
  {
    _close(h);
    errno = ENOEXEC;
    return 0;
  }

  data = (char *)malloc(dh.element_size);
  if (data == 0)
  {
    _close(h);
    errno = ENOMEM;
    return 0;
  }

  _read(h, data, dh.element_size);

  {
    long relocs[dh.nrelocs];
    int i;
    _read(h, relocs, sizeof(long)*dh.nrelocs);
    _close(h);

    for (i=0; i<dh.nrelocs; i++)
      *(long *)(data + relocs[i]) += (int)data;
  }

  return (void *)(data + dh.symbol_offset);
}
