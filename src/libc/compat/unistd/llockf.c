/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */

/* lockf is a simplified interface to fcntl's locking facilities.  */

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int
llockf (int _fildes, int _cmd, offset_t _len)
{
  struct flock64 lock_req;

  memset ((char *) &lock_req, '\0', sizeof (lock_req));

  /* lockf is always relative to the current file position.  */
  lock_req.l_whence = SEEK_CUR;
  lock_req.l_start = 0L;
  lock_req.l_len = _len;

  switch (_cmd)
    {
    case F_TEST:
      lock_req.l_type = F_WRLCK;
      /* Test the lock: return 0 if FD is unlocked;
       return -1, set errno to EACCES, if the lock cannot be obtained.  */
      if (fcntl (_fildes, F_GETLK64, &lock_req) < 0)
       return -1;
      if (lock_req.l_type == F_UNLCK)
       return 0;
      errno = EACCES;
      return -1;

    case F_ULOCK:
      lock_req.l_type = F_UNLCK;
      _cmd = F_SETLK64;
      break;
    case F_LOCK:
      lock_req.l_type = F_WRLCK;
      _cmd = F_SETLKW64;
      break;
    case F_TLOCK:
      lock_req.l_type = F_WRLCK;
      _cmd = F_SETLK64;
      break;

    default:
      errno = EINVAL;
      return -1;
    }

  return fcntl (_fildes, _cmd, &lock_req);
}
