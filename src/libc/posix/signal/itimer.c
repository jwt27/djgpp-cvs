/* Copyright (C) 1995 Charles Sandmann (sandmann@clio.rice.edu)
   setitimer implmentation - used for profiling and alarm
   BUGS: ONLY ONE AT A TIME, first pass code
   This software may be freely distributed, no warranty. */

#include <libc/stubs.h>
#include <sys/time.h>
#include <errno.h>
#include <dpmi.h>
#include <signal.h>

static struct itimerval real, prof;

/* not right, should compute from current tic count.  Do later */
int getitimer(int which, struct itimerval *value)
{
  if(which == ITIMER_REAL) {
    *value = real;
    return 0;
  } else if(which == ITIMER_PROF) {
    *value = prof;
    return 0;
  }
  errno = EINVAL;
  return -1;
}

extern unsigned __djgpp_timer_countdown;
extern __dpmi_paddr __djgpp_old_timer;
extern int __djgpp_timer_hdlr;
static char timer_on = 0;
static int sigtype = SIGALRM;
static int reload = 0;

static void stop_timer(void)
{
  if(!timer_on)
    return;
  __djgpp_timer_countdown = -1;
  __dpmi_set_protected_mode_interrupt_vector(8, &__djgpp_old_timer);
  timer_on = 0;
  signal(SIGTIMR, SIG_DFL);
}

static void timer_action(int signum)
{
  if(reload)
    __djgpp_timer_countdown = reload;
  else
    stop_timer();
  raise(sigtype);
}

static void start_timer(void)
{
  __dpmi_paddr int8;

  if(timer_on)
    return;
  timer_on = 1;
  signal(SIGTIMR, timer_action);
  __dpmi_get_protected_mode_interrupt_vector(8, &__djgpp_old_timer);
  asm("movw %%cs,%0" : "=g" (int8.selector) );
  int8.offset32 = (unsigned) &__djgpp_timer_hdlr;
  __dpmi_set_protected_mode_interrupt_vector(8, &int8);
}

/* Note, this should have a scheduler to handle both, do later.  Currently
   can't have both at same time */

int setitimer(int which, struct itimerval *value, struct itimerval *ovalue)
{
  if(ovalue)
    if(getitimer(which,ovalue))
      return -1;	/* errno already set */

  if((value->it_value.tv_sec | value->it_value.tv_usec) == 0) {
    stop_timer();
    return 0;
  }
  
  if(which == ITIMER_REAL) {
    sigtype = SIGALRM;
  } else if(which == ITIMER_PROF) {
    sigtype = SIGPROF;
  } else {
    errno = EINVAL;
    return -1;
  }
  
  __djgpp_timer_countdown = value->it_value.tv_sec * 18;
  __djgpp_timer_countdown += value->it_value.tv_sec / 5;
  __djgpp_timer_countdown += (value->it_value.tv_usec + 54944) / 54955;
  
  reload = value->it_interval.tv_sec * 18;
  reload += value->it_interval.tv_sec / 5;
  reload += (value->it_interval.tv_usec + 54944) / 54955;
  
  start_timer();
  return 0;
}
