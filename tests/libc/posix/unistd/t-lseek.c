#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define TEST_FILE "t-lseek.tst"

int
main (int argc, char *argv[])
{
  int   fd;
  /* off_t o; */
  int   ret;

  fd = open(TEST_FILE, O_RDWR|O_TRUNC|O_CREAT|O_BINARY, S_IRUSR|S_IWUSR);
  if (fd < 0)
    {
      puts("Unable to create test file '" TEST_FILE "'!");
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  ret = write(fd, "a", 1);
  if (ret < 0)
    {
      puts("write() failed!");
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  /* o = */ lseek(fd, 1024 * 1024, SEEK_SET);
  if (ret < 0)
    {
      puts("lseek() failed!");
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  ret = write(fd, "b", 1);
  if (ret < 0)
    {
      puts("write() failed!");
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  /* o = */ lseek(fd, 2 * 1024 * 1024, SEEK_SET);
  if (ret < 0)
    {
      puts("lseek() failed!");
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  ret = write(fd, "c", 1);
  if (ret < 0)
    {
      puts("write() failed!");
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  ret = close(fd);
  if (ret < 0)
    {
      puts("close() failed!");
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  puts("PASS");
  return(EXIT_SUCCESS);
}
