/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * BIOSDISK.C.
 *
 * Modified by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <libc/stubs.h>
#include <bios.h>
#include <go32.h>
#include <dpmi.h>
#include <stdlib.h>

static int dos_segment = 0;
static int dos_selector = 0;

static void free_dos_buffer(void);
static void
free_dos_buffer(void)
{
  __dpmi_free_dos_memory(dos_selector);
  dos_segment = dos_selector = 0;
}

static void alloc_dos_buffer(void);
static void
alloc_dos_buffer(void)
{
  if (dos_segment)
    return;
  dos_segment = __dpmi_allocate_dos_memory(18*512/16, &dos_selector);
  if (dos_segment == -1)
  {
    dos_segment = 0;
    return;
  }
  atexit(free_dos_buffer);
}

int
biosdisk(int cmd, int drive, int head, int track,
	 int sector, int nsects, void *buffer)
{
  int seg=0, ofs=0, before=0;
  size_t xfer=0;
  __dpmi_regs r;
  switch (cmd)
  {
  case 2:
    xfer = 512 * nsects;
    before = 0;
    break;
  case 3:
    xfer = 512 * nsects;
    before = 1;
    break;
  case 5:
    xfer = 2 * 256;
    before = 1;
    break;
  case 0x0a:
    xfer = (512+7) * nsects;
    before = 0;
    break;
  case 0x0b:
    xfer = (512+7) * nsects;
    before = 1;
    break;
  case 0x0e:
    xfer = 512;
    before = 0;
    break;
  case 0x0f:
    xfer = 512;
    before = 1;
    break;
  }
  if (xfer)
  {
    if (xfer > 18*512)
      return 1;			/* bad command */
    if (xfer > __tb_size)
    {
      alloc_dos_buffer();
      if (dos_segment == 0)
	return 0xbb;		/* undefined error */
      seg = dos_segment;
      ofs = 0;
    }
    else
    {
      seg = __tb >> 4;
      ofs = __tb & 15;
    }
  }
  r.h.ah = cmd;
  r.h.al = nsects;
  r.x.es = seg;
  r.x.bx = ofs;
  r.h.ch = track & 0xff;
  r.h.cl = sector | ((track >> 2) & 0xc0);
  r.h.dh = head;
  r.h.dl = drive;
  if (xfer && before)
    dosmemput(buffer, xfer, seg*16+ofs);
  __dpmi_int(0x13, &r);
  if (xfer && !before)
    dosmemget(seg*16+ofs, xfer, buffer);
  if (cmd == 0x08)
  {
    ((short *)buffer)[0] = r.x.cx;
    ((short *)buffer)[1] = r.x.dx;
  }
  return r.h.ah;
}

unsigned 
_bios_disk(unsigned _cmd, struct diskinfo_t *_di)
{
  int seg=0, ofs=0, before=0;
  size_t xfer=0;
  __dpmi_regs r;

  switch( _cmd )
  {
  case 2:
    xfer = 512 * _di->nsectors;
    before = 0;
    break;
  case 3:
    xfer = 512 * _di->nsectors;
    before = 1;
    break;
  case 5:
    xfer = 2 * 256;
    before = 1;
    break;
  }
  if (xfer)
  {
    if (xfer > 18*512)
      return 1;			/* bad command */
    if (xfer > __tb_size)
    {
      alloc_dos_buffer();
      if (dos_segment == 0)
        return 0xbb;      /* undefined error */
      seg = dos_segment;
      ofs = 0;
    }
    else
    {
      seg = __tb >> 4;
      ofs = __tb & 15;
    }
  }
  r.h.ah = _cmd;
  r.h.al = _di->nsectors;
  r.x.es = seg;
  r.x.bx = ofs;
  r.h.ch = _di->track & 0xff;
  r.h.cl = _di->sector | ((_di->track >> 2) & 0xc0);
  r.h.dh = _di->head;
  r.h.dl = _di->drive;
  if (xfer && before)
    dosmemput(_di->buffer, xfer, seg*16+ofs);
  __dpmi_int(0x13, &r);
  if (xfer && !before)
    dosmemget(seg*16+ofs, xfer, _di->buffer);
  return r.x.ax;
}
