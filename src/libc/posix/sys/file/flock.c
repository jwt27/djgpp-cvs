/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */

/* This file implements the `flock' function in terms of the POSIX.1 `fcntl'
   locking mechanism.  */

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>

/* Apply or remove an advisory lock, according to OPERATION,
   on the file FD refers to.  */
int
flock (int _fildes, int _op)
{
  struct flock lock_req;

  switch (_op & ~LOCK_NB)
    {
    case LOCK_SH:
      lock_req.l_type = F_RDLCK;
      break;
    case LOCK_EX:
      lock_req.l_type = F_WRLCK;
      break;
    case LOCK_UN:
      lock_req.l_type = F_UNLCK;
      break;
    default:
      errno = EINVAL;
      return -1;
    }

  lock_req.l_whence = SEEK_SET;
  lock_req.l_start = lock_req.l_len = 0L; /* Lock the whole file.  */

  return fcntl (_fildes, (_op & LOCK_NB) ? F_SETLK : F_SETLKW, &lock_req);
}
