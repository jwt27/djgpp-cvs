/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <errno.h>

/* The references disagree on the values of these.  Higher value
   means less important process.  */
#define NICE_MIN -20
#define NICE_MAX +20
#define NICE_USER 0

/* Currently not used, but __dpmi_int might later be set
   to call __dpmi_yield() also when (nice_val > NICE_USER)
   except, of course, for the __dpmi_yield() call itself.  */
static int nice_val = NICE_USER;

int
nice (int incr)
{
  if (incr < 0 && getuid () != 0) {
    errno = EPERM;
    return -1;
  }

  nice_val += incr;
  if (nice_val < NICE_MIN) nice_val = NICE_MIN;
  if (nice_val > NICE_MAX) nice_val = NICE_MAX;

  /* This is braindead by design!  If -1 returned you don't know
     if you had an error!  Luckily you can ignore errors for a
     function like this.  */
  errno = 0;
  return (nice_val - NICE_USER);
}

