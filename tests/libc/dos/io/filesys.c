/* This program runs its command-line arguments through all the
   library functions which take file names as arguments, and reports
   any abnormal results.  All file-name oriented functions are tested,
   except those which are mere wrappers around other functions (for
   example, `utimes' is a trivial wrapper around `utime', and thus is
   not tested here).

   The original purpose of this program is to test support for special
   file names, like "/dev/x/foo" etc.  But you can use this program
   for any file names you like.

   Keep in mind that you are supposed to know what you are doing when
   you choose the file names to pass to the program.  The program
   expects each command-line argument to be a name of a regular
   existing file, which is deleted during the testing, and a directory
   by the same name is created in its stead (the directory is also
   deleted near the end of the test).  You can pass any other types of
   arguments to the program, like non-existing files, device name, etc.,
   but some of the tests might fail then.  For example, if you start with
   a directory, the test of `open' will fail even if there's nothing
   wrong with the underlying library functions.  */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/vfs.h>
#include <utime.h>
#include <sys/time.h>
#include <process.h>
#include <dir.h>
#include <io.h>

#include <libc/dosio.h>
#include <dpmi.h>
#include <go32.h>

#define ALL_FILES (FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_LABEL|FA_DIREC|FA_ARCH)

/* The directory where it all started.  */
static char *orig_dir;

/*
 *
 * Test functions.
 *
 */

/* Test functions which take zero to 2 arguments, besides the file
   name, and return an int status.  */
int t_misc (const char *func, int narg, const char *fname, int arg1, int arg2)
{
  int retval = -1;

  switch (narg)
    {
      case 0:
	if (strcmp (func, "chdir") == 0)
	  {
	    retval = chdir (fname);
	    chdir (orig_dir);
	  }
	else if (strcmp (func, "rmdir") == 0)
	  {
	    char d_root[4];

	    _put_path (fname);
	    dosmemget (__tb, 2, d_root);

	    /* If FNAME is on another drive, it can be the CWD on that
	       drive, and DOS won't let us remove it.  So we chdir to
	       the root of that drive, just in case. */
	    if (d_root[0] && d_root[1] == ':')
	      {
		d_root[2] = '/';
		d_root[3] = '\0';
		chdir (d_root);
	      }
	    retval = rmdir (fname);
	    chdir (orig_dir);
	  }
	else if (strcmp (func, "remove") == 0)
	  {
	    if (!__file_exists (fname))
	      mkdir (fname, 0666);
	    retval = remove (fname);
	  }
	else if (strcmp (func, "_use_lfn") == 0)
	  retval = _use_lfn (fname);
	else if (strcmp (func, "system") == 0)
	  retval = system (fname);
	break;
      case 1:
	if (strcmp (func, "_open") == 0)
	  {
	    retval = _open (fname, arg1);
	    if (retval > 2)
	      _close (retval);
	  }
	else if (strcmp (func, "chmod") == 0)
	  retval = chmod (fname, arg1);
	else if (strcmp (func, "access") == 0)
	  retval = access (fname, arg1);
	else if (strcmp (func, "mkdir") == 0)
	  retval = mkdir (fname, arg1);
	else if (strcmp (func, "pathconf") == 0)
	  retval = pathconf (fname, arg1);
	else if (strcmp (func, "truncate") == 0)
	  retval = truncate (fname, arg1);
	break;
      case 2:
	if (strcmp (func, "open") == 0)
	  {
	    retval = open (fname, arg1, arg2);
	    if (retval > 2)
	      close (retval);
	  }
	else if (strcmp (func, "_chmod") == 0)
	  retval = _chmod (fname, arg1, arg2);
	else if (strcmp (func, "_is_executable") == 0)
	  retval = _is_executable (fname, arg1, (const char *)arg2);
	else
	  retval = -1;
	break;
      default:
	retval = -1;
	break;
    }
  return retval;
}

/* Test functions which copy/rename/remove the file, or transform its
   file name.  These usually take two char * arguments.  */
int t_path (const char *func, int narg, const char *fname, int d1, int d2)
{
  static const char *report_fmt = "%s: `%s' -> `%s'\n";
  int retval;
  char tpath[PATH_MAX];
  int (*testee)(const char *, const char *) = link;

  d2 = d1;	/* pacify gcc -Wall */

  if (strcmp (func, "_fixpath") == 0)
    {
      _fixpath (fname, tpath);
      printf (report_fmt, func, fname, tpath);
      retval = 0;
    }
  else if ((strcmp (func, "link") == 0 ? testee = link : 0) != 0
	   || (strcmp (func, "symlink") == 0 ? testee = symlink : 0) != 0
	   || (strcmp (func, "rename") == 0 ? testee = rename : 0) != 0
	   || (strcmp (func, "_rename") == 0 ? testee = _rename : 0) != 0)
    {
      size_t fnlast = strlen (fname) - 1;

      /* Beginning with `!', look for a character which can be used to
	 generate a name of a non-existent file by replacing with it
	 the last character of FNAME.  */
      strcpy (tpath, fname);
      tpath[fnlast] = '!';
      while (__file_exists (tpath))
	{
	  tpath[fnlast]++;

	  /* Skip characters disallowed by DOS.  */
	  if (strchr (".+,;=[]|<>/\\\":?*", tpath[fnlast]))
	    tpath[fnlast]++;
	}
      retval = testee (fname, tpath);
      if (retval == 0)
	{
	  if (testee == rename || testee == _rename)
	    rename (tpath, fname);
	  else
	    {
	      remove (tpath);

	      /* `symlink' creates foo.exe */
	      if (testee == symlink)
		remove (strcat (tpath, ".exe"));
	    }
	}
    }
  else if (strcmp (func, "mkstemp") == 0)
    {
      char *s, *p;

      strcpy (tpath, fname);
      for (s = p = basename (tpath); p < s + 8; p++)
	*p = 'X';
      s[0] = 'f';
      s[1] = 't';
      *p = '\0';
      retval = mkstemp (tpath);
      printf (report_fmt, func, fname, tpath);
      if (retval > 2)
	{
	  close (retval);
	  remove (tpath);
	}
    }
  else if (strcmp (func, "tmpnam") == 0)
    {
      char *p = dirname (fname);

      if (setenv ("TMPDIR", p, 1) == -1)
	{
	  printf ("%s: setenv failed\n", func);
	  retval = -1;
	}
      else
	{
	  printf (report_fmt, func, fname, tmpnam (NULL));
	  retval = 0;
	}
      free (p);
    }
  else if (strcmp (func, "_truename") == 0)
    {
      char *p = _truename (fname, NULL);

      if (p)
	{
	  printf (report_fmt, func, fname, p);
	  retval = 0;
	  free (p);
	}
      else
	retval = -1;
    }
  else
    retval = -1;

  return retval;
}

/* Test findfirst.  */
int t_findfirst (const char *func, int narg, const char *fname, int attr, int dummy)
{
  struct ffblk ff;

  dummy = dummy;	/* shut up gcc -Wall */

  return findfirst (fname, &ff, attr);
}

/* Test fopen.  */
int t_fopen (const char *func, int narg, const char *fname, int d1, int d2)
{
  int retval;
  FILE *fp = fopen (fname, "r");

  d2 = d1;		/* keep gcc -Wall happy */

  retval = fp == NULL ? -1 : 0;

  if (fp)
    fclose (fp);
  return retval;
}

/* Test opendir.  */
int t_opendir (const char *func, int narg, const char *fname, int d1, int d2)
{
  int retval;
  char *fdir = dirname (fname);
  char *base = basename (fname);

  d2 = d1;		/* for gcc -Wall */

  if (!fdir)
    {
      printf ("%s: dirname failed\n", func);
      retval = -1;
    }
  else
    {
      DIR *dirp = opendir (fdir);

      free (fdir);
      if (!dirp)
	retval = -1;
      else
	{
	  struct dirent *de;

	  retval = -1;

	  /* See that we actually opened the right directory, by
	     reading it until we see the file FNAME.  */
	  while ((de = readdir (dirp)) != NULL)
	    {
	      if (strcasecmp (de->d_name, base) == 0)
		{
		  retval = 0;
		  break;
		}
	    }
	  closedir (dirp);
	}
    }
  return retval;
}

/* Test spawnl.  */
int t_spawn (const char *func, int narg, const char *fname, int arg, int dummy)
{
  dummy = dummy;	/* can't stand gcc -Wall whining */

  if (!_is_executable (fname, -1, NULL))
    {
      printf ("%s: %s is not an executable file, skipping\n", func, fname);
      return 0;
    }
  return spawnl (arg, fname, fname, NULL);
}

/* Test stat.  */
int t_stat (const char *func, int narg, const char *fname, int d1, int d2)
{
  struct stat st_buf;

  d2 = d1;		/* prevent gcc -Wall from complaining */
  return stat (fname, &st_buf);
}

/* Test statfs.  */
int t_statfs (const char *func, int narg, const char *fname, int d1, int d2)
{
  struct statfs sbuf;
  int retval = statfs (fname, &sbuf);

  d2 = d1;		/* avoid messages from gcc -Wall */

  if (retval == 0)
    {
      char dos_path[FILENAME_MAX];
      int dev_drv;

      /* Make sure "/dev/x/foo" reports the correct drive, not the
         current one.  */
      _fixpath (fname, dos_path);
      dev_drv = (dos_path[0] & 0x1f) - 1;
      if (dev_drv != sbuf.f_fsid[0])
	retval = -1;
    }
  return retval;
}

/* Test utime.  */
int t_utime (const char *func, int narg, const char *fname, int at, int mt)
{
  struct utimbuf tb = {(time_t)at, (time_t)mt};
  return utime (fname, &tb);
}

struct fs_test {
  const char *func_name;
  int nargs;
  int (*tester)(const char *, int, const char *, int, int);
  int arg1;
  int arg2;
  int result;
  int negate_result;
} test_vec[] = {  /* the order of the tests below is not entirely arbitrary! */
    {"_open", 1, t_misc, O_RDONLY, 0, -1, 1},
    {"open", 2, t_misc, O_RDONLY, 0666, -1, 1},
    {"_chmod", 2, t_misc, 0, 0, -1, 1},
    {"chmod", 1, t_misc, S_IWUSR, 0, 0, 0},
    {"access", 1, t_misc, R_OK, 0, 0, 0},
    {"findfirst", 2, t_findfirst, 0, ALL_FILES, 0, 0},
    {"_fixpath", 1, t_path, 0, 0, 0, 0},
    {"fopen", 1, t_fopen, 0, 0, 0, 0},
    {"_is_executable", 2, t_misc, -1, 0, 1, 0},
    {"link", 1, t_path, 0, 0, 0, 0},
    {"mkstemp", 0, t_path, 0, 0, -1, 1},
    {"opendir", 0, t_opendir, 0, 0, -1, 1},
    {"pathconf", 1, t_misc, _PC_NAME_MAX, 0, 256, 0},
    {"_rename", 1, t_path, 0, 0, 0, 0},
    {"rename", 1, t_path, 0, 0, 0, 0},
    {"spawnl", 2, t_spawn, P_WAIT, 0, -1, 1},
    {"stat", 1, t_stat, 0, 0, 0, 0},
    {"statfs", 0, t_statfs, 0, 0, 0, 0},
    {"symlink", 1, t_path, 0, 0, 0, 0},
    {"system", 0, t_misc, 0, 0, -1, 1},
    {"tmpnam", 0, t_path, 0, 0, 0, 0},
    {"_truename", 1, t_path, 0, 0, 0, 0},
    {"truncate", 1, t_misc, 0, 0, 0, 0},
    {"utime", 2, t_utime, 930000000, 925000000, 0, 0},
    {"remove", 0, t_misc, 0, 0, 0, 0},
    {"mkdir", 1, t_misc, S_IWUSR, 0, 0, 0},
    {"chdir", 0, t_misc, 0, 0, 0, 0},
    {"_use_lfn", 0, t_misc, 0, 0, 1, 0},
    {"rmdir", 0, t_misc, 0, 0, 0, 0}
};

/* Non-zero when verbose operation was requested.  */
static int verbose = 0;

/* Run a single file FILE through all the tests.  */
int fstest (const char *file)
{
  int i, retval = 0;

  for (i = 0; i < sizeof (test_vec) / sizeof (test_vec[0]); i++)
    {
      int result;

      if (verbose)
	printf ("\nTesting %s(%s)...", test_vec[i].func_name, file);
      errno = 0;
      result = test_vec[i].tester(test_vec[i].func_name, test_vec[i].nargs,
				  file, test_vec[i].arg1, test_vec[i].arg2);
      if (test_vec[i].negate_result
	  ? result == test_vec[i].result
	  : result != test_vec[i].result)
	{
	  printf ("%s%s(%s): incorrect result: %d (expected: %s%d)\n",
		  verbose ? "\n" : "",
		  test_vec[i].func_name, file, result,
		  test_vec[i].negate_result ? "NOT " : "",
		  test_vec[i].result);
	  if (errno)
	    printf ("System error was: %s\n", strerror (errno));
	  retval++;
	}
    }
  return retval;
}

int main (int argc, char **argv)
{
  if (argc == 1)
    {
      printf ("Usage: %s [-v] file...\n", argv[0]);
      return 0;
    }

  orig_dir = getcwd (NULL, PATH_MAX);

  if (strcmp (argv[1], "-v") == 0)
    {
      verbose = 1;
      --argc;
      ++argv;
    }

  while (--argc)
    {
      char *arg = *++argv;

      if (fstest (arg) == -1)
	return 1;
    }

  return 0;
}
