/* Testsuite for __solve_symlinks()
 * There are following success tests:
 *   1. Simple case with symlink in current directory
 *   2. Recursive symlinks
 *   3. Symlink in subdirectory
 *   4. Real file in symlinked directory
 *   5. Symlink in symlinked directory
 *   6. Symlink in symlinked directory to UNIX-style absolute path
 *   7. The same with DOS-style absolute path
 *   8. Real file in a symlink subdir in a symlink subdir
 *   9. Symlink in a subdir to file in an upper dir
 * Any unhandled cases are more than welcome.
 *
 * And following are failure tests:
 *  11. Simple symlink loop.
 *  12. Symlink loop across directories
 *
 *     TODO: device a test, where symlink expands to absolute path, which,
 *  in turn, has symlinks somewhere inside. There was such a test once, but
 *  it was broken - it depended on testsuite location.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libc/symlink.h>
#include <sys/stat.h>

static void test_success(const char * slink, const char * expect);
static void test_failure(const char * slink);

static int test_num = 0;

int main(void)
{
   if (!__file_exists("test1") || !__file_exists("test4") ||
       !__file_exists("test5") || !__file_exists("dirtest") ||
       !__file_exists("fail1") || !__file_exists("fail2") ||
       !__file_exists("fail3") || !__file_exists("dir1/fail1") ||
       !__file_exists("dir1/test1") || !__file_exists("dir1/test2") ||
       !__file_exists("dir1/test3") || !__file_exists("dir1/test4") ||
       !__file_exists("dir1/test5") || !__file_exists("dir1/test6") ||
       !__file_exists("dir1/test7") || access("dir1/dir2", D_OK))
   {
       fprintf(stderr, "Required data files not found");
       exit(1);
   }
   printf("Running readlink() testsuite:\n");
   test_success("test1", "file1");
   test_success("test4", "file2");
   test_success("dir1/test1", "dir1/file1");
   test_success("dirtest/file1", "dir1/file1");
   test_success("dirtest/test1", "dir1/file1");
   test_success("dirtest/test2", "/dev/env/DJDIR/bin/gcc.exe");
   test_success("dirtest/test3", "c:\\autoexec.bat");
   test_success("dirtest/test6/file", "dir1/dir2/file");
   test_success("dir1/test7", "file");
   test_failure("fail1");
   test_failure("fail3");
   printf("Done.\n");
   return 0;
}

static void test_success(const char * slink, const char * expect)
{
   char real_name[FILENAME_MAX + 1];
   char real_fixed[FILENAME_MAX + 1];
   char expect_fixed[FILENAME_MAX + 1];
   char err_buf[50];
   if (!__solve_symlinks(slink, real_name))
   {
      sprintf(err_buf, "Test %d failed ", ++test_num);
      perror(err_buf);
      exit(1);
   }
   _fixpath(expect, expect_fixed);
   _fixpath(real_name, real_fixed);
   if (strcmp(real_fixed, expect_fixed))
   {
      fprintf(stderr,
            "Test %d failed - __solve_symlinks returns wrong resolved path\n",
            test_num);
      fprintf(stderr, "Returned path: %s\n", real_fixed);
      fprintf(stderr, "Expected path: %s\n", expect_fixed);
      exit(1);
   }
   printf("Test %d passed\n", test_num);
}

static void test_failure(const char * slink)
{
   char buf[FILENAME_MAX + 1];
   char err_buf[50];
   errno = 0;
   if (__solve_symlinks(slink, buf))
   {
      fprintf(stderr,
            "Test %d failed - __solve_symlinks suceeds when it should fail\n",
            ++test_num);
      exit(1);
   }
   if (errno != ELOOP)
   {
      sprintf(err_buf, "Test %d failed - wrong errno returned ", test_num);
      perror(err_buf);
      exit(1);
   }
   printf("Test %d passed\n", test_num);
}
