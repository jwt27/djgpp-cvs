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
 *   5. Open a symlink with O_NOLINK but with symlinks in leading dirs.
 *   6. Open a symlink file in a simple subdir. Check if we really have opened
 *        the referred file. 
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void test_success(int testnum, const char * fn, int flags,
                         const char * data);

int main(void)
{
   int fd;
   if (!__file_exists("file1") || !__file_exists("test1") || 
       !__file_exists("test2") || !__file_exists("dir/file1"))
   {
      fprintf(stderr, "Required data file is missing\n");
      exit(1);
   }
   test_success(1, "test1", O_RDONLY, "file1");
   test_success(2, "test1", O_RDONLY | O_NOLINK, "!<symlink>");
   test_success(3, "test2/file1", O_RDONLY | O_NOFOLLOW, "file1");
   fd = open("test2/test1", O_RDONLY | O_NOFOLLOW);
   if (fd != -1)
   {
      fprintf(stderr, "Test 4 failed - unexpected open() success.\n");
      exit(1);
   }
   if (errno != ELOOP)
   { 
      perror("Test 4 failed - wrong errno returned ");
      exit(1);
   }
   printf("Test 4 passed\n");
   test_success(5, "test2/test1", O_RDONLY | O_NOLINK, "!<symlink>");
   test_success(6, "dir/test2", O_RDONLY, "tstlink2");
   return 0;
} 

static void test_success(int testnum, const char * fn, int flags, 
                         const char * data)
{
   char err_buf[50];
   int bytes_read;
   char buffer[100];
   int fd = open(fn, flags);
   int data_size = strlen(data);
   if (fd == -1)
   {            
      sprintf(err_buf, "Test %d failed - unexpected open() failure ", testnum);
      perror(err_buf);
      exit(1);
   }
   bytes_read = read(fd, buffer, data_size);
   if (bytes_read == -1)
   {
      sprintf(err_buf, "Test %d failed - unexpected read() failure ", testnum);
      perror(err_buf);
      close(fd);
      exit(1);
   }
   if (bytes_read != data_size)
   {
      fprintf(stderr, 
      "Test %d failed - read() returned less bytes than expected.\n", testnum);
      buffer[bytes_read] = '\0';
      printf("buffer = %s\n", buffer);
      close(fd);
      exit(1);
   }
   if (strncmp(buffer, data, data_size))
   {
      fprintf(stderr, "Test %d failed - read() returned wrong file data.\n", 
              testnum);
      close(fd);
      exit(1);
   }
   close(fd);
   printf("Test %d passed\n", testnum);
}
