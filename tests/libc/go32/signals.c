#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>
#include <float.h>
#include <pc.h>
#include <dpmi.h>

int tick;
void sig_handler(int);

volatile int result;
void int_handler(int sig)
{
  double urand;

  _clear87();
  if (sig != SIGINT)
    abort();
  puts ("\tSIGINT");
  urand = ((double)rand()) / RAND_MAX;
  if (urand > 0.5)
    result = urand / (rand() < 0);
  else
    result = sqrt (-1. / urand);
}

void quit_handler(int sig)
{
  _clear87();
  if (sig != SIGQUIT)
    abort();
  puts ("\t\tSIGQUIT");
}

jmp_buf jb;
void fpe_handler(int sig)
{
  _clear87();
  if (sig != SIGFPE)
    abort();
  puts ("\t\t\tSIGFPE");
  longjmp(jb, 1);
}

int main(void)
{
  struct itimerval t;
  int c;

  printf ("%s", "\n\
This program tests the stability of exception/signal handling.\n\
It fires SIGALRM and SIGFPE signals at a high rate, and allows you to\n\
add SIGINTs and SIGQUITs for a good measure by pressing and holding\n\
Ctrl-C and Ctrl-\\ respectively.  When compiled with -pg, SIGPROF is\n\
fired every 55ms.\n\
\n\
      To end the program, press `q'.\n\
      To abort the program (and see the registers), press capital `A'\n\
      and then Ctrl-BREAK.\n\
      If SIGFPE stops being printed, try pressing capital `T'.\n\
\n\
   When you press `q', the prgram deliberately generates an FP exception,\n\
   to let you examine the segment registers.\n\
\n\
   Press any key to begin...");
  fflush (stdout);
  while (!kbhit ())
    ;
  getkey ();

  t.it_interval.tv_sec = 0;
  t.it_interval.tv_usec =100;
  t.it_value.tv_sec = 0;
  t.it_value.tv_usec = 100; 
  signal(SIGALRM, sig_handler);
  setitimer(ITIMER_REAL, &t, NULL);
  signal (SIGINT, int_handler);
  signal (SIGQUIT, quit_handler);
  if (!setjmp(jb))
    signal (SIGFPE, fpe_handler);
  _control87(0x033a, 0xffff);	/* unmask zero-divide and invalid-op */
  c = 0;
  while(!kbhit() || (c = getkey()) != 'q')
    {
      printf("Tick %d\n", tick);
      if (c == 'A')
	{
	  signal (SIGINT, SIG_DFL);
	}
      else if (c == 'T')
	{
	  _clear87();
	}
      else if ((tick % 5) == 0 || (tick % 11) == 0 || (tick % 23) == 0)
	{
	  result = (M_PI + tick) / (tick < 0) + sqrt (M_PI - tick);
	}
      c = 0;
    }
  printf ("x87 CW: 0x%x\n", _control87(0, 0));
  printf ("SIGFPE handler: 0x%p (should be 0x%p)\n",
	  signal(SIGFPE, SIG_DFL), fpe_handler);
  printf ("The program should now crash due to FP exception:\n");
  printf ("log of a negative number is %f\n", log(-1.0*tick));
  abort();
  return 0;
}	
void sig_handler(int signum)
{
  _clear87();
  tick++;
}
