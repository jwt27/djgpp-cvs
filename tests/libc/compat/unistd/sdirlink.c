/* Testsuite for __solve_dir_symlinks()
 * There are following success tests:
 *   1. Simple case with symlink in current directory
 *   2. Symlink with a trailing slash
 *   3. Symlink in subdirectory
 *   4. Real file in symlinked directory
 *   5. Symlink in symlinked directory
 *   6. Real file in a symlink subdir in a symlink subdir
 *   7. Root directory / or \
 * Any unhandled cases are more than welcome.
 *
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libc/symlink.h>
#include <sys/stat.h>

static void test_success(const char * slink, const char * expect);

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
   test_success("/", "/");
   test_success("c:/", "c:/");
   test_success("/dev/c", "/dev/c");
   test_success("/dev/c/", "/dev/c");
   test_success("test1", "test1");
   test_success("test1/", "test1");
   test_success("dir1/test1", "dir1/test1");
   test_success("dirtest/file1", "dir1/file1");
   test_success("dirtest/test1", "dir1/test1");
   test_success("dirtest/test6/file", "dir1/dir2/file");
   test_success("c:test1", "c:test1");
   symlink("c:/file", "c:/linkfile");
   test_success("c:/linkfile", "c:/linkfile");
   test_success("c:/djgpp/tests/libc/compat/unistd/file1",
                "c:/djgpp/tests/libc/compat/unistd/file1");
   test_success("/dev/env/DJDIR/bin/gcc.exe", "/dev/env/DJDIR/bin/gcc.exe");
   test_success("/dev/c/linkfile", "/dev/c/linkfile");
   test_success("/", "/");
   test_success("\\", "\\");
   remove("c:/linkfile");
   printf("Done.\n");
   return 0;
}

static void test_success(const char * slink, const char * expect)
{
   static int num = 1;
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
   printf("Test %d passed\n", num++);
}

