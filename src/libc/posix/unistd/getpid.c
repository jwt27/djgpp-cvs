/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <go32.h>
#include <libc/farptrgs.h>
#include <libc/bss.h>

static int pid_count = -1;
static pid_t my_pid;

pid_t
getpid(void)
{
  if (pid_count != __bss_count)
  {
    pid_count = __bss_count;
    my_pid = _farpeekw(_dos_ds, 0x46c);
    my_pid &= 0x3fff;
    my_pid |= 0x1000;
  }

  return my_pid;
}
