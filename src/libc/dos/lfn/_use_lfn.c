/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <dpmi.h>
#include <go32.h>
#include <crt0.h>
#include <libc/dosio.h>
#include <libc/bss.h>

static int use_lfn_bss_count = -1;
static char use_lfn = 2;	/* 0 = no, 1 = yes, 2 = find out */

char _use_lfn(void)
{
  if (use_lfn_bss_count != __bss_count)
  {
    __dpmi_regs r;
    char *lfnenv;

    use_lfn_bss_count = __bss_count;

    if(_crt0_startup_flags & _CRT0_FLAG_NO_LFN) {
      use_lfn = 0;
      return 0;
    }

    lfnenv = getenv("LFN");
    if(lfnenv && (tolower(lfnenv[0]) == 'n'))
    {
      _crt0_startup_flags |= _CRT0_FLAG_NO_LFN;
      use_lfn = 0;
      return 0;
    }
     
    r.x.ax = 0x7147;
    r.x.dx = 0;						/* Current drive */
    r.x.si = __tb_offset + _go32_info_block.size_of_transfer_buffer - FILENAME_MAX;	/* end */
    r.x.ds = __tb_segment;
    r.x.ss = r.x.sp = 0;
    r.x.flags = 1;		/* Set the carry */
    __dpmi_simulate_real_mode_interrupt(0x21, &r);
    if(r.x.ax == 0x7100 || r.x.flags & 1)
    {
      _crt0_startup_flags |= _CRT0_FLAG_NO_LFN;
      use_lfn = 0;
    } else
      use_lfn = 1;
  }
  return use_lfn;
}
