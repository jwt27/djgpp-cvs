/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char *libs[] = { "gpp", "stdcxx", "m", NULL };

int gcc_not_found = 0;

static int
gcc_search(const char *lib)
{
  const char cmd_fmt[] = "gcc.exe -print-file-name=lib%s.a";
  char cmdbuf[FILENAME_MAX + sizeof(cmd_fmt)];
  FILE *fp;
  char lib_name[FILENAME_MAX];
  int retval = 0;

  sprintf(cmdbuf, cmd_fmt, lib);
  fp = popen(cmdbuf, "r");

  if (fp)
  {
    int fnlen = fread(lib_name, 1, FILENAME_MAX - 1, fp);

    if (fnlen != -1 && fnlen <= FILENAME_MAX - 1)
    {
      if (lib_name[fnlen-1] == '\n')
        fnlen--;
      lib_name[fnlen] = '\0';
      if (__file_exists(lib_name))
        retval = 1;
    }
    pclose(fp);
  }
  else if (!gcc_not_found)
  {
    perror("gxx: Cannot execute `gcc.exe'");
    gcc_not_found = 1;
  }

  return retval;
}

static const char *
library_installed(const char *lname)
{
  if (gcc_search(lname))
    return lname;
  else if (strlen(lname) > 5)
  {
    /* This library's file name exceeds the DOS 8+3 limits.  Maybe
       they have an LFN snafu.  */
    char *dos_alias = strdup(lname);

    if (dos_alias)
    {
      /* First, try the truncated name: libstdcxx.a -> libstdcx.a.  */
      dos_alias[5] = '\0';
      if (!gcc_search(dos_alias))
      {
	/* Finally, try the name with a numeric tail.  */
	strcpy(dos_alias + 3, "~1");
	if (!gcc_search(dos_alias))
	{
	  free(dos_alias);
	  return NULL;
	}
      }
    }

    return dos_alias;
  }
  else
    return NULL;
}

int
main(int argc, char **argv)
{
  char **newargs = (char **)malloc((argc+4) * sizeof(char *));
  int i, j;
  int add_libs = 1;

  if (newargs == NULL)
  {
    perror("gxx");
    return -1;
  }

  for (i=0; i<argc; i++)
  {
    if (strcmp(argv[i], "-c") == 0 || strncmp(argv[i], "--comp", 6) == 0
	|| strcmp(argv[i], "-S") == 0 || strncmp(argv[i], "--assem", 7) == 0
	|| strcmp(argv[i], "-E") == 0 || strncmp(argv[i], "--prep", 6) == 0)
      add_libs = 0;
    newargs[i] = argv[i];
  }

  if (add_libs) {
    for (j=0; libs[j]; j++)
    {
      const char *lib = library_installed(libs[j]);
      char libarg[FILENAME_MAX];

      if (lib)
      {
	sprintf(libarg, "-l%s", lib);
	newargs[i++] = strdup(libarg);
      }
    }
  }
  newargs[i++] = 0;

  return execvp("gcc.exe", newargs);
}
