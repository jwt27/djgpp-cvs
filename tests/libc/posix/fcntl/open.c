/* Testsuite for open()
 * TODO: only symlink handling aspect is checked. Other things should be
 * checked too.
 * Currently there are following tests:
 *   1. Open a symlink. Check if we really have opened a referred file.
 *   2. Open a symlink with O_NOLINK flag. Check if we really have opened a
 *        symlink file itself.
 *   3. Open simple file in a symlink subdir with O_NOFOLLOW flag. Check if
 *        we really have opened a referred file.
 *   4. Open symlink in a symlink subdir with O_NOFOLLOW flag. Should fail with
 *        ELOOP.
 *   5. Open an existing symlink with O_CREAT|O_EXCL. Should fail
 *        with EEXIST.
 *   6. Open an existing symlink with O_CREAT|O_EXCL|O_NOLINK. Should fail
 *        with EEXIST.
 *   7. Open a symlink with O_NOLINK but with symlinks in leading dirs.
 *   8. Open a symlink file in a simple subdir. Check if we really have opened
 *        the referred file.
 *   9. Open and close a file with O_TEMPORARY. Verify the file is deleted.
 *   10. Open a file with O_TEMPORARY. Duplicate the file handle. Close both
 *       handles. Verify the file is deleted at the correct time.
 *   11. Open a file with O_TEMPORARY. Duplicate the file handle to an
 *       unused file handle. Close both handles. Verify the file is deleted
 *       at the correct time.
 *   12. Open a file with O_TEMPORARY. Open the same file without O_TEMPORARY.
 *       Close both handles. Verify the file is deleted at the correct time.
 *   13. Open a file without O_TEMPORARY. Open the same file with O_TEMPORARY.
 *       Duplicate the handle opened without O_TEMPORARY on to the handle
 *       opened with O_TEMPORARY. Close both handles. Verify the file is
 *       deleted at the correct time.
 *   14. Open a file with O_TEMPORARY. Open it again with O_TEMPORARY.
 *       Duplicate one handle on to the other. Close both handles. Verify the
 *       file is deleted at the correct time.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

static void test_success(const char * fn, int flags,
                         const char * data);
void test_o_temporary(void);

static int testnum;

int main(void)
{
   int fd;

   if (!__file_exists("file1") || !__file_exists("test1") ||
       !__file_exists("test2") || !__file_exists("dir/file1"))
   {
      fprintf(stderr, "Required data file is missing\n");
      exit(EXIT_FAILURE);
   }

   if (__file_exists("doesnotexist"))
   {
      fprintf(stderr,
	      "File 'doesnotexist' is in the way - "
	      "please remove it!\n");
      exit(EXIT_FAILURE);
   }

   test_success("test1", O_RDONLY, "file1");
   test_success("test1", O_RDONLY | O_NOLINK, "!<symlink>");
   test_success("test2/file1", O_RDONLY | O_NOFOLLOW, "file1");

   ++testnum;
   fd = open("test2/test1", O_RDONLY | O_NOFOLLOW);
   if (fd != -1)
   {
      fprintf(stderr, "Test %d failed - unexpected open() success.\n",
	      testnum);
      exit(EXIT_FAILURE);
   }
   if (errno != ELOOP)
   {
      fprintf(stderr, "Test %d failed - wrong errno returned: %s\n",
	      testnum, strerror(errno));
      exit(EXIT_FAILURE);
   }
   printf("Test %d passed\n", testnum);

   ++testnum;
   fd = open("test3", O_RDONLY | O_CREAT | O_EXCL);
   if (fd != -1)
   {
      fprintf(stderr, "Test %d failed - unexpected open() success.\n",
	      testnum);
      exit(EXIT_FAILURE);
   }
   if (errno != EEXIST)
   {
      fprintf(stderr, "Test %d failed - wrong errno returned: %s\n",
	      testnum, strerror(errno));
      exit(EXIT_FAILURE);
   }
   printf("Test %d passed\n", testnum);

   ++testnum;
   fd = open("test3", O_RDONLY | O_CREAT | O_EXCL | O_NOLINK);
   if (fd != -1)
   {
      fprintf(stderr, "Test %d failed - unexpected open() success.\n",
	      testnum);
      exit(EXIT_FAILURE);
   }
   if (errno != EEXIST)
   {
      fprintf(stderr, "Test %d failed - wrong errno returned: %s\n",
	      testnum, strerror(errno));
      exit(EXIT_FAILURE);
   }
   printf("Test %d passed\n", testnum);

   test_success("test2/test1", O_RDONLY | O_NOLINK, "!<symlink>");
   test_success("dir/test2", O_RDONLY, "tstlink2");
   test_o_temporary();
   puts("PASS");
   return EXIT_SUCCESS;
}

static void test_success(const char * fn, int flags,
                         const char * data)
{
   char err_buf[50];
   int bytes_read;
   char buffer[100];
   int fd = open(fn, flags);
   int data_size = strlen(data);

   ++testnum;
   if (fd == -1)
   {
      sprintf(err_buf, "Test %d failed - unexpected open() failure ", testnum);
      perror(err_buf);
      exit(EXIT_FAILURE);
   }
   bytes_read = read(fd, buffer, data_size);
   if (bytes_read == -1)
   {
      sprintf(err_buf, "Test %d failed - unexpected read() failure ", testnum);
      perror(err_buf);
      close(fd);
      exit(EXIT_FAILURE);
   }
   if (bytes_read != data_size)
   {
      fprintf(stderr,
      "Test %d failed - read() returned less bytes than expected.\n", testnum);
      buffer[bytes_read] = '\0';
      printf("buffer = %s\n", buffer);
      close(fd);
      exit(EXIT_FAILURE);
   }
   if (strncmp(buffer, data, data_size))
   {
      fprintf(stderr, "Test %d failed - read() returned wrong file data.\n",
              testnum);
      close(fd);
      exit(EXIT_FAILURE);
   }
   close(fd);
   printf("Test %d passed\n", testnum);
}

const char temp_test_file[]="otemp";

int open_temp_test_file(int flags)
{
  char err_buf[64];
  int fd = open(temp_test_file, flags, S_IWUSR);

  if (fd == -1)
  {
    sprintf(err_buf, "Test %d failed - unexpected open() failure ", testnum);
    perror(err_buf);
    exit(EXIT_FAILURE);
  }
  return fd;
}

void start_temp_test(void)
{
  int fd;

  ++testnum;
  fd = open_temp_test_file(O_WRONLY | O_CREAT | O_TRUNC);
  write(fd, temp_test_file, sizeof(temp_test_file) - 1);
  close(fd);
}

#define FILE_SHOULD_NOT_EXIST   0
#define FILE_SHOULD_STILL_EXIST 1

void close_temp_test_file(int fd, int should_exist)
{
  const char *msg[]={"File was not deleted.",
                     "File was deleted too soon."};
  int exists;

  close(fd);
  exists = __file_exists(temp_test_file) ? 1 : 0;
  if (exists ^ should_exist)
  {
    fprintf(stderr, "Test %d failed - %s\n", testnum, msg[should_exist]);
    exit(EXIT_FAILURE);
  }
}

void test_o_temporary(void)
{
  int fd1, fd2;

  start_temp_test();
  fd1 = open_temp_test_file(O_RDONLY | O_TEMPORARY);
  close_temp_test_file(fd1, FILE_SHOULD_NOT_EXIST);
  printf("Test %d passed\n", testnum);

  start_temp_test();
  fd1 = open_temp_test_file(O_RDONLY | O_TEMPORARY);
  fd2 = dup(fd1);
  close_temp_test_file(fd1, FILE_SHOULD_STILL_EXIST);
  close_temp_test_file(fd2, FILE_SHOULD_NOT_EXIST);
  printf("Test %d passed\n", testnum);

  start_temp_test();
  fd1 = open_temp_test_file(O_RDONLY | O_TEMPORARY);
  fd2 = 128;
  dup2(fd1, fd2);
  close_temp_test_file(fd1, FILE_SHOULD_STILL_EXIST);
  close_temp_test_file(fd2, FILE_SHOULD_NOT_EXIST);
  printf("Test %d passed\n", testnum);

  start_temp_test();
  fd1 = open_temp_test_file(O_RDONLY | O_TEMPORARY);
  fd2 = open_temp_test_file(O_RDONLY);
  close_temp_test_file(fd1, FILE_SHOULD_STILL_EXIST);
  close_temp_test_file(fd2, FILE_SHOULD_NOT_EXIST);
  printf("Test %d passed\n", testnum);

  start_temp_test();
  fd1 = open_temp_test_file(O_RDONLY);
  fd2 = open_temp_test_file(O_RDONLY | O_TEMPORARY);
  dup2(fd1, fd2);
  close_temp_test_file(fd1, FILE_SHOULD_STILL_EXIST);
  close_temp_test_file(fd2, FILE_SHOULD_NOT_EXIST);
  printf("Test %d passed\n", testnum);

  start_temp_test();
  fd1 = open_temp_test_file(O_RDONLY | O_TEMPORARY);
  fd2 = open_temp_test_file(O_RDONLY | O_TEMPORARY);
  dup2(fd1, fd2);
  close_temp_test_file(fd1, FILE_SHOULD_STILL_EXIST);
  close_temp_test_file(fd2, FILE_SHOULD_NOT_EXIST);
  printf("Test %d passed\n", testnum);
}

