/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <time.h>
#include <errno.h>
#include <pc.h>
#include <libc/farptrgs.h>
#include <go32.h>
#include <dpmi.h>
#include <libc/bss.h>

static int uclock_bss = -1;

/* tics = about 18.2 * 65536 (1,192,755)

   actually, it's   0x1800b0 tics/day (FAQ)
                  / 24*60*60 sec/day
                  * 65536 utics/tic
		  = 1,193,180 utics/sec */

uclock_t
uclock(void)
{
  static uclock_t base = 0;
  static unsigned long last_tics = 0;
  unsigned char lsb, msb;
  unsigned long tics, otics;
  uclock_t rv;

  if (uclock_bss != __bss_count)
  {
    int e = errno;

    /* switch the timer to mode 2 (rate generator) */
    /* rather than mode 3 (square wave), which doesn't count linearly. */

    outportb(0x43, 0x34);
    outportb(0x40, 0xff);
    outportb(0x40, 0xff);

    base = 0;
    last_tics = 0;
    uclock_bss = __bss_count;

    /* It seems like Windows 9X virtualization of the timer device
       delays the actual execution of the above command until the
       next timer tick.  Or maybe it only consults the actual device
       once per tick.  In any case, the values returned during the
       first 55 msec after the timer was reprogrammed still look as
       if the timer worked in mode 3.  So we simply wait for one clock
       tick when we run on Windows.  */
    _farsetsel(_dos_ds);
    otics = _farnspeekl(0x46c);
    do {
      errno = 0;
      __dpmi_yield();	/* will set errno to ENOSYS on plain DOS */
    } while (errno == 0 && _farnspeekl(0x46c) == otics);
    errno = e;
  }

  /* Make sure the numbers we get are consistent */
  do {
    otics = _farpeekl(_dos_ds, 0x46c);
    outportb(0x43, 0x00);
    lsb = inportb(0x40);
    msb = inportb(0x40);
    tics = _farpeekl(_dos_ds, 0x46c);
  } while (otics != tics);

  /* calculate absolute time */
  msb ^= 0xff;
  lsb ^= 0xff;
  rv = ((uclock_t)tics << 16) | (msb << 8) | lsb;

  if (base == 0L)
    base = rv;

  if (last_tics > tics) /* midnight happened */
    base -= 0x1800b00000LL;

  last_tics = tics;

  /* return relative time */
  return rv - base;
}
