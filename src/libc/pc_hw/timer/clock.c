/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <time.h>
#include <go32.h>
#include <libc/farptrgs.h>
#include <libc/bss.h>

static int clock_bss = -1;

clock_t
clock(void)
{
  static clock_t base = 0;
  static unsigned long last_tics = 0;
  unsigned long tics, otics;
  clock_t rv;

  if (clock_bss != __bss_count)
  {
    clock_bss = __bss_count;
    last_tics = 0;
    base = 0;
  }

  /* Make sure the numbers we get are consistent */
  do {
    otics = _farpeekl(_dos_ds, 0x46c);
    tics = _farpeekl(_dos_ds, 0x46c);
  } while (otics != tics);

  rv = tics;

  if (base == 0L)
    base = rv;

  if (last_tics > tics) /* midnight happened */
    base -= 0x1800b0;

  last_tics = tics;

  /* return relative time */
  /* The 5 matches the scale for CLOCKS_PER_SEC */
  return (rv - base) * 5;
}
