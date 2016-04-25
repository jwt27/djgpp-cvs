#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/resource.h>
#include <unistd.h>


int
main (void)
{
  int result = 0;
  int bad_fd = INT_MAX;
  struct rlimit rlim;


  if (getrlimit(RLIMIT_NOFILE, &rlim) == 0
      && 0 <= rlim.rlim_cur && rlim.rlim_cur <= INT_MAX)
    bad_fd = rlim.rlim_cur;

  if (fcntl(0, F_DUPFD, -1) != -1) result |= 1;
  if (errno != EINVAL) result |= 2;

  if (fcntl(0, F_DUPFD, bad_fd) != -1) result |= 4;
  if (errno != EINVAL) result |= 8;

  close (0);
  if (fcntl(0, F_DUPFD, STDERR_FILENO + 1) != -1) result |= 16;
  if (errno != EBADF) result |= 32;

  {
    int fd;
    fd = open(".", O_RDONLY);
    if (fd == -1)
      result |= 64;
    else if (fcntl(fd, F_DUPFD, STDERR_FILENO + 1) == -1)
      result |= 128;

    close (fd);
  }

  if (result)
    printf("fcntl check failed with result = %d\n", result);
  else
    printf("fcntl check passed.\n");

  return result;
}
