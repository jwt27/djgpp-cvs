/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <utime.h>		/* For utime() */
#include <time.h>		/* For localtime() */
#include <fcntl.h>		/* For open() */
#include <unistd.h>
#include <errno.h>		/* For errno */
#include <go32.h>
#include <dpmi.h>

/* An implementation of utime() for DJGPP.  The utime() function
   specifies an access time and a modification time.  DOS has only one
   time, so we will (arbitrarily) use the modification time.

   IF LFN is supported, then both times are used.  */
int
utime(const char *path, const struct utimbuf *times)
{
  __dpmi_regs r;
  struct tm *tm;
  time_t modtime;
  int fildes;
  unsigned int dostime, dosdate;
  int retval = 0, e = 0;

  /* DOS wants the file open */
  fildes = open(path, O_RDONLY);
  if (fildes == -1) return -1;

  /* NULL times means use current time */
  if (times == NULL)
    modtime = time((time_t *) 0);
  else
    modtime = times->modtime;

  /* Convert UNIX time to DOS date and time */
  tm = localtime(&modtime);
  dosdate = tm->tm_mday + ((tm->tm_mon + 1) << 5) +
    ((tm->tm_year - 80) << 9);
  dostime = tm->tm_sec / 2 + (tm->tm_min << 5) +
    (tm->tm_hour << 11);

  /* Set the file timestamp */
  r.h.ah = 0x57; /* DOS FileTimes call */
  r.h.al = 0x01; /* Set date/time request */
  r.x.bx = fildes; /* File handle */
  r.x.cx = dostime; /* New time */
  r.x.dx = dosdate; /* New date */
  __dpmi_int(0x21, &r);
  if (r.x.flags & 1)
  {
    e = EIO;
    retval = -1;
  }
  else if (_USE_LFN)
  {
    /* We can set access time as well.  */
    if (times)
      modtime = times->actime;
    tm = localtime(&modtime);
    dosdate = tm->tm_mday + ((tm->tm_mon + 1) << 5) +
      ((tm->tm_year - 80) << 9);
    dostime = tm->tm_sec / 2 + (tm->tm_min << 5) +
      (tm->tm_hour << 11);

    r.x.ax = 0x5705;
    r.x.bx = fildes;
    r.x.cx = dostime;	/* this might be ignored */
    r.x.dx = dosdate;
    __dpmi_int(0x21, &r);
    if (r.x.flags & 1)
    {
      e = EIO;
      retval = -1;
    }
  }

  /* Close the file */
  (void) close(fildes);
  if (e)
    errno = e;

  return retval;
}
