#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <io.h>
#include <sys/ioctl.h>
#include <time.h>
#include <limits.h>
#include <libc/fd_props.h>

#define TEST_DIR "t-fchdir.dir"
#define TEST_FILE "t-fchdir.fil"

static inline void
fail (void)
{
  puts("FAIL");
  exit(EXIT_FAILURE);
}

int
main (int argc, char *argv[])
{
  off_t offset;
  char buf[1024], buf2[1024];
  char cwd[PATH_MAX];
  fd_set read_fds, write_fds, except_fds;
  short dev_info;
  struct stat stat_buf, fstat_buf;
  int fd, ret;
  FILE *fp;
  fpos_t fpos;
  long l;
  char *p;

  if (   (access(TEST_DIR, D_OK) != 0)
      && (mkdir(TEST_DIR, S_IRUSR|S_IWUSR) < 0))
    {
      perror(argv[0]);
      fail();
    }

  /* --- Regular file testing --- */

  /* Check we haven't broken I/O for normal files. */

  /* - POSIX I/O functions - */

  if (!access(TEST_FILE, F_OK) && (unlink(TEST_FILE) < 0))
    {
      puts("'" TEST_FILE "' is in the way - please remove it.");
      fail();
    }

  fd = open(TEST_FILE, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
  if (fd < 0)
    {
      puts("Unable to open() '" TEST_FILE "'!");
      fail();
    }

  assert(sizeof(buf) == sizeof(buf2));
  memset(buf, 'X', sizeof(buf));
  memset(buf2, ' ', sizeof(buf2));

  ret = write(fd, buf, sizeof(buf));
  if (ret < 0)
    {
      puts("Unable to write() '" TEST_FILE "'!");
      fail();
    }

  ret = lseek(fd, 0, SEEK_SET);
  if (ret < 0)
    {
      puts("Unable to lseek() '" TEST_FILE "'!");
      fail();
    }

  ret = read(fd, buf2, sizeof(buf2));
  if (ret < 0)
    {
      puts("Unable to read() '" TEST_FILE "'!");
      fail();
    }

  if (memcmp(buf, buf2, sizeof(buf)) != 0)
    {
      puts("Differences found in comparison of data read() "
	   "after write() on '" TEST_FILE "'!");
      fail();
    }

  ret = close(fd);
  if (ret < 0)
    {
      puts("Unable to close '" TEST_FILE "'!");
      perror(argv[0]);
      fail();
    }

  /* - C stream I/O functions - */

  fp = fopen(TEST_FILE, "r+b");
  if (fp == NULL)
    {
      puts("Unable to fopen() '" TEST_FILE "' for read-write & append!");
      fail();
    }

  memset(buf2, ' ', sizeof(buf2));

  if (fread(buf2, sizeof(buf2), 1, fp) != 1)
    {
      puts("Unable to fread() '" TEST_FILE "'!");
      fail();
    }

  if (memcmp(buf, buf2, sizeof(buf)) != 0)
    {
      puts("Differences found in comparison of data fread() "
	   "after write() on '" TEST_FILE "'!");
      fail();
    }

  if (fseek(fp, sizeof(buf), SEEK_SET))
    {
      puts("Unable to fseek() '" TEST_FILE "'!");
      fail();
    }

  if (fwrite(buf, sizeof(buf), 1, fp) != 1)
    {
      puts("Unable to fwrite() '" TEST_FILE "'!");
      fail();
    }

  if (fseek(fp, sizeof(buf), SEEK_SET))
    {
      puts("Unable to fseek() '" TEST_FILE "'!");
      fail();
    }

  if (fread(buf2, sizeof(buf2), 1, fp) != 1)
    {
      puts("Unable to fread() '" TEST_FILE "'!");
      fail();
    }

  if (memcmp(buf, buf2, sizeof(buf)) != 0)
    {
      puts("Differences found in comparison of data fread() "
	   "after fwrite() on '" TEST_FILE "'!");
      fail();
    }

  if (fclose(fp) == EOF)
    {
      puts("Unable to fclose() '" TEST_FILE "'!");
      fail();
    }

  /* --- Directory testing --- */

  /* - POSIX I/O functions - */

  /* Try opening the directory for writes. */
  fd = open(TEST_DIR, O_RDWR);
  if ((fd >= 0) || ((fd < 0) && (errno != EISDIR)))
    {
      if (fd >= 0)
	puts("Unexpectedly able to open() '" TEST_DIR "' for read-write!");
      else
	perror(argv[0]);

      fail();
    }

  fd = open(TEST_DIR, O_WRONLY);
  if ((fd >= 0) || ((fd < 0) && (errno != EISDIR)))
    {
      if (fd >= 0)
	puts("Unexpectedly able to open() '" TEST_DIR "' for write-only!");
      else
	perror(argv[0]);

      fail();
    }

  /* Try opening the file for reads. */
  fd = open(TEST_DIR, O_RDONLY);
  if (fd < 0)
    {
      puts("Unable to open() '" TEST_DIR "'!");
      perror(argv[0]);
      fail();
    }

  /* Try read, write. */
  memset(buf, 'X', sizeof(buf));

  ret = read(fd, buf, sizeof(buf));
  if ((ret >= 0) || ((ret < 0) && (errno != EISDIR)))
    {
      if (ret >= 0)
	puts("Unexpectedly able to read() from '" TEST_DIR "'!");
      else
	perror(argv[0]);

      fail();
    }

  ret = write(fd, buf, sizeof(buf));
  if ((ret >= 0) || ((ret < 0) && (errno != EBADF)))
    {
      if (ret >= 0)
	puts("Unexpectedly able to write() to '" TEST_DIR "'!");
      else
	perror(argv[0]);

      fail();
    }

  /* Try lseek. */
  offset = lseek(fd, 42L, SEEK_SET);
  if (offset != 0)
    {
      if (offset != -1)
	printf("Unexpected result from lseek() on '%s': %ld\n",
	       TEST_DIR, (long) offset);
      else
	perror(argv[0]);

      fail();
    }

  /* Try ftruncate. */
  ret = ftruncate(fd, 20L);
  if ((ret != -1) || ((ret == -1) && (errno != EINVAL)))
    {
      if (ret != -1)
	puts("Unexpectedly able to ftruncate() '" TEST_DIR "'!");
      else
	perror(argv[0]);

      fail();
    }

  /* Try fchmod. Make the directory read-only. */
  ret = fchmod(fd, S_IRUSR);
  if (ret < 0)
    {
      perror(argv[0]);
      fail();
    }

  /* Try fchown. */
  ret = fchown(fd, getuid() * 2, getgid() * 2);
  if (ret < 0)
    {
      perror(argv[0]);
      fail();
    }

  /* Try ioctl. Check that we get the same device info as _get_dev_info. */
  dev_info = _get_dev_info(fd);
  if (dev_info == -1)
    {
      puts("Unable to get device information for '" TEST_DIR "'!");
      perror(argv[0]);
      fail();
    }

  ret = ioctl(fd, DOS_GETDEVDATA);
  if ((ret == -1) || ((ret & 0xffff) != (dev_info & 0xffff)))
    {
      if (ret != dev_info)
	{
	  puts("Different device information from ioctl() for '"
	       TEST_DIR "'!");
	  printf("_get_dev_info() vs. ioctl(): %d vs. %d\n",
		 dev_info, ret);
	}
      else
	{
	  perror(argv[0]);
	}

      fail();
    }

  /* Try fcntl. Check that the close-on-exec flag is set. */
  ret = fcntl(fd, F_GETFD);
  if ((ret < 0) || ((ret & FD_CLOEXEC) == 0))
    {
      if (ret >= 0)
	puts("FD_CLOEXEC was not set for '" TEST_DIR "'!");
      else
	perror(argv[0]);

      fail();
    }

  /* Try lockf. */
  ret = lockf(fd, F_TLOCK, 1024);
  if ((ret >= 0) || ((ret < 0) && (errno != EINVAL)))
    {
      if (ret >= 0)
	puts("Unexpectedly able to lockf() '" TEST_DIR "'!");
      else
	perror(argv[0]);

      fail();
    }

  /* Try fstat. Compare the result with that of stat. */
  ret = fstat(fd, &fstat_buf);
  if (ret < 0)
    {
      puts("fstat() failed!");
      perror(argv[0]);
      fail();
    }

  ret = stat(TEST_DIR, &stat_buf);
  if (ret < 0)
    {
      puts("stat() failed!");
      perror(argv[0]);
      fail();
    }

  if (memcmp(&stat_buf, &fstat_buf, sizeof(stat_buf)) != 0)
    {
      puts("fstat() and stat() returned different results!");
      fail();
    }

  if (!S_ISDIR(stat_buf.st_mode))
    {
      puts("fstat() doesn't think '" TEST_DIR "' is a directory!");
      fail();
    }

  /* Copied & modified from src/libc/posix/sys/stat/stat.c. */
  printf("%s: %d %6u %o %d %d %ld %lu %s", __get_fd_name(fd),
	 stat_buf.st_dev,
	 (unsigned)stat_buf.st_ino,
	 stat_buf.st_mode,
	 stat_buf.st_nlink,
	 stat_buf.st_uid,
	 (long)stat_buf.st_size,
	 (unsigned long)stat_buf.st_mtime,
	 ctime(&stat_buf.st_mtime));

  printf("\t\t\tBlock size: %d\n",
	 stat_buf.st_blksize);

  _djstat_describe_lossage(stderr);

  /* Try select. */
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_ZERO(&except_fds);

  FD_SET(fd, &read_fds);
  FD_SET(fd, &write_fds);
  FD_SET(fd, &except_fds);

  ret = select(fd + 1, &read_fds, &write_fds, &except_fds, NULL);
  if (ret < 0)
    {
      puts("select() failed!");
      perror(argv[0]);
      fail();
    }

  if (ret != 1)
    {
      puts("select() did not return the expected number!");
      fail();
    }

  if (!FD_ISSET(fd, &read_fds))
    {
      puts("Directory '" TEST_DIR "' should be ready for reading!");
      fail();
    }

  if (FD_ISSET(fd, &write_fds))
    {
      puts("Directory '" TEST_DIR "' should not be ready for writing!");
      fail();
    }

  if (FD_ISSET(fd, &except_fds))
    {
      puts("Directory '" TEST_DIR "' should not have an error!");
      fail();
    }

  /* Try fsync. */
  ret = fsync(fd);
  if ((ret >= 0) || ((ret < 0) && (errno != EINVAL)))
    {
      if (ret >= 0)
	puts("Unexpectedly able to fsync() to '" TEST_DIR "'!");
      else
	perror(argv[0]);

      fail();
    }

  /* Try fchdir. */
  ret = fchdir(fd);
  if (ret < 0)
    {
      perror(argv[0]);
      fail();
    }

  /* Where are we? */
  if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
      perror(argv[0]);
      fail();
    }

  p = basename(cwd);
  if (p == NULL)
    {
      printf("basename() failed on '%s'\n", cwd);
      fail();
    }

  if (strcasecmp(p, TEST_DIR) != 0)
    {
      printf("Now in '%s'; expected to be in '%s'\n", p, TEST_DIR);
      fail();
    }

  /* Change back. */
  ret = chdir("..");
  if (ret < 0)
    {
      perror(argv[0]);
      fail();
    }

  /* Done. */
  ret = close(fd);
  if (ret < 0)
    {
      puts("Unable to close '" TEST_DIR "'!");
      perror(argv[0]);
      fail();
    }

  /* - C stream I/O functions - */

  /* Try various flags on fopen. */
  fp = fopen(TEST_DIR, "wt");
  if (fp != NULL)
    {
      puts("Unexpectedly able to fopen() '" TEST_DIR "' for writing!");
      fail();
    }

  fp = fopen(TEST_DIR, "w+t");
  if (fp != NULL)
    {
      puts("Unexpectedly able to fopen() '" TEST_DIR "' for read & writing!");
      fail();
    }

  fp = fopen(TEST_DIR, "at");
  if (fp != NULL)
    {
      puts("Unexpectedly able to fopen() '" TEST_DIR "' for read & writing!");
      fail();
    }

  fp = fopen(TEST_DIR, "a+t");
  if (fp != NULL)
    {
      puts("Unexpectedly able to fopen() '" TEST_DIR "' for read & writing!");
      fail();
    }

  fp = fopen(TEST_DIR, "rt");
  if (fp == NULL)
    {
      puts("Unable to fopen() '" TEST_DIR "' for reading!");
      fail();
    }

  /* Try fread, getc, fgetc, fgets. */
  if (fread(buf, sizeof(buf), 1, fp) != 0)
    {
      puts("Unexpectedly able to fread() '" TEST_DIR "'!");
      fail();
    }

  if (getc(fp) != EOF)
    {
      puts("Unexpectedly able to getc() '" TEST_DIR "'!");
      fail();
    }

  if (fgetc(fp) != EOF)
    {
      puts("Unexpectedly able to fgetc() '" TEST_DIR "'!");
      fail();
    }

  if (fgets(buf, sizeof(buf), fp) != NULL)
    {
      puts("Unexpectedly able to fgets() '" TEST_DIR "'!");
      fail();
    }

  /* Try ungetc. */
  if (ungetc('X', fp) != EOF)
    {
      puts("Unexpectedly able to ungetc() '" TEST_DIR "'!");
      fail();
    }

  /* Try fwrite, putc, fputc, fputs. */
  memset(buf, 'X', sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';

  if (fwrite(buf, sizeof(buf), 1, fp) != 0)
    {
      puts("Unexpectedly able to fwrite() '" TEST_DIR "'!");
      fail();
    }

  if (fputc('X', fp) != EOF)
    {
      puts("Unexpectedly able to fputc() '" TEST_DIR "'!");
      fail();
    }

  if (fputs(buf, fp) != EOF)
    {
      puts("Unexpectedly able to fputs() '" TEST_DIR "'!");
      fail();
    }

  /* Try fgetpos, ftell. */
  if (fgetpos(fp, &fpos))
    {
      puts("Unable to fgetpos() '" TEST_DIR "'!");
      fail();
    }

  l = ftell(fp);

  if (l == -1)
    {
      puts("Unable to ftell() '" TEST_DIR "'!");
      fail();
    }

  if (l != 0)
    {
      puts("Unexpected non-zero offset from ftell() '" TEST_DIR "'!");
      fail();
    }

  /* Try fsetpos, fseek. */
  fpos = 1024;

  if (fsetpos(fp, &fpos))
    {
      puts("Unable to fsetpos() '" TEST_DIR "'!");
      fail();
    }

  if (fseek(fp, 1024, SEEK_SET))
    {
      puts("Unable to fseek() '" TEST_DIR "'!");
      fail();
    }

  /* Try rewind. */
  rewind(fp);

  /* Try fgetc, fputc again, to check rewind hasn't removed
   * the EOF condition. */
  if (fgetc(fp) != EOF)
    {
      puts("Unexpectedly able to fgetc() '" TEST_DIR "' after rewind!");
      fail();
    }

  if (fputc('X', fp) != EOF)
    {
      puts("Unexpectedly able to fputc() '" TEST_DIR "' after rewind!");
      fail();
    }

  /* Check that we're at EOF. */
  if (!feof(fp))
    {
      puts("'" TEST_DIR "' should always be at EOF!");
      fail();
    }

  /* Check that there are no errors. */
  if (ferror(fp))
    {
      puts("Unexpected error for '" TEST_DIR "'!");
      fail();
    }

  /* Try fpurge. */
  if (fpurge(fp))
    {
      puts("Unable to fpurge() '" TEST_DIR "'!");
      fail();
    }

  /* Try fclose. */
  if (fclose(fp) == EOF)
    {
      puts("Unable to fclose() '" TEST_DIR "'!");
      fail();
    }

  puts("PASS");
  return(EXIT_SUCCESS);
}
