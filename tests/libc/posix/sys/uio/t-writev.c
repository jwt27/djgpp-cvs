#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <libc/unconst.h>

#define DATA_FILENAME "t-writev.dat"

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
  const char   *data[] = { "some", "d", "a", "t", "a" };
  /* NB: data[0] is a pointer. */
  const size_t  n_data = sizeof(data) / sizeof(data[0]);
  struct iovec  iov[IOV_MAX];
  char          alldata[128];
  int           fd;
  int           ret;
  char          buf[128];
  unsigned      i;

  assert(n_data <= IOV_MAX);

  /* Construct the iovec & collapse data into one string. */
  alldata[0] = '\0';
  for (i = 0; i < n_data; i++) {
    iov[i].iov_base = (void *) unconst(data[i], char *);
    iov[i].iov_len  = strlen(data[i]);

    strcat(alldata, data[i]);
  }

  /* Write out the data. */
  fd = open(DATA_FILENAME, O_RDWR|O_TEXT|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
  if (fd < 0)
    fail(argv[0]);

  ret = writev(fd, iov, n_data);
  if (ret < 0)
    fail(argv[0]);

  close(fd);

  /* Read back the data and check it. */
  fd = open(DATA_FILENAME, O_RDONLY|O_TEXT);
  if (fd < 0)
    fail(argv[0]);

  ret = read(fd, buf, sizeof(buf) - 1 /* leave space for nul */);
  if (ret < 0)
    fail(argv[0]);

  close(fd);

  buf[ret] = '\0';

  if (strcmp(buf, alldata) != 0) {
    printf("Expected '%s' - got '%s'\nFAIL\n", alldata, buf);
    return(EXIT_FAILURE);
  }

  puts("PASS");
  return(EXIT_SUCCESS);
}
