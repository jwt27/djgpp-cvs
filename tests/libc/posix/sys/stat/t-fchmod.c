#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dos.h>

const char TEST_FILE[] = "t-fchmod.tst";

typedef struct {
  FILE         *fp;
  const char   *name;
  const mode_t  mode;
} std_test_t;

/* Try setting the handles to the mode they should have anyway. */
std_test_t passing_std_tests[] = {
  { stdin,  "stdin",  S_IRUSR },
  { stdout, "stdout", S_IWUSR },
  { stderr, "stderr", S_IWUSR },
  { stdaux, "stdaux", S_IRUSR|S_IWUSR },
  { stdprn, "stdprn", S_IWUSR },
  { NULL, NULL, 0 }
};

/* Try setting the handles to the mode they shouldn't have. */
std_test_t failing_std_tests[] = {
  { stdin,  "stdin",  S_IWUSR },
  { stdout, "stdout", S_IRUSR },
  { stderr, "stderr", S_IRUSR },
#if 0
  { stdaux, "stdaux", 0 },
  { stdprn, "stdprn", S_IRUSR },
#endif
  { NULL, NULL, 0 }
};

static void
die (const char *prog_name)
{
  perror(prog_name);
  puts("FAIL");
  exit(EXIT_FAILURE);
}

static int
check_read_only (const char *file)
{
  if (access(file, R_OK))
    {
      printf("%s should be readable!\n", file);
      return(0);
    }

  if (!access(file, W_OK))
    {
      printf("%s should not be writeable!\n", file);
      return(0);
    }

  return(1);
}

int
main (int argc, char *argv[])
{
  int ok = 1;
  int fd;
  int fd_array[2];
  int i;

  /* - Standard handles - */

  for (i = 0; passing_std_tests[i].fp != NULL; i++)
    {
      if (fchmod(fileno(passing_std_tests[i].fp),
		 passing_std_tests[i].mode) < 0)
	{
	  perror(passing_std_tests[i].name);
	  ok = 0;
	}
  }

  for (i = 0; failing_std_tests[i].fp != NULL; i++)
    {
      int std_fd = fileno(failing_std_tests[i].fp);

      if (   (fchmod(std_fd, failing_std_tests[i].mode) == 0)
	  && !isatty(std_fd))
	{
	  printf("%s: Should not be able to set mode to 0%o\n",
		 failing_std_tests[i].name, failing_std_tests[i].mode);
	  ok = 0;
	}
    }

  /* - Conventional file - */

  /* Remove the test file, if it exists. */
  if (!access(TEST_FILE, F_OK) && unlink(TEST_FILE))
    die(argv[0]);

  /* Create the file with read-write permissions. */
  fd = open(TEST_FILE, O_BINARY|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
  if (fd < 0)
    die(argv[0]);
  if (close(fd) < 0)
    die(argv[0]);

  /* Check the permissions are read-write. */
  if (access(TEST_FILE, R_OK) || access(TEST_FILE, W_OK))
    {
      printf("%s should be readable and writeable!\nFAIL\n", TEST_FILE);
      return(EXIT_FAILURE);
    }

  /* Re-open the file and change the permissions to read-only.
   * Try writing to the file before and after. */
  fd = open(TEST_FILE, O_BINARY|O_RDWR);
  if (fd < 0)
    die(argv[0]);

  /* Try writing to the file. */
  {
    char wibble[] = "wibble-read-write";

    if (write(fd, wibble, strlen(wibble)) < 0)
      {
	printf("Unable to write to read-write file %s\n", TEST_FILE);
	perror(argv[0]);
	ok = 0;
      }
  }

  if (fchmod(fd, S_IRUSR) < 0)
    die(argv[0]);

  /* Check the permissions are read-only. */
  if (!check_read_only(TEST_FILE))
    ok = 0;

  /* Try writing to the file. */
  {
    char wibble[] = "wibble-read-only";

    if (write(fd, wibble, strlen(wibble)) < 0)
      {
	printf("Unable to write to read-only file %s\n", TEST_FILE);
	perror(argv[0]);
	ok = 0;
      }
  }

  if (close(fd) < 0)
    die(argv[0]);

  /* Check the permissions are read-only. */
  if (!check_read_only(TEST_FILE))
    ok = 0;

  /* - Bypass normal open calls to get a file handle. */
  if (_dos_open(TEST_FILE, O_RDONLY, &fd) != 0)
    die(argv[0]);

  /* Test that fchmod just returns, when the file permissions are
   * the same as the current and there is no filename in fd_props. */
  if (fchmod(fd, S_IRUSR) < 0)
    die(argv[0]);

  /* Check that it fails, when we try different file permissions. */
  if (fchmod(fd, S_IRUSR|S_IWUSR) >= 0)
    {
      printf("fchmod call should have failed: %s:%d!\n", __FILE__, __LINE__);
      ok = 0;
    }

  if (_dos_close(fd) != 0)
    die(argv[0]);

  /* - Pipe - */
  if (pipe(fd_array) < 0) {
    puts("Unable to create pipe");
    perror(argv[0]);
    ok = 0;
  } else {
    if (fchmod(fd_array[0], S_IRUSR|S_IWUSR) >= 0)
      {
	printf("fchmod call should have failed: %s:%d!\n", __FILE__, __LINE__);
	ok = 0;
      }
  }

  /* - Done - */

  if (!ok) {
    puts("FAIL");
    return(EXIT_FAILURE);
  }

  puts("PASS");
  return(EXIT_SUCCESS);
}
