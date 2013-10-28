#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/stat.h>

#define DATA_FILENAME "t-readv.dat"

void
fail (const char *argv0)
{
  perror(argv0);
  puts("FAIL");
  exit(EXIT_FAILURE);
}

int
main (int argc, char *argv[])
{
  char          data[] = "somedata";
  char          bufs[5][4];
  const size_t  n_bufs = 5;
  struct iovec  iov[IOV_MAX];
  char         *p = NULL;
  char          allbuf[128];
  int           fd;
  int           ret;
  int           i;

  assert(n_bufs <= IOV_MAX);

  /* Construct the iovec. */
  iov[0].iov_base = (void *) bufs[0];
  iov[0].iov_len  = 4;

  for (i = 1; i < 5; i++) {
    iov[i].iov_base = (void *) bufs[i];
    iov[i].iov_len  = 1;
  }

  /* Create the data file. */
  fd = open(DATA_FILENAME, O_RDWR|O_TEXT|O_TRUNC|O_CREAT, S_IRUSR|S_IWUSR);

  if (fd < 0)
    fail(argv[0]);

  /* Don't write the nul to the file. */
  ret = write(fd, data, strlen(data));

  close(fd);

  /* Read the data back in. */
  fd = open(DATA_FILENAME, O_RDONLY|O_TEXT);

  if (fd < 0)
    fail(argv[0]);

  ret = readv(fd, iov, n_bufs);
  if (ret < 0)
    fail(argv[0]);

  close(fd);

  /* Reconstruct the string in bufs. */
  for (p = allbuf, i = 0; i < (int)n_bufs; p += iov[i].iov_len, i++) {
    memcpy(p, bufs[i], iov[i].iov_len);
  }
  *p = '\0';

  if (strcmp(allbuf, data) != 0) {
    printf("Expected '%s' - got '%s'\nFAIL\n", data, allbuf);
    return(EXIT_FAILURE);
  }

  puts("PASS");
  return(EXIT_SUCCESS);
}
