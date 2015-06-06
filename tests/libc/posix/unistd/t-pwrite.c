#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <io.h>
#include <libc/fd_props.h>

#define TEST_FILE     "t-pwrite.tst"
#define EXTEND_LENGTH (1024 * 1024)

int
main (int argc, char *argv[])
{
  int          fd;
  size_t       ret;
  const char   orig_buf[]   = "abc";
  const size_t orig_bufsize = strlen(orig_buf);
  const char   repl_char[]  = "d"; /* Replacement character */
  const size_t repl_pos     = 1;   /* Replacement position */
  char         after_buf[]  = "XXX";
  char         buf[128];
  const size_t bufsize      = sizeof(buf);
  const int    fd_stdout    = fileno(stdout);
  const int    fd_stderr    = fileno(stderr);
  const char   msg_stdout[] = "Just a test (stdout)\n";
  const char   msg_stderr[] = "Just a test (stderr)\n";

  /* Test overwriting a particular position. */
  strcpy(after_buf, orig_buf);
  after_buf[repl_pos] = repl_char[0];

  fd = open(TEST_FILE, O_RDWR|O_TRUNC|O_CREAT|O_BINARY, S_IRUSR|S_IWUSR);
  if (fd < 0)
    {
      puts("Unable to create test file '" TEST_FILE "'!");
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  ret = write(fd, orig_buf, orig_bufsize);
  if (ret < 0)
    {
      puts("write() failed!");
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  ret = pwrite(fd, repl_char, 1, repl_pos);
  if (ret < 0)
    {
      puts("pwrite() to overwrite failed!");
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  /* Check that the file now contains after_buf. */
  ret = lseek(fd, 0, SEEK_SET);
  if (ret < 0)
    {
      puts("lseek() failed!");
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  ret = read(fd, buf, bufsize);
  if (ret < 0)
    {
      puts("read() failed!");
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  buf[sizeof(buf) - 1] = '\0';

  if (strncmp(after_buf, buf, strlen(after_buf)) != 0)
    {
      puts("Data comparison failed!");
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  /* Try extending the file's size. */
  ret = pwrite(fd, orig_buf, orig_bufsize, EXTEND_LENGTH);
  if (ret < 0)
    {
      puts("pwrite() to extend failed!");
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  /* Check the file size. */
  ret = filelength(fd);
  if (ret < 0)
    {
      puts("filelength() failed!");
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  if (ret != (EXTEND_LENGTH + orig_bufsize))
    {
      puts("Failed to extend file to correct length!");
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

  /* Try writing to stdout, stderr. */
  ret = pwrite(fd_stdout, msg_stdout, strlen(msg_stdout), 1024);
  if (ret < 0)
    {
      puts("pwrite() to stdout failed!");
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  ret = pwrite(fd_stderr, msg_stderr, strlen(msg_stderr), 1024);
  if (ret < 0)
    {
      puts("pwrite() to stderr failed!");
      perror(argv[0]);
      puts("FAIL");
      return(EXIT_FAILURE);
    }

  puts("PASS");
  return(EXIT_SUCCESS);
}
