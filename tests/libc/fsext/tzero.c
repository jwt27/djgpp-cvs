/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */

/* Test written by Richard Dawe <richdawe@bigfoot.com> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <assert.h>

#include <sys/xdevices.h>

static const char DEV_ZERO_PATH[] = "/dev/zero";
static const char DEV_FULL_PATH[] = "/dev/full";

static int
jumble_buffer (char *buf, size_t buflen)
{
  size_t i;

  for (i = 0; i < buflen; i++) {
    buf[i] = random() % 0xff;
  }

  return(1);
}

static int
dump_stat (struct stat *sbuf)
{
  printf("st_atime   = %d\n"
	 "st_ctime   = %d\n"
	 "st_dev     = %d\n"
	 "st_gid     = %d\n"
	 "st_ino     = %d\n"
	 "st_mode    = %d\n"
	 "st_mtime   = %d\n"
	 "st_nlink   = %d\n"
	 "st_size    = %d\n"
	 "st_blksize = %d\n"
	 "st_uid     = %d\n",
	 sbuf->st_atime, sbuf->st_ctime, sbuf->st_dev,
	 sbuf->st_gid, sbuf->st_ino, sbuf->st_mode,
	 sbuf->st_mtime, sbuf->st_nlink, sbuf->st_size,
	 sbuf->st_blksize, sbuf->st_uid);

  return(1);
}

static int
test_fstat (const char *filename)
{
  struct stat sbuf;
  char        buf[32768];
  int         fd = 0;
  int         n  = 0;

  fd = open(filename, O_RDWR);
  if (fd == -1) {
    fprintf(stderr,
	    "Unable to open %s: %s\n", filename, strerror(errno));
    return(EXIT_FAILURE);
  }

  sleep(1);

  n = read(fd, buf, sizeof(buf));
  if (n < 0) {
    fprintf(stderr,
	    "Unable to read %lu bytes from %s: %s\n",
	    sizeof(buf), filename, strerror(errno));
  }

  sleep(1);

  n = write(fd, buf, sizeof(buf));
  if (n < 0) {
    fprintf(stderr,
	    "Unable to write %lu bytes to %s: %s\n",
	    sizeof(buf), filename, strerror(errno));
  }

  if (fstat(fd, &sbuf) < 0) {
    fprintf(stderr,
	    "Unable to fstat() %s: %s\n", filename, strerror(errno));
    close(fd);
    return(0);
  }

  printf("fstat() result for %s:\n", filename);
  dump_stat(&sbuf);
  printf("\n");

  close(fd);

  return(1);
}

int
main (int argc, char *argv[])
{
  char           buf[32768];
  char           filename[PATH_MAX];
  int            fd           = 0;
  int            new_fd       = 0;
  fd_set         readfds, writefds;
  struct timeval tv;
  struct stat    sbuf;
  off_t          offset       = 0;
  offset_t       lloffset     = 0;
  off_t          ret_offset   = 0;
  offset_t       ret_lloffset = 0;
  uid_t          owner        = 0;
  gid_t          group        = 0;
  int            ret          = 0;
  int            n            = 0;
  size_t         i            = 0;

  if (!__install_dev_zero()) {
    fprintf(stderr, "__install_dev_zero() failed\n");
    return(EXIT_FAILURE);
  }

  if (!__install_dev_full()) {
    fprintf(stderr, "__install_dev_full() failed\n");
    return(EXIT_FAILURE);
  }

  /* - Test open() - */

  /* Normal path */
  fd = open(DEV_ZERO_PATH, O_RDWR);
  if (fd == -1) {
    fprintf(stderr,
	    "Unable to open %s: %s\n", DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  close(fd);

  fd = open(DEV_FULL_PATH, O_RDWR);
  if (fd == -1) {
    fprintf(stderr,
	    "Unable to open %s: %s\n", DEV_FULL_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  close(fd);

  /* Drive-extended path */
  sprintf(filename, "c:%s", DEV_ZERO_PATH);

  fd = open(filename, O_RDWR);
  if (fd == -1) {
    fprintf(stderr,
	    "Unable to open %s: %s\n", filename, strerror(errno));
    return(EXIT_FAILURE);
  }

  close(fd);

  /* Upper case */
  strcpy(filename, DEV_ZERO_PATH);
  strupr(filename);

  fd = open(filename, O_RDWR);
  if (fd == -1) {
    fprintf(stderr,
	    "Unable to open %s: %s\n", filename, strerror(errno));
    return(EXIT_FAILURE);
  }

  close(fd);

  /* - Generic tests of /dev/zero. - */
  fd = open(DEV_ZERO_PATH, O_RDWR);
  if (fd == -1) {
    fprintf(stderr,
	    "Unable to open %s: %s\n", DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  /* Write buffer into /dev/zero. */
  jumble_buffer(buf, sizeof(buf));

  n = write(fd, buf, sizeof(buf));
  if (n < 0) {
    fprintf(stderr,
	    "Unable to write %lu bytes to %s: %s\n",
	    sizeof(buf), DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  assert(((size_t) n) == sizeof(buf));

  /* Zero buffer by reading from /dev/zero. */
  n = read(fd, buf, sizeof(buf));
  if (n < 0) {
    fprintf(stderr,
	    "Unable to read %lu bytes from %s: %s\n",
	    sizeof(buf), DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  assert(((size_t) n) == sizeof(buf));

  for (i = 0; i < sizeof(buf); i++) {
    if (buf[i] != '\0') {
      fprintf(stderr, "Byte %lu in read data is non-zero\n", i);
      return(EXIT_FAILURE);
    }
  }

  close(fd);

  /* - Generic tests for /dev/full - */

  /* Writing to /dev/full is tested by test_fstat() later on. */

  /* - Test /dev/zero opened read-only. - */
  fd = open(DEV_ZERO_PATH, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr,
	    "Unable to open %s read-only: %s\n",
	    DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  /* Check that writing fails. */
  jumble_buffer(buf, sizeof(buf));

  n = write(fd, buf, sizeof(buf));
  if (n >= 0) {
    fprintf(stderr,
	    "Able to write %lu bytes to %s when read-only: %s\n",
	    sizeof(buf), DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  /* Zero buffer by reading from /dev/zero. */
  n = read(fd, buf, sizeof(buf));

  if (n < 0) {
    fprintf(stderr,
	    "Unable to read %lu bytes from %s when read-only: %s\n",
	    sizeof(buf), DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  assert(((size_t) n) == sizeof(buf));

  close(fd);

  /* - Test /dev/zero opened write-only. - */
  fd = open(DEV_ZERO_PATH, O_WRONLY);
  if (fd == -1) {
    fprintf(stderr,
	    "Unable to open %s write-only: %s\n",
	    DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  /* Write buffer into /dev/zero. */
  jumble_buffer(buf, sizeof(buf));

  n = write(fd, buf, sizeof(buf));
  if (n < 0) {
    fprintf(stderr,
	    "Unable to write %lu bytes to %s when write-only: %s\n",
	    sizeof(buf), DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  assert(((size_t) n) == sizeof(buf));

  /* Check that reading fails. */  
  n = read(fd, buf, sizeof(buf));
  if (n >= 0) {
    fprintf(stderr,
	    "Able to read %lu bytes from %s when write-only: %s\n",
	    sizeof(buf), DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  close(fd);

  /* - Check that creat() fails. - */
  fd = creat(DEV_ZERO_PATH, S_IRUSR|S_IWUSR);
  if (fd >= 0) {
    fprintf(stderr,
	    "creat() succeeded in creating %s - it should fail\n",
	    DEV_ZERO_PATH);
    return(EXIT_FAILURE);
  }

  assert(errno == EEXIST);

  /* - Check that open() fails, when using O_CREAT. - */
  fd = open(DEV_ZERO_PATH, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
  if (fd >= 0) {
    fprintf(stderr,
	    "open() succeeded in creating %s - it should fail\n",
	    DEV_ZERO_PATH);
    return(EXIT_FAILURE);
  }

  assert(errno == EEXIST);

  /* - Check select() support. - */
  fd = open(DEV_ZERO_PATH, O_RDWR);
  if (fd == -1) {
    fprintf(stderr,
	    "Unable to open %s: %s\n", DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  memset(&tv, 0, sizeof(tv));

  FD_SET(fd, &readfds);
  FD_SET(fd, &writefds);

  n = select(fd + 1, &readfds, &writefds, NULL, &tv);
  if (n < 0) {
    fprintf(stderr,
	    "select() on %sfailed: %s\n", DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  if (!FD_ISSET(fd, &readfds)) {
    fprintf(stderr, "Expected %s to be ready for reading\n", DEV_ZERO_PATH);
    return(EXIT_FAILURE);
  }

  if (!FD_ISSET(fd, &writefds)) {
    fprintf(stderr, "Expected %s to be ready for writing\n", DEV_ZERO_PATH);
    return(EXIT_FAILURE);
  }

  close(fd);

  /* - Check link() fails - */
#define LINK_TEST_PATH "/temp/wibble"

  n = link(DEV_ZERO_PATH, LINK_TEST_PATH);
  if (n == 0) {
    fprintf(stderr,
	    "link(\"%s\", \"%s\") succeeded - it should fail\n",
	    DEV_ZERO_PATH, LINK_TEST_PATH);
    return(EXIT_FAILURE);
  }

  assert(errno == EPERM);

#undef LINK_TEST_PATH

  /* - Check unlink() fails - */
  n = unlink(DEV_ZERO_PATH);
  if (n >= 0) {
    fprintf(stderr,
	    "unlink() succeeded in removing %s - it should fail\n",
	    DEV_ZERO_PATH);
    return(EXIT_FAILURE);
  }

  assert(errno == EPERM);

  /* - Check lseek() - */
  fd = open(DEV_ZERO_PATH, O_RDWR);
  if (fd == -1) {
    fprintf(stderr,
	    "Unable to open %s: %s\n", DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  for (i = 0; i < 1000; i++) {
    offset     = (off_t) random();
    ret_offset = lseek(fd, offset, SEEK_SET);

    if (ret_offset < 0)
      fprintf(stderr, "lseek() to position %d failed\n", offset);
  }

  close(fd);

  /* - Check llseek() - */
  fd = open(DEV_ZERO_PATH, O_RDWR);
  if (fd == -1) {
    fprintf(stderr,
	    "Unable to open %s: %s\n", DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  for (i = 0; i < 1000; i++) {
    lloffset     = (offset_t) random();
    ret_lloffset = llseek(fd, offset, SEEK_SET);

    if (ret_lloffset < 0)
      fprintf(stderr, "llseek() to position %Lu failed\n", lloffset);
  }

  close(fd);

  /* - Check fchown() - */

  /* fchown() should behave the same way for /dev/zero as it does for
   * regular files - it should always succeed. */
  fd = open(DEV_ZERO_PATH, O_RDWR);
  if (fd == -1) {
    fprintf(stderr,
	    "Unable to open %s: %s\n", DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  /* Try the current uid, gid. */
  owner = getuid();
  group = getgid();

  ret = fchown(fd, owner, group);
  if (ret < 0) {
    fprintf(stderr,
	    "fchown() on %s failed unexpectedly, when changing ownership "
	    "to current owner: %s\n", DEV_ZERO_PATH, strerror(errno));
    close(fd);
    return(EXIT_FAILURE);
  }

  /* Try a non-existent uid, gid. */
  owner *= 2, group *= 2;

  ret = fchown(fd, owner, group);
  if (ret < 0) {
    fprintf(stderr,
	    "fchown() on %s failed unexpectedly, when changing ownership: "
	    "%s\n", DEV_ZERO_PATH, strerror(errno));
    close(fd);
    return(EXIT_FAILURE);
  }

  close(fd);

  /* - Check dup works - */
#ifdef DJGPP_SUPPORTS_FSEXT_DUP_NOW
  fd = open(DEV_ZERO_PATH, O_RDWR);
  if (fd == -1) {
    fprintf(stderr,
	    "Unable to open %s: %s\n", DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  new_fd = dup(fd);
  if (new_fd < 0) {
    fprintf(stderr,
	    "Unable do dup file descriptor for %s: %s\n",
	    DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  close(fd);

  /* Zero buffer by reading from /dev/zero. */
  n = read(new_fd, buf, sizeof(buf));
  if (n < 0) {
    fprintf(stderr,
	    "Unable to read %lu bytes from %s: %s\n",
	    sizeof(buf), DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  assert(((size_t) n) == sizeof(buf));

  close(new_fd);
#endif /* DJGPP_SUPPORTS_FSEXT_DUP_NOW */

  /* - Check fcntl() - */
  fd = open(DEV_ZERO_PATH, O_RDWR);
  if (fd == -1) {
    fprintf(stderr,
	    "Unable to open %s: %s\n", DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  new_fd = fcntl(fd, F_DUPFD);
  if (new_fd < 0) {
    fprintf(stderr, "F_DUPFD fcntl failed: %s\n", strerror(errno));
    return(EXIT_FAILURE);
  }

  /* Zero buffer by reading from /dev/zero. */
  n = read(new_fd, buf, sizeof(buf));
  if (n < 0) {
    fprintf(stderr,
	    "Unable to read %lu bytes from %s: %s\n",
	    sizeof(buf), DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  assert(((size_t) n) == sizeof(buf));

  /* Close old fd and check we can still read using new_fd. */
  close(fd);

  n = read(new_fd, buf, sizeof(buf));
  if (n < 0) {
    fprintf(stderr,
	    "Unable to read %lu bytes from %s: %s\n",
	    sizeof(buf), DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  assert(((size_t) n) == sizeof(buf));

  close(new_fd);

  /* Now try other fcntls */
  fd = open(DEV_ZERO_PATH, O_RDWR);
  if (fd == -1) {
    fprintf(stderr,
	    "Unable to open %s: %s\n", DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  n = fcntl(fd, F_GETFD);
  if (n < 0) {
    fprintf(stderr, "F_GETFD fcntl failed: %s\n", strerror(errno));
    return(EXIT_FAILURE);
  }
  printf("close-on-exec is %s\n", n ? "enabled" : "disabled");

  n = fcntl(fd, F_SETFD, 1);
  if (n != -1) {
    fprintf(stderr, "F_SETFD succeeded - it should fail\n");
    return(EXIT_FAILURE);
  }

  assert(errno == ENOSYS);

  n = fcntl(fd, F_GETFL);
  printf("F_GETFL returns: 0x%x\n", n);

  if (n & O_NONBLOCK) {
    n = fcntl(fd, F_SETFL, n & ~O_NONBLOCK);
  } else {
    n = fcntl(fd, F_SETFL, n | O_NONBLOCK);
  }
 
  if (n < 0) {
    fprintf(stderr, "Failed to flip O_NONBLOCK using F_SETFL fcntl: %s\n",
	    strerror(errno));
    return(EXIT_FAILURE);
  }

  /* Try a bogus fcntl */
  n = fcntl(fd, 0xff);
  if (n != -1) {
    fprintf(stderr, "Bogus fcntl succeeded - it should fail\n");
    return(EXIT_FAILURE);
  }

  assert(errno == ENOSYS);

  close(fd);

  /* - Check stat() works - */
  if (stat(DEV_ZERO_PATH, &sbuf) < 0) {
    fprintf(stderr,
	    "Unable to stat() %s: %s\n", DEV_ZERO_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  printf("stat() result for %s:\n", DEV_ZERO_PATH);
  dump_stat(&sbuf);
  printf("\n");

  if (stat(DEV_FULL_PATH, &sbuf) < 0) {
    fprintf(stderr,
	    "Unable to stat() %s: %s\n", DEV_FULL_PATH, strerror(errno));
    return(EXIT_FAILURE);
  }

  printf("stat() result for %s:\n", DEV_FULL_PATH);
  dump_stat(&sbuf);
  printf("\n");

  /* - Check fstat() works - */
  test_fstat(DEV_ZERO_PATH);
  test_fstat(DEV_FULL_PATH);

  /* Success!*/
  printf("SUCCESS\n");

  return(EXIT_SUCCESS);
}
