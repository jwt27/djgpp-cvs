/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <errno.h>
#include <unistd.h>
#include <io.h>
#include <libc/getdinfo.h>

ssize_t
pwrite (int fd, const void *buf, size_t nbyte, off_t offset)
{
  short dev_info;
  int needs_seek = 1; /* Seek by default */
  offset_t start_pos, curr_pos;
  int old_errno;
  ssize_t ret;

  /* Is this a character device? If so, don't do seeks.
   *
   * Character devices do not support seeks. Seeking on stdout, stderr
   * does not fail, but it may for other character devices. So we avoid
   * seeking for character devices. */
  dev_info = _get_dev_info(fd);
  if (dev_info == -1)
    return(-1); /* Pass through errno. */

  if (dev_info & _DEV_CDEV)
    needs_seek = 0;

  /* TODO: When we support the Large File Summit, fail here, if the offset
   * is negative. */
#if 0
  if (offset < 0) {
    errno = EINVAL;
    return(-1);
  }
#endif

  /* Get the current position. */
  start_pos = needs_seek ? llseek(fd, 0LL, SEEK_CUR) : 0LL;
  if (start_pos == -1)
    return(-1); /* Pass through errno. */

  /* Seek to the new position. */
  curr_pos = needs_seek ? llseek(fd, offset, SEEK_SET) : 0LL;
  if (curr_pos == -1) {
    old_errno = errno;

    /* Failed. Try to seek back to the original position. */
    llseek(fd, start_pos, SEEK_SET);

    errno = old_errno;
    return(-1);
  }

  /* Write the data. */
  ret = write(fd, buf, nbyte);
  if (ret < 0) {
    old_errno = errno;

    /* Failed. Try to seek back to the original position. */
    llseek(fd, start_pos, SEEK_SET);

    errno = old_errno;
    return(-1);
  }

  /* Seek back to the start. */
  curr_pos = needs_seek ? llseek(fd, start_pos, SEEK_SET) : 0LL;
  if (curr_pos == -1)
    return(-1); /* Pass through errno. */

  return(ret);
}
