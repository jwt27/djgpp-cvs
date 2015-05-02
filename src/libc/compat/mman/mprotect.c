/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2007 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <sys/types.h>
#include <sys/mman.h>
#include <dpmi.h>
#include <crt0.h>
#include <errno.h>

int mprotect(void *addr, size_t len, int prot)
{
  unsigned start, end;
  unsigned npage;
  unsigned short newprot;

  start = ~0xfff & (unsigned)addr;
  end = ((unsigned)addr + len + 0xfff) & ~0xfff;
  npage = (end - start) / 0x1000;
  
  if(prot & PROT_WRITE)
    newprot = 9;		/* committed, writeable */
  else if(prot & PROT_READ)
    newprot = 1;		/* committed, read-only */
  else
    newprot = 0;		/* uncommitted */

  {
    unsigned short pageprot[npage];
    unsigned i;
    __dpmi_meminfo meminfo;
    __djgpp_sbrk_handle *handle_info;

    for(i=0;i < npage; i++)
      pageprot[i] = newprot;

    handle_info = __djgpp_memory_handle(start);
    meminfo.handle = handle_info->handle;
    meminfo.address = start - handle_info->address;
    meminfo.size = npage;

    i = __dpmi_set_page_attributes(&meminfo, (short *) pageprot);
    if(i)
      errno = EACCES;
    return i;
  }
}
