/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>
#include <dir.h>
#include <crt0.h>
#include <fcntl.h>
#include <libc/environ.h>
#include <libc/bss.h>

static int env_changed = 0;
static int srchpath_bss_count = -1;
 
/* Search PATH for FILE.
   If successful, store the full pathname in static buffer and return a
   pointer to it.
   If not sucessful, return NULL.
   This is what the Borland searchpath() library function does.
*/
 
char *
searchpath(const char *file)
{
  static char found[PATH_MAX];
  static char *path;

  memset(found, 0, sizeof(found));
 
  /* Get $PATH and store it for reuse.  */
  if (path == 0
      || srchpath_bss_count != __bss_count
      || env_changed != __environ_changed)
  {
    char *p = getenv("PATH");

    if (path && srchpath_bss_count == __bss_count)
      free(path);
    path = (char *)calloc(p ? strlen(p) + 3 : 2, sizeof(char));
    if (path == (char *)0)
      return (char *)0;
 
    /* Prepend `.' to the PATH, so current directory
       is always searched first.  */
    path[0] = '.';
 
    if (p)
    {
      register char *s, *name_start = 0;
      int preserve_case = _preserve_fncase();
 
      path[1] = ';';
      strcpy(path+2, p);

      /* switch FOO\BAR to foo/bar, downcase where appropriate */
      for (s = path + 2, name_start = s; *name_start; s++)
      {
	char lname[FILENAME_MAX], sname[13];

	if (*s == '\\')
	  *s = '/';
	if (s == name_start)
	  continue;
	if (*s == ':')
	  name_start = s + 1;
	else if (!preserve_case && (*s == '/' || *s == ';' || *s == '\0'))
	{
	  memcpy(lname, name_start+1, s - name_start - 1);
	  lname[s - name_start - 1] = '\0';
	  if (!strcmp(_lfn_gen_short_fname(lname, sname), lname))
	  {
	    name_start++;
	    while (name_start < s)
	    {
	      if (*name_start >= 'A' && *name_start <= 'Z')
		*name_start += 'a' - 'A';
	      name_start++;
	    }
	  }
	  else
	    name_start = s;
	}
	else if (*s == '\0')
	  break;
      }
    }
    else
      path[1] = 0;
  }
  if (strpbrk (file, "/\\:") != 0)
  {
    strcpy(found, file);
    return found;
  }
  else
  {
    char *test_dir = path;
 
    do {
      char *dp;
 
      dp = strchr(test_dir, ';');
      if (dp == (char *)0)
	dp = test_dir + strlen(test_dir);
 
      if (dp == test_dir)
	strcpy(found, file);
      else
      {
	strncpy(found, test_dir, dp - test_dir);
	found[dp - test_dir] = '/';
	strcpy(found + (dp - test_dir) + 1, file);
      }

      if (__file_exists(found))
	return found;

      if (*dp == 0)
	break;
      test_dir = dp + 1;
    } while (*test_dir != 0);
  }
 
  return NULL;
}

#ifdef TEST

int main(int argc, char *argv[])
{
  if (argc > 1)
    {
      char *found = searchpath (argv[1]);
      printf ("%s: %s%s\n",
	      argv[1], found ? "found as " : "not found", found ? found : " ");
    }
  return 0;
}

#endif
