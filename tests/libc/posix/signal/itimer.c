#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

volatile int sigtimr = 0;
volatile int sigprof = 0;

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

  tv.it_value.tv_sec   =  5; tv.it_value.tv_usec    = 0;
  tv.it_interval.tv_sec=  0; tv.it_interval.tv_usec = 0;

  /* Set profiler timer to 0.2 sec */
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
      printf("  %d\r",sigprof);
      fflush(stdout);
      oldval = sigprof;
    }
  }

  memset(&tv,0,sizeof(tv));
  setitimer(ITIMER_REAL, &tv, NULL);
  setitimer(ITIMER_PROF, &tv, NULL);

  printf("\n");

  return 0;
}
