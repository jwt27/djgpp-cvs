/* Testsuite for __solve_symlinks()
 * There are following success tests:
 *   1. Simple case with symlink in current directory
 *   2. Recursive symlinks
 *   3. Symlink in subdirectory
 *   4. Real file in symlinked directory
 *   5. Symlink in symlinked directory
 *   6. Symlink in symlinked directory to UNIX-style absolute path
 *   7. The same with DOS-style absolute path
 *   8. Symlink in symlinked directory to absolute path with symlinks itself
 *   9. Real file in a symlink subdir in a symlink subdir
 *  10. Symlink in a subdir to file in an upper dir
 * Any unhandled cases are more than welcome.
 *
 * And following are failure tests:
 *  11. Simple symlink loop.
 *  12. Symlink loop across directories
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libc/symlink.h>
#include <sys/stat.h>

static void test_success(int num, const char * slink, const char * expect);
static void test_failure(int num, const char * slink);

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
   test_success( 1, "test1", "file1");
   test_success( 2, "test4", "file2");
   test_success( 3, "dir1/test1", "dir1/file1");
   test_success( 4, "dirtest/file1", "dir1/file1");
   test_success( 5, "dirtest/test1", "dir1/file1");
   test_success( 6, "dirtest/test2", "/dev/env/DJDIR/bin/gcc.exe");
   test_success( 7, "dirtest/test3", "c:\\autoexec.bat");
   test_success( 8, "dirtest/test4", "c:/dir/file");
   test_success( 9, "dirtest/test6/file", "dir1/dir2/file");
   /* Following one returns dir1/../file, which is perfectly valid but
      forces us to use _fixpath before comparisson */
   test_success(10, "dir1/test7", "file");
   test_failure(11, "fail1");
   test_failure(12, "fail3");
   printf("Done.\n");
   return 0;
}

static void test_success(int num, const char * slink, const char * expect)
{
   char real_name[FILENAME_MAX + 1];
   char real_fixed[FILENAME_MAX + 1];
   char expect_fixed[FILENAME_MAX + 1];
   char err_buf[50];
   if (!__solve_symlinks(slink, real_name))
   {
      sprintf(err_buf, "Test %d failed ", num);
      perror(err_buf);
      exit(1);
   }
   _fixpath(expect, expect_fixed);
   _fixpath(real_name, real_fixed);
   if (strcmp(real_fixed, expect_fixed))
   {
      fprintf(stderr,
            "Test %d failed - __solve_symlinks returns wrong resolved path\n",
            num);
      exit(1);
   }
   printf("Test %d passed\n", num);
}

static void test_failure(int num, const char * slink)
{
   char buf[FILENAME_MAX + 1];
   char err_buf[50];
   errno = 0;
   if (__solve_symlinks(slink, buf))
   {
      fprintf(stderr,
            "Test %d failed - __solve_symlinks suceeds when it should fail\n",
            num);
      exit(1);
   }
   if (errno != ELOOP)
   {
      sprintf(err_buf, "Test %d failed - wrong errno returned ", num);
      perror(err_buf);
      exit(1);
   }
   printf("Test %d passed\n", num);
}
