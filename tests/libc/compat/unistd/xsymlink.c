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
 *  10. 3 with a double-slash
 *  11. 7 with a double-slash
 *  12. 8 with a double-slash
 *  13. Regular file
 * Any unhandled cases are more than welcome.
 *
 * There are some tests based on the current-working directory:
 *   1. An absolute path using '..' to navigate through the directories.
 *   2. An absolute path without drive letter using '..' to navigate and
 *      not ending in current-working directory.
 *   3. A relative path using a drive letter and '..'.
 *   4. A relative path using a drive letter and one too many '..' over 3.
 *
 * And following are failure tests:
 *   1. Simple symlink loop.
 *   2. Symlink loop across directories
 *
 *     TODO: device a test, where symlink expands to absolute path, which,
 *  in turn, has symlinks somewhere inside. There was such a test once, but
 *  it was broken - it depended on testsuite location.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <libc/symlink.h>
#include <sys/stat.h>

typedef struct {
  const char *src;    /* Source: includes symlink(s) */
  const char *target; /* Target of the symlink */
} test_success_t;

static const test_success_t tests_success[] = {
  { "test1", "file1" },
  { "test4", "file2" },
  { "dir1/test1", "dir1/file1" },
  { "dirtest/file1", "dir1/file1" },
  { "dirtest/test1", "dir1/file1" },
  { "dirtest/test2", "/dev/env/DJDIR/bin/gcc.exe" },
  { "dirtest/test3", "c:\\autoexec.bat" },
  { "dirtest/test6/file", "dir1/dir2/file" },
  { "dir1/test7", "file" },

  /* Check that __solve_symlinks copes with double-slashes. */
  { "dir1//test1", "dir1/file1" },
  { "dirtest//test3", "c:\\autoexec.bat" },
  { "dirtest//test6//file", "dir1/dir2/file" },

  /* Non-symlinks */
  { "makefile", "makefile" }
};

static const int n_tests_success
= sizeof(tests_success) / sizeof(tests_success[0]);

static const char *tests_failure[] = {
  "fail1",
  "fail3"
};

static const int n_tests_failure
= sizeof(tests_failure) / sizeof(tests_failure[0]);

static int test_success(const int test_num,
			const char * slink,
			const char * expect);

static int test_failure(const int test_num, const char * slink);

int main(void)
{
   int ok = 1;
   int ret, i, j;

   if (!__file_exists("test1") || !__file_exists("test4") ||
       !__file_exists("test5") || !__file_exists("dirtest") ||
       !__file_exists("fail1") || !__file_exists("fail2") ||
       !__file_exists("fail3") || !__file_exists("dir1/fail1") ||
       !__file_exists("dir1/test1") || !__file_exists("dir1/test2") ||
       !__file_exists("dir1/test3") || !__file_exists("dir1/test4") ||
       !__file_exists("dir1/test5") || !__file_exists("dir1/test6") ||
       !__file_exists("dir1/test7") || access("dir1/dir2", D_OK))
   {
       fprintf(stderr, "Required data files not found\n");
       exit(1);
   }
   puts("Running __solve_symlinks() and readlink() testsuite:");

   puts("Tests that check __solve_symlinks() works:");
   for (i = 0; i < n_tests_success; i++) {
     ret = test_success(i + 1, tests_success[i].src, tests_success[i].target);
     if (!ret)
       ok = 0;
   }

   /* Construct some tests with relative paths. */
   puts("Tests that check __solve_symlinks() based on current directory:");
   {
     char cwd[PATH_MAX], path[PATH_MAX + 1];
     test_success_t test;
     int n_slashes = 0;
     char *cwd_without_drive = cwd;
     char *ptr = NULL;

     if (getcwd(cwd, sizeof(cwd) - 1) == NULL)
     {
       fprintf(stderr, "Error: Unable to get current working directory\n");
       return(EXIT_FAILURE);
     }

     if (cwd[0] && (cwd[1] == ':'))
       cwd_without_drive = cwd + 2;

     /* Count the number of slashes in the current directory => number
      * of '..' we can use. */
     for (ptr = strpbrk(cwd, "/\\"), n_slashes = 0;
	  ptr != NULL;
	  ptr = strpbrk(ptr + 1, "/\\"), n_slashes++) {;}

     j = 0;

     /* Try tests_success[0] with an absolute path with '..'. */
     j++;

     strcpy(path, cwd);
     for (i = 0; i < n_slashes; i++)
     {
       strcat(path, "/..");
     }
     strcat(path, cwd_without_drive);
     strcat(path, "/");
     strcat(path, tests_success[0].src);

     test.src    = path;
     test.target = tests_success[0].target;

     printf("Test %d: Solving %s\n", j, test.src);
     ret = test_success(j, test.src, test.target);
     if (!ret)
       ok = 0;

     /* Try tests_success[2] with an absolute path without drive letter
      * and with '..'. */
     j++;

     strcpy(path, cwd_without_drive);
     for (i = 0; i < n_slashes; i++)
     {
       strcat(path, "/..");
     }
     strcat(path, cwd_without_drive);
     strcat(path, "/");
     strcat(path, tests_success[2].src);

     test.src    = path;
     test.target = tests_success[2].target;

     printf("Test %d: Solving %s\n", j, test.src);
     ret = test_success(j, test.src, test.target);
     if (!ret)
       ok = 0;

     /* Try tests_success[0] with a drive-letter and relative path
      * with '..'. */
     j++;

     if (cwd[0] && (cwd[1] == ':'))
     {
       path[0] = cwd[0];
       path[1] = cwd[1];
       path[2] = '\0';
       for (i = 0; i < n_slashes; i++)
	 {
	   if (i)
	     strcat(path, "/..");
	   else
	     strcat(path, "..");
	 }
       strcat(path, cwd_without_drive);
       strcat(path, "/");
       strcat(path, tests_success[0].src);

       test.src    = path;
       test.target = tests_success[0].target;

       printf("Test %d: Solving %s\n", j, test.src);
       ret = test_success(j, test.src, test.target);
       if (!ret)
	 ok = 0;
     }
     else
     {
       printf("Test %d: No drive letter - skipping\n", j);
     }

     /* Try tests_success[0] with a drive-letter and relative path
      * with too many '..'. */
     j++;

     if (cwd[0] && (cwd[1] == ':'))
     {
       path[0] = cwd[0];
       path[1] = cwd[1];
       path[2] = '\0';
       for (i = 0; i <= n_slashes; i++)
	 {
	   if (i)
	     strcat(path, "/..");
	   else
	     strcat(path, "..");
	 }
       strcat(path, cwd_without_drive);
       strcat(path, "/");
       strcat(path, tests_success[0].src);

       test.src    = path;
       test.target = tests_success[0].target;

       printf("Test %d: Solving %s\n", j, test.src);
       ret = test_success(j, test.src, test.target);
       if (!ret)
	 ok = 0;
     }
     else
     {
       printf("Test %d: No drive letter - skipping\n", j);
     }
   }

   puts("Tests that check __solve_symlinks() failure cases:");
   for (i = 0; i < n_tests_failure; i++) {
     ret = test_failure(i + 1, tests_failure[i]);
     if (!ret)
       ok = 0;
   }

   if (ok) {
     puts("PASS");
     return(EXIT_SUCCESS);
   } else {
     puts("FAIL");
     return(EXIT_FAILURE);
   }
}

static int test_success(const int test_num,
			const char * slink,
			const char * expect)
{
   char real_name[FILENAME_MAX + 1];
   char real_fixed[FILENAME_MAX + 1];
   char expect_fixed[FILENAME_MAX + 1];
   char err_buf[50];
   if (!__solve_symlinks(slink, real_name))
   {
      sprintf(err_buf, "Test %d failed ", test_num);
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
      return(0);
   }
   printf("Test %d passed\n", test_num);
   return(1);
}

static int test_failure(const int test_num, const char * slink)
{
   char buf[FILENAME_MAX + 1];
   char err_buf[50];
   errno = 0;
   if (__solve_symlinks(slink, buf))
   {
      fprintf(stderr,
            "Test %d failed - __solve_symlinks suceeds when it should fail\n",
            test_num);
      exit(1);
   }
   if (errno != ELOOP)
   {
      sprintf(err_buf, "Test %d failed - wrong errno returned ", test_num);
      perror(err_buf);
      return(0);
   }
   printf("Test %d passed\n", test_num);
   return(1);
}
