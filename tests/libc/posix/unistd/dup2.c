#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


int main (void)
{
  int result = 0;
  int bad_fd = INT_MAX;
  struct rlimit rlim;


  if (getrlimit(RLIMIT_NOFILE, &rlim) == 0
      && 0 <= rlim.rlim_cur && rlim.rlim_cur <= INT_MAX)
    bad_fd = rlim.rlim_cur;

  if (dup2(1, 1) != 1)
    result |= 1;

  close(0);
  if (dup2(0, 0) != -1)
    result |= 2;

  if (dup2(2, bad_fd) == -1 && errno != EBADF)
    result |= 4;

  if (dup2(2, -1) != -1 && errno != EBADF)
    result |= 8;

  if (dup2(2, bad_fd - 1) != bad_fd -1)
    result |= 16;
  if (dup2(2, bad_fd) != -1)
    result |= 32;

  {
    int fd = open(".", O_RDONLY);
    if (fd == -1)
      result |= 64;
    else if (dup2(fd, fd + 1) == -1)
      result |= 128;

    close(fd);
  }

  if (result)
    printf("dup2 check failed with result = %d\n", result);
  else
    printf("dup2 check passed.\n");

  return result;
}
