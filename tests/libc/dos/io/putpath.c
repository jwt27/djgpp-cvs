/* A test program for the various /dev/* transformations inside _put_path().
   Written by Markus F.X.J. Oberhumer.  */

#include <stdio.h>
#include <libc/environ.h>
extern char **environ;

static void p(const char *path)
{
  int l = _put_path(path);
  char *tmp = calloc(1, l+1);
  dosmemget(__tb, l, tmp);
  printf("%-45s %-28s %2d\n", path, tmp, l);
  free(tmp);
}

static void p2(const char *path)
{
  char *tmp = strdup(path);
  char *t;
  p(path);
  for (t = tmp; *t; t++)
    if (*t == '/')
      *t = '\\';
  p(tmp);
  free(tmp);
}

int main()
{
  /* first clear the environment the quick way */
  environ = (char **) calloc(1, sizeof(char *));
  __environ_changed++;
  /* setup out test environment */
  putenv("DJDIR=c:/djgpp2");
  putenv("HOME=c:\\home");

  /* normal dir tests */
  p2("/dir1/dir2");
  p2("/dir1/dir2/");
  p2("//dir1/dir2/");
  p2("/dir1//dir2/");
  p2("/dir1/dir2//");
  p2("/dev/foo");
  printf("\n");

  /* /dev/null tests */
  p2("/dev/null");
  p2("c:/dev/null");
  p2("/DEV/NULL");
  p2("/dev/null/");
  p2("/dev/null//");
  p2("//dev/null");
  p2("/dev//null");
  printf("\n");

  /* /dev/x/ tests */
  p2("/dev/c");
  p2("/dev/c/");
  p2("a:/dev/c/");
  p("a:/dev/c//");
  p("A:/dev/c/");
  p("a:/dev/C/");
  printf("\n");

  /* other /dev tests */
  p("/dev/fd/0");		/* common for stdin - shouldn't get changed */
  p2("/dev/con");
  p2("c:/dev/con");
  p("/dev/0");
  p("/dev/0/");
  printf("\n");

  /* /dev/env tests */
  p2("/dev/env/DJDIR");
  p2("a:/dev/env/DJDIR");
  p("/dev/env/foo");
  p("/dev/env/foo~");
  p("/dev/env/foo~x");
  p("/dev/env/foo~x~");
  p("/dev/env/foo~x~~");
  p("/dev/env/foo~x~~~");
  p("/dev/env/foo~x~~y");
  p("/dev/env/foo~x~~y~");
  p("/dev/env/foo~x~y~~");
  p("/dev/env/foo~x~~~/bar");
  printf("\n");

  /* nested tests */
  p("/dev/c/dev/null");
  p("/dev/c/dev/env/DJDIR");
  p("/dev/env/foo~/dev/env/foo~/bar");
  p("/dev/env/foo~/dev/env/DJDIR~/bar");
  p("/dev/env/a~/dev/env/b~~/dev/env/DJDIR~~~/bar");	/* double nested */
  printf("\n");

  /* deeply nested tests */
  putenv("X2=/dev/env/X1");
  putenv("X3=/dev/env/foo~/dev/env/X1~");
  putenv("X4=/dev/env/foo~/dev/env/X2~");
  putenv("X5=/dev/env/foo~/dev/env/X4~");
  {
    const char **x;
    static const char *x1[] = {
      "X1=a:/this/is/a/very/long/path",
      "X1=/dev/c/this/is/a/very/long/path",
      "X1=/",
      0,
    };
    for (x = x1; *x; x++)
    {
      printf("%s\n", *x);
      putenv(*x);
      p("/dev/env/X1");
      p("/dev/env/X2");
      p("/dev/env/X3");
      p("/dev/env/X4");
      p("/dev/env/X5");
      printf("\n");
    }
  }

  /* infinite recursion tests */
  /* these all should print an error message with ENOMEM
     and should *not* crash */
  putenv("A1=/dev/env/A1");
  errno = 0;
  p("/dev/env/A1");
  if (errno) perror("/dev/env/A1");
  errno = 0;
  putenv("A1=/dev/env/foo~/dev/env/A1~");
  p("/dev/env/A1");
  if (errno) perror("/dev/env/A1");
  errno = 0;
  putenv("A1=/dev/env/foo~/dev/env/A2~");
  putenv("A2=/dev/env/foo~/dev/env/A1~");
  p("/dev/env/A1");
  if (errno) perror("/dev/env/A1");

  return 0;
}
