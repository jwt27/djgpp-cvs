/* Testsuite for symlink() */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LINK_CONT "whatever.file"

static void test_failure(int test_no, const char * source, const char * target,
                         int expect_errno);

int main(void)
{
   char *link_name;
   char fn[FILENAME_MAX + 1];
   int  bytes_read;
   if (!__file_exists("fail1") || !__file_exists("fail2"))
   {
      fprintf(stderr, "Cannot run testsuite - required data files not found\n");
      exit(1);
   }
   printf("Running symlink() testsuite:\n");
   /* Test if symlink generated file is understandable by readlink() */
   link_name = tmpnam(NULL);
   if (symlink(LINK_CONT, link_name))
   {
      fprintf(stderr, "Test 1 failed - unexpected symlink failure\n");
      exit(1);
   }
   bytes_read = readlink(link_name, fn, FILENAME_MAX);
   if (bytes_read == -1)
   {
      fprintf(stderr, "Test 1 failed - cannot read link made with symlink()\n");
      exit(1);
   }
   fn[bytes_read] = '\0';
   if (strcmp(LINK_CONT, fn))
   {
      fprintf(stderr, "Test 1 failed - wrong link contents\n");
      exit(1);
   }
   printf("Test 1 passed\n");
   remove(link_name);
   test_failure(2, NULL, "who.cares", EINVAL);
   test_failure(3, "cares.who", NULL, EINVAL);
   test_failure(4, "middle.of.nowhere", "fail1", ELOOP);
   test_failure(5, "nowhere.in.middle", "/dev/env/DJDIR/djgpp.env", EEXIST);
   return 0;
}

static void test_failure(int test_no, const char * source, const char * target,
                         int expect_errno)
{
   errno = 0;
   if (!symlink(source, target))
   {
      fprintf(stderr,
              "Test %d failed - unexpected symlink() success\n", test_no);
      exit(1);
   }
   if (errno != expect_errno)
   {
      char buf[50];
      sprintf(buf, "Test %d failed - wrong errno value ", test_no);
      perror(buf);
      exit(1);
   }
   printf("Test %d passed\n", test_no);
}
