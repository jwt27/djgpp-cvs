/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <signal.h>
#include <errno.h>
#include <libc/bss.h>

/* A counter to know when to re-initialize the static sets.  */
static int sigprocmask_count = -1;

/* Which signals are currently blocked (initially none).  */
static sigset_t current_mask;

/* Which signals are pending (initially none).  */
sigset_t __sigprocmask_pending_signals;

/* Previous handlers to restore when the blocked signals are unblocked.  */
typedef void (*sighandler_t)(int);
static sighandler_t prev_handlers[SIGMAX];

/* A signal handler which just records that a signal occurred
   (it will be raised later, if and when the signal is unblocked).  */
static void
sig_suspender (int signo)
{
  sigaddset (&__sigprocmask_pending_signals, signo);
}

int
sigprocmask (int how, const sigset_t *new_set, sigset_t *old_set)
{
  int signo;
  sigset_t new_mask;

  /* If called for the first time, initialize.  */
  if (sigprocmask_count != __bss_count)
    {
      sigprocmask_count = __bss_count;
      sigemptyset (&__sigprocmask_pending_signals);
      sigemptyset (&current_mask);
      for (signo = 0; signo < SIGMAX; signo++)
	prev_handlers[signo] = SIG_ERR;
    }

  if (old_set)
    *old_set = current_mask;

  if (new_set == 0)
    return 0;

  if (how != SIG_BLOCK && how != SIG_UNBLOCK && how != SIG_SETMASK)
    {
      errno = EINVAL;
      return -1;
    }

  sigemptyset (&new_mask);

  for (signo = 0; signo < SIGMAX; signo++)
    {
      if (sigismember (&current_mask, signo))
	sigaddset (&new_mask, signo);
      else if (sigismember (new_set, signo) && how != SIG_UNBLOCK)
	{
	  sigaddset (&new_mask, signo);

	  /* SIGKILL is silently ignored, as on other platforms.  */
	  if (signo != SIGKILL && prev_handlers[signo] == SIG_ERR)
	    prev_handlers[signo] = signal (signo, sig_suspender);
	}
      if ((   how == SIG_UNBLOCK
	      && sigismember (&new_mask, signo)
	      && sigismember (new_set, signo))
	  || (how == SIG_SETMASK
	      && sigismember (&new_mask, signo)
	      && !sigismember (new_set, signo)))
	{
	  sigdelset (&new_mask, signo);
	  if (prev_handlers[signo] != SIG_ERR)
	    {
	      signal (signo, prev_handlers[signo]);
	      prev_handlers[signo] = SIG_ERR;
	    }
	  if (sigismember (&__sigprocmask_pending_signals, signo))
	    {
	      sigdelset (&__sigprocmask_pending_signals, signo);
	      raise (signo);
	    }
	}
    }
  current_mask = new_mask;
  return 0;
}

#ifdef TEST

#include <math.h>
#include <conio.h>

static void
sig_catcher (int signo)
{
  cprintf ("\r\nGot signal %d\r\n", signo);
}

int
main (void)
{
  int *p = 0;

  sigset_t sigmask, prevmask;

  signal (SIGINT, sig_catcher);

  sigemptyset (&sigmask);

  sigaddset (&sigmask, SIGINT);
  if (sigprocmask (SIG_SETMASK, &sigmask, &prevmask) == 0)
    cputs ("SIGINT blocked.  Try to interrupt me now.\r\n");
  while (!kbhit ())
    ;
  cputs ("See?  I wasn't interrupted.\r\n");
  cputs ("But now I will unblock SIGINT, and then get the signal.\r\n");
  sigprocmask (SIG_UNBLOCK, &sigmask, &prevmask);

  sigemptyset (&sigmask);
  sigaddset (&sigmask, SIGFPE);
  signal (SIGFPE, sig_catcher);
  sigprocmask (SIG_SETMASK, &sigmask, &prevmask);
  cputs ("SIGFPE is blocked.  Let's try some FP exception.\r\n");
  cprintf ("asin (2.0) = %f, sqrt(-4.0) = %f\r\n", asin (2.0), sqrt (-4.0));
  cputs ("Did we get the signal?  If not, Let's see if we get it now.\r\n");
  sigprocmask (SIG_UNBLOCK, &sigmask, &prevmask);

  sigemptyset (&sigmask);
  sigaddset (&sigmask, SIGSEGV);
  signal (SIGSEGV, sig_catcher);
  sigprocmask (SIG_SETMASK, &sigmask, &prevmask);
  cputs ("SIGSEGV is blocked.  Let's try a null pointer dereference.\r\n");
  cprintf ("Zero address has this value: %d\r\n", *p);
  cputs ("Did we get Page Fault?  If not, Let's see if we get it now.\r\n");
  sigprocmask (SIG_SETMASK, &prevmask, &sigmask);
  cputs ("Are we still alive?  If so, that's all, folks!\r\n");
  return 0;
}

#endif
