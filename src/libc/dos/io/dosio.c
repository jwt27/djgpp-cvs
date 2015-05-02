/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dpmi.h>

#include <libc/dosio.h>
#include <libc/bss.h>

static char init_file_handle_modes[20] = {
  O_TEXT,
  O_TEXT,
  O_TEXT,
  O_BINARY,
  O_BINARY
};

static int dosio_bss_count = -1;
static size_t count=20;	/* DOS default limit */

char *__file_handle_modes = init_file_handle_modes;

void
__file_handle_set(int fd, int mode)
{
  __dpmi_regs r;

  if (dosio_bss_count != __bss_count)
  {
    dosio_bss_count = __bss_count;
    count = 20;
    __file_handle_modes = init_file_handle_modes;
  }

  /* Check for bogus values */
  if (fd < 0)
    return;

  /* See if we need to expand the tables.  Check this BEFORE it might fail,
     so that when we hit the count'th request, we've already up'd it. */
  if ((size_t)fd >= (count-1) && count < 255)
  {
    int oldcount = count;
    count = 255;

    __file_handle_modes = (char *)malloc(count * sizeof(*__file_handle_modes));
    memcpy(__file_handle_modes, init_file_handle_modes, sizeof(init_file_handle_modes));
    memset(__file_handle_modes + oldcount, 0, (count-oldcount)*sizeof(__file_handle_modes[0]));

    /* Tell DOS to allow more */
    r.h.ah = 0x67;
    r.x.bx = count;
    __dpmi_int(0x21, &r);
  }

  /* Fill in the value */
  __file_handle_modes[fd] = mode;
}
