/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/time.h>
#include <sys/exceptn.h>
#include <sys/mono.h>

/* header of a GPROF type file
*/
typedef struct {
  long low;
  long high;
  long nbytes;
} header;

/* entry of a GPROF type file
*/
typedef struct {
    unsigned long from;
    unsigned long to;
    unsigned long count;
} MTABE;

/* internal form - sizeof(MTAB) is 4096 for efficiency
*/
typedef struct MTAB {
  MTABE calls[341];
  struct MTAB *prev;
} MTAB;

static header h;
static short *histogram;
static int mcount_skip = 1;
static int histlen;
static MTAB *mtab=0;

extern int etext;

/* called by functions.  Use the pointer it provides to cache
** the last used MTABE, so that repeated calls to/from the same
** pair works quickly - no lookup.
*/
void mcount(int _to);
void mcount(int _to)
{
  MTAB *m;
  int i;
  int to;
  int ebp;
  int from;
  int mtabi;
  MTABE **cache;

  if (&_to < &etext)
    *(int *)(-1) = 0; /* fault! */

  mcount_skip = 1;
  asm("movl %%edx,%0" : "=g" (cache)); /* obtain the cached pointer */
  to = *((&_to)-1) - 12;
  ebp = *((&_to)-2); /* glean the caller's return address from the stack */
  from = ((int *)ebp)[1];
  _mono_printf("from %08x  to %08x\r\n", from, to);
  if (*cache && ((*cache)->from == from) && ((*cache)->to == to))
  {
    /* cache paid off - works quickly */
    (*cache)->count++;
    mcount_skip = 0;
    return;
  }

  /* no cache hit - search all mtab tables for a match, or an empty slot */
  mtabi = -1;
  for (m=mtab; m; m=m->prev)
  {
    for (i=0; i<341; i++)
    {
      if (m->calls[i].from == 0)
      {
        /* empty slot - end of table */
        mtabi = i;
        break;
      }
      if ((m->calls[i].from == from) &&
          (m->calls[i].to == to))
        {
          /* found a match - bump count and return */
          m->calls[i].count ++;
          *cache = m->calls + i;
          mcount_skip = 0;
          return;
        }
    }
  }
  if (mtabi != -1)
  {
    /* found an empty - fill it in */
    mtab->calls[mtabi].from = from;
    mtab->calls[mtabi].to = to;
    mtab->calls[mtabi].count = 1;
    *cache = mtab->calls + mtabi;
    mcount_skip = 0;
    return;
  }
  /* lob off another page of memory and initialize the new table */
  m = (MTAB *)sbrk(sizeof(MTAB));
  memset(m, 0, sizeof(MTAB));
  m->prev = mtab;
  mtab = m;
  m->calls[0].from = from;
  m->calls[0].to = to;
  m->calls[0].count = 1;
  *cache = m->calls;
  mcount_skip = 0;
}

/* this is called during program exit (installed by atexit). */
static void
mcount_write(void)
{
  MTAB *m;
  int i, f;
  struct itimerval new_values;

  mcount_skip = 1;

  /* disable timer */
  new_values.it_value.tv_usec = new_values.it_interval.tv_usec = 0;   
  new_values.it_value.tv_sec = new_values.it_interval.tv_sec = 0;
  setitimer(ITIMER_PROF, &new_values, NULL);

  f = open("gmon.out", O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666);
  write(f, &h, sizeof(header));
  write(f, histogram, histlen);
  for (m=mtab; m; m=m->prev)
  {
    for (i=0; i<341; i++)
      if (m->calls[i].from == 0)
        break;
    write(f, m->calls, i*12);
  }
  close(f);
}

extern unsigned start __asm__ ("start");
#define START (unsigned)&start
extern int etext;

/* ARGSUSED */
static void
mcount_tick(int _x)
{
  unsigned bin;
  
  if(!mcount_skip) {
    bin = __djgpp_exception_state->__eip;
    if(bin >= START && bin <= (unsigned)&etext) {
      bin = (bin - START) / 4;	/* 4 EIP's per bin */
      histogram[bin]++;
    }
  }
}

/* this is called to initialize profiling before the program starts */
void _mcount_init(void);
void
_mcount_init(void)
{
  struct itimerval new_values;

  h.low = START;
  h.high = (int)&etext;
  histlen = (h.high-h.low)/4*sizeof(short);
  h.nbytes = sizeof(header) + histlen;
  histogram = (short *)sbrk(histlen);
  memset(histogram, 0, histlen);
  atexit(mcount_write);

  /* here, do whatever it takes to initialize the timer interrupt */
  signal(SIGPROF, mcount_tick);

  /* 18.2 tics per second */
  new_values.it_value.tv_usec = new_values.it_interval.tv_usec = 5494;
  new_values.it_value.tv_sec = new_values.it_interval.tv_sec = 0;

  setitimer(ITIMER_PROF, &new_values, NULL);

  mcount_skip = 0;
}
