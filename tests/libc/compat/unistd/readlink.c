/* Testsuite for readlink()
 * There are following tests:
 *   1. Simple case with valid symlink
 *   2. Check if readlink writes beyond end of result buffer, if filename
 * was longer
 *   3 & 4. Check if it does not accept NULL args
 *   5. Check with not present symlink file
 *   6. Check with wrong size, OK contents symlink file
 *   7. Check with OK size, wrong contents symlink file
 */
 
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void failure_test(int testno, const char * linkname, char * buf,
                         int expect_errno,
                         const char * errmsg1, const char * errmsg2);

int main(void)
{
   int bytes_read;
   char buffer[FILENAME_MAX + 1];
   errno = 0;

   /* Check if we have required files */
   if (!__file_exists("test1") || !__file_exists("test2") ||
       !__file_exists("test3"))
   {
      fprintf(stderr, "Cannot run testsuite - required files missing");
      exit(1);
   }
   
   /* Test 1 - simple case with symlink OK */
   bytes_read = readlink("test1", buffer, FILENAME_MAX);
   if (bytes_read == -1)
   {
      perror("Test 1 failed ");
      exit(1);
   }
   /* Is buffer size OK? */
   if (bytes_read != (signed)strlen("file1"))
   {
      fprintf(stderr, "Test 1 failed - readlink returns wrong buffer size\n");
      exit(1);
   }
   /* Are buffer contents OK? */
   if (strncmp(buffer, "file1", bytes_read))
   {
      fprintf(stderr, "Test 1 failed - readlink returns wrong link value\n");
      exit(1);
   }
   printf("Test 1 passed\n");

   /* Test 2 - check if readlink does not overwrite buffer */
   memset(buffer, 0, sizeof(buffer));
   buffer[4] = 0x7F;
   bytes_read = readlink("test1", buffer, 4);
   /* Is buffer size OK? */
   if (bytes_read != 4)
   {
      fprintf(stderr, "Test 2 failed - readlink returns wrong buffer size\n");
      exit(1);
   }
   /* Are buffer contents OK? */
   if (strncmp(buffer, "file", bytes_read))
   {
      fprintf(stderr, "Test 2 failed - readlink returns wrong link value\n");
      exit(1);
   }
   /* Does readlink have security hole? */
   if (buffer[4] != 0x7F)
   {
      fprintf(stderr, "Test 2 failed - readlink writes beyond the buffer\n");
      exit(1);
   }
   printf("Test 2 passed\n");

   /* Tests 3 & 4 - stupid args */
   failure_test(3, NULL, buffer, EINVAL, "readlink accepts NULL arg",
                                 "readlink returns wrong errno for NULL arg");
   failure_test(4, "doesntmatter", NULL, EINVAL, "readlink accepts NULL arg",
                                 "readlink returns wrong errno for NULL arg");
                                 
   /* Test 5 - file not found */
   failure_test(5, "/Pink/Floyd/Animals/Dogs/Shouldnt/Exist", buffer, ENOENT,
                "readlink found non-existing file",
                "readlink returns wrong errno for non-existing file");

   /* Test 6: symlink file contents OK, size wrong */
   failure_test(6, "test2", buffer, EINVAL,
                "readlink accepted broken symlink file",
                "readlink returns wrong errno for broken file");

   /* Test 7: symlink file size wrong, contents OK */
   failure_test(7, "test3", buffer, EINVAL,
                "readlink accepted broken symlink file",
                "readlink returns wrong errno for broken file");

   return 0;
}

static void failure_test(int testno, const char * linkname, char * buf,
                  int expect_errno, const char * errmsg1, const char * errmsg2)
{
   int bytes_read;
   errno = 0;
   bytes_read = readlink(linkname, buf, FILENAME_MAX);
   if (bytes_read != -1)
   {
      fprintf(stderr, "Test %d failed - %s\n", testno, errmsg1);
      exit(1);
   }
   if (errno != expect_errno)
   {
      fprintf(stderr, "Test %d failed - %s\n", testno, errmsg2);
      exit(1);
   }
   printf("Test %d passed\n", testno);
}
