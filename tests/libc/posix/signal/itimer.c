#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

volatile int sigtimr = 0;
volatile int sigprof = -1;

void
_my_handler(int sig)
{
  sigtimr = 1;
}

void
_my_prof_handler(int sig)
{
  ++sigprof;
}

int
main(int argc, char **argv)
{
  struct itimerval tv;
  struct itimerval prof;
  int oldval=-1;
  uclock_t tprev = uclock();

  tv.it_value.tv_sec   =  5; tv.it_value.tv_usec    = 0;
  tv.it_interval.tv_sec=  0; tv.it_interval.tv_usec = 0;

  /* Set profiler timer to 0.2 sec */
  /* If you want to test better, try these values in addition to
     200000:
             50000 56000 100000 110000 250000 800000 970999.

     To test the functions work, make sure the time difference printed
     (as a floating-point value) is close enough to tv_usec you used.  */
  prof.it_value.tv_sec = prof.it_interval.tv_sec = 0;
  prof.it_value.tv_usec= prof.it_interval.tv_usec= 200000;
  signal(SIGALRM, _my_handler);
  signal(SIGPROF, _my_prof_handler);

  setitimer(ITIMER_REAL, &tv,NULL);
  setitimer(ITIMER_PROF, &prof,NULL);

  while (!sigtimr)
  {
    if (sigprof != oldval)
    {
      uclock_t tnow = uclock();

      printf("  %d: %f\n",sigprof,
	     ((double)(tnow - tprev))/UCLOCKS_PER_SEC);
      fflush(stdout);
      oldval = sigprof;
      tprev = tnow;
    }
  }

  memset(&tv,0,sizeof(tv));
  setitimer(ITIMER_REAL, &tv, NULL);
  setitimer(ITIMER_PROF, &tv, NULL);

  printf("\n");

  return 0;
}
