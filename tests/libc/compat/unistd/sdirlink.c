/* Testsuite for __solve_dir_symlinks()
 * There are following success tests:
 *   1. Simple case with symlink in current directory
 *   2. Symlink with a trailing slash
 *   3. Symlink in subdirectory
 *   4. Real file in symlinked directory
 *   5. Symlink in symlinked directory
 *   6. Real file in a symlink subdir in a symlink subdir
 * Any unhandled cases are more than welcome.
 *
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libc/symlink.h>
#include <sys/stat.h>

static void test_success(int num, const char * slink, const char * expect);

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
   printf("Running __solve_dir_symlink() testsuite:\n");
   test_success( 1, "test1", "test1");
   test_success( 2, "test1/", "test1");
   test_success( 3, "dir1/test1", "dir1/test1");
   test_success( 4, "dirtest/file1", "dir1/file1");
   test_success( 5, "dirtest/test1", "dir1/test1");
   test_success( 6, "dirtest/test6/file", "dir1/dir2/file");
   test_success( 7, "c:test1", "c:test1");
   symlink("c:/file", "c:/linkfile");
   test_success( 8, "c:/linkfile", "c:/linkfile");
   remove("c:/linkfile");
   test_success( 9, "c:/djgpp/tests/libc/compat/unistd/file1", 
                    "c:/djgpp/tests/libc/compat/unistd/file1");
   printf("Done.\n");
   return 0;
}

static void test_success(int num, const char * slink, const char * expect)
{
   char real_name[FILENAME_MAX + 1];
   char real_fixed[FILENAME_MAX + 1];
   char expect_fixed[FILENAME_MAX + 1];
   char err_buf[50];
   if (!__solve_dir_symlinks(slink, real_name))
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
            "Test %d failed - __solve_dir_symlinks returns wrong resolved path\n",
            num);
      fprintf(stderr, "Expected   %s\n", expect_fixed);
      fprintf(stderr, "It returns %s\n", real_fixed);
      exit(1);
   }
   printf("Test %d passed\n", num);
}

