#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define TEST_FILE "t-select.tst"

int
main (int argc, char *argv[])
{
  fd_set read_fds;
  fd_set write_fds;
  fd_set except_fds;
  struct timeval tv;
  int fd, ret;

  /* Create the test file, if necessary. */
  if (access(TEST_FILE, D_OK) == 0)
    {
      puts("Directory in way - please remove '" TEST_FILE "'!");
      puts("FAIL");
      return(EXIT_SUCCESS);
    }

  if (access(TEST_FILE, F_OK) != 0)
    {
      fd = open(TEST_FILE, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
      if (fd < 0)
	puts("Unable to create '" TEST_FILE "'!");
    }
  else
    {
      fd = open(TEST_FILE, O_RDWR);
      if (fd < 0)
	puts("Unable to open '" TEST_FILE "'!");
    }

  if (fd < 0)
    {
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  /* Check the file is ready for read & write and that there aren't
   * any errors. */
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_ZERO(&except_fds);

  FD_SET(fd, &read_fds);
  FD_SET(fd, &write_fds);
  FD_SET(fd, &except_fds);

  ret = select(fd + 1, &read_fds, &write_fds, &except_fds, NULL);
  if (ret < 0)
    {
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  if (ret != 2)
    {
      puts("select() did not return the expected number!");
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  if (!FD_ISSET(fd, &read_fds))
    {
      puts("File '" TEST_FILE "' should be ready for reading!");
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  if (!FD_ISSET(fd, &write_fds))
    {
      puts("File '" TEST_FILE "' should be ready for writing!");
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  if (FD_ISSET(fd, &except_fds))
    {
      puts("File '" TEST_FILE "' should not have an error!");
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  /* Try a time-out. */
  FD_ZERO(&except_fds);

  FD_SET(fd, &except_fds);

  tv.tv_sec  = 5;
  tv.tv_usec = 0;
  printf("Trying a time-out: %ld seconds\n", (long) tv.tv_sec);

  ret = select(fd + 1, NULL, NULL, &except_fds, &tv);
  if (ret < 0)
    {
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  if (FD_ISSET(fd, &except_fds))
    {
      puts("File '" TEST_FILE "' should not have an error!");
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  /* Done with the file now. */
  close(fd);
  fd = -1;

  /* Try select on stdin. */
  FD_ZERO(&read_fds);

  FD_SET(fileno(stdin), &read_fds);

  tv.tv_sec  = 5;
  tv.tv_usec = 0;
  printf("Please press a key in the next %ld seconds\n", (long) tv.tv_sec);

  ret = select(fileno(stdin) + 1, &read_fds, NULL, NULL, &tv);
  if (ret <= 0)
    {
      if (ret == 0)
	puts("No key pressed");
      else
	perror(argv[0]);

      puts("FAIL");
      return(EXIT_FAILURE);
    }

  if (!FD_ISSET(fileno(stdin), &read_fds))
    {
      puts("stdin should be ready for reading!");
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  /* Try select on stdout. */
  FD_ZERO(&write_fds);

  FD_SET(fileno(stdout), &write_fds);

  ret = select(fileno(stdout) + 1, NULL, &write_fds, NULL, NULL);
  if (ret != 1)
    {
    }

  if (!FD_ISSET(fileno(stdout), &write_fds))
    {
      puts("stdout should be ready for writing!");
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  /* Try select on stderr. */
  FD_ZERO(&write_fds);

  FD_SET(fileno(stderr), &write_fds);

  ret = select(fileno(stderr) + 1, NULL, &write_fds, NULL, NULL);
  if (ret != 1)
    {
    }

  if (!FD_ISSET(fileno(stderr), &write_fds))
    {
      puts("stderr should be ready for writing!");
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  puts("PASS");
  return(EXIT_SUCCESS);
}
