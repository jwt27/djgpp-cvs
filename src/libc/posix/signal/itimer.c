/* Copyright (C) 1995 Charles Sandmann (sandmann@clio.rice.edu)
   setitimer implmentation - used for profiling and alarm
   BUGS: ONLY ONE AT A TIME, first pass code
   This software may be freely distributed, no warranty.

   Changed to work with SIGALRM & SIGPROF by Tom Demmer.
   Gotchas:
     - It relies on uclock(), which does not work under Windows 95.
     - It screws up debuggers for reasons I cannot figure out.
     - Both is true for the old version, too.
*/

#include <libc/stubs.h>
#include <sys/time.h>
#include <errno.h>
#include <dpmi.h>
#include <signal.h>
#include <go32.h>

static uclock_t r_exp, r_rel,  /* When REAL expires & reload value */
                p_exp, p_rel;  /* When PROF expires & reload value */

static uclock_t u_now;

int
getitimer(int which, struct itimerval *value)
{
  uclock_t expire, reload;

  u_now = uclock();
  if (which == ITIMER_REAL)
  {
    if (r_exp)
    {
      expire = r_exp - u_now;
      reload = r_rel;
    }
    else
      expire = reload = 0;
  }
  else if (which == ITIMER_PROF)
  {
    if (p_exp)
    {
      expire = p_exp - u_now;
      reload = p_rel;
    }
    else
      expire = reload = 0;
  }
  else
  {
    errno = EINVAL;
    return -1;
  }
  value->it_value.tv_sec    = expire / UCLOCKS_PER_SEC;
  value->it_value.tv_usec   = (expire % UCLOCKS_PER_SEC)*3433/4096;
  value->it_interval.tv_sec = reload / UCLOCKS_PER_SEC;
  value->it_interval.tv_usec= (reload % UCLOCKS_PER_SEC)*3433/4096;
  return 0;
}

extern unsigned __djgpp_timer_countdown;
extern __dpmi_paddr __djgpp_old_timer;
extern int __djgpp_timer_hdlr;
static char timer_on = 0;

/* Set back IRQ2 handler to default values and disable own signal
   handler */
static void
stop_timer(void)
{
  if(!timer_on)
    return;
  __djgpp_timer_countdown = -1;
  __dpmi_set_protected_mode_interrupt_vector(8, &__djgpp_old_timer);
  timer_on = 0;
  signal(SIGTIMR, SIG_DFL);
}

/* Returns the time to the next event in UCLOCK_PER_SEC u_now must be
   set by calling routine.  Return 0 if no event pending. */

static inline uclock_t
GetNextEvent(void)
{
  if (r_exp && p_exp)
     return (r_exp < p_exp ? r_exp - u_now : p_exp - u_now );
  else if (r_exp)
     return  r_exp - u_now;
  else if (p_exp)
     return p_exp - u_now;
  else
     return 0;
}

/* Handler for SIGTIMR */
static void
timer_action(int signum)
{
  int do_tmr=0,do_prof=0;
  uclock_t next;

  u_now = uclock();

  /* Check the real timer */
  if (r_exp && (r_exp <= u_now) )
  {
    do_tmr = 1;
    if (r_rel)
      r_exp += r_rel;
    else
      r_exp = 0;
  }

  /* Check profile timer */
  if (p_exp && (p_exp <= u_now))
  {
    do_prof = 1;
    if (p_rel)
      p_exp += p_rel;
    else
      p_exp = 0;
  }

  /* Now we have to schedule the next interrupt, if any pending */
  if (do_tmr || do_prof)
  {
    if ((next = GetNextEvent()) != 0)
    {
      next /= 65536L;
      /* Why do I subtract 1 from NEXT below?  Because the timer
	 interrupt handler (see exceptn.S) checks whether the
	 countdown variable is zero *before* it decrements it.  So
	 setting it to zero means the timer will expire on the next
	 tick, which is exactly what we want.

	 Note also that NEXT might be negative if the timer just
	 fired, and if the reload value is smaller than u_now - X_exp.
	 We treat that as if NEXT were zero, meaning that the timer
	 will expire on the next tick.  */
      __djgpp_timer_countdown = next > 0 ? next - 1 : 0 ;
    }
    else
      stop_timer();

    if (do_tmr)
      raise(SIGALRM);
    if (do_prof)
      raise(SIGPROF);
  }
}

static void
start_timer(void)
{
  uclock_t next;
  __dpmi_paddr int8;

  next = GetNextEvent();
  next /= 65536L;
  /* See the commentary above about subtracting 1 from NEXT, and about
     negative values being returned by GetNextEvent.  */
  __djgpp_timer_countdown = next > 0 ? next - 1 : 0;

  if (timer_on)
    return;

  timer_on = 1;
  signal(SIGTIMR, timer_action);
  __dpmi_get_protected_mode_interrupt_vector(8, &__djgpp_old_timer);
  int8.selector = _my_cs();
  int8.offset32 = (unsigned) &__djgpp_timer_hdlr;
  __dpmi_set_protected_mode_interrupt_vector(8, &int8);
}


int
setitimer(int which, struct itimerval *value, struct itimerval *ovalue)
{
  uclock_t *t_exp, *t_rel;

  if (ovalue)
  {
    if (getitimer(which,ovalue)) /* also sets u_now */
      return -1;  /* errno already set */
  }
  else
    u_now = uclock();

  if ((which != ITIMER_REAL) && ( which != ITIMER_PROF ) )
  {
    errno = EINVAL;
    return -1;
  }

  t_exp = which == ITIMER_REAL ? &r_exp: &p_exp;
  t_rel = which == ITIMER_REAL ? &r_rel: &p_rel;

  if ((value->it_value.tv_sec|value->it_value.tv_usec)==0 )
  {
    /* Disable this timer */
    *t_exp = *t_rel = 0;
    /* If both stopped, stop timer */
    if (( p_exp | r_exp ) == 0 )
    {
      stop_timer();
      return 0;
    }
  }

  *t_exp = value->it_value.tv_sec    * UCLOCKS_PER_SEC;
  *t_rel = value->it_interval.tv_sec * UCLOCKS_PER_SEC;

  /* Rounding errors ?? First multiply and then divide gives an
     overflow if the USEC member is larger than 524288. */
  if (value->it_value.tv_usec < 524200)
    *t_exp += (value->it_value.tv_usec * 4096) / 3433;
  else
    *t_exp += (value->it_value.tv_usec * 2048) / 1716;

  if (value->it_interval.tv_usec < 524200)
    *t_rel += (value->it_interval.tv_usec * 4096) / 3433;
  else
    *t_rel += (value->it_interval.tv_usec * 2048) / 1716;

  /* u_now is returned zero first time uclock() is called.  That first
     call could be the one we issued above, or it could be two days
     ago, when the calling program started.  We need to make {rp}_exp
     and u_now be relative to the same point of origin.  */
  *t_exp += u_now;

  start_timer();
  return 0;
}
