/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dir.h>
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>

int
main(void)
{
  int i;

  glob_t flist;
  glob(".../*", 0, 0, &flist); /**/

  for (i = 0; i<(ssize_t)flist.gl_pathc; i++)
  {
    char pathp[300], fname[100], ext[100];
    fnsplit(flist.gl_pathv[i], 0, pathp, fname, ext);

    struct stat st;
    if (stat(flist.gl_pathv[i], &st) < 0)
      continue;

    char cline[200];
    struct tm *tm = localtime(&st.st_mtime);
    int y = tm->tm_year;
    if (y<80) y += 2000;
    if (y<200) y += 1900;

    // Compute what we'd like the first line to be
    if (!strcmp(ext, ".c")
	|| !strcmp(ext, ".h")
	|| !strcmp(ext, ".cc")
	|| !strcmp(ext, ".y")
	|| !strcmp(ext, ".s"))
    {
      sprintf(cline, "/* Copyright (C) %d DJ Delorie, see COPYING.DJ for details */\n", y);
    }
    else if (!strcmp(fname, "makefile") && !strcmp(ext, ""))
    {
      sprintf(cline, "# Copyright (C) %d DJ Delorie, see COPYING.DJ for details\n", y);
    }
    else if (!strcmp(ext, ".asm"))
    {
      sprintf(cline, "; Copyright (C) %d DJ Delorie, see COPYING.DJ for details\n", y);
    }
    else
      continue;

    // read first line
    FILE *f = fopen(flist.gl_pathv[i], "r");
    char line1[2000];
    fgets(line1, 2000, f);

    // It's OK
    if (strcmp(line1, cline) == 0)
    {
      fclose(f);
      continue;
    }

    // Is it copyright someone else?
    if (strstr(line1, "Copyright") && !strstr(line1, "Delorie"))
    {
      printf("%s: %s", flist.gl_pathv[i], line1);
      fclose(f);
      continue;
    }

    // We need to append the right copyright notice.

    char tmp[300];
    if (pathp[0] == 0)
      strcpy(tmp, "tempxxx.crn");
    else
      sprintf(tmp, "%s/tempxxx.crn", pathp);
    printf("updating %s\n", flist.gl_pathv[i]);

    FILE *wf = fopen(tmp, "w");
    fputs(cline, wf);
    fputs(line1, wf);
    while (fgets(line1, 2000, f))
      fputs(line1, wf);
    fclose(wf);

    fclose(f);

    rename(tmp, flist.gl_pathv[i]);

    // Don't set file time. If we set the file time, CVS will not notice
    // that the file has changed.
#if 0
    struct utimbuf ut;
    ut.actime = st.st_atime;
    ut.modtime = st.st_mtime;
    utime(flist.gl_pathv[i], &ut);
#endif
  }
  return 0;
}
