/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include <process.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
  int i;
  char cwd[200];
  char path[200];
  char file[200];
  FILE *oi = fopen("makefile.oi", "w");
  FILE *rf = fopen("makefile.rf2", "w");

  getcwd(cwd, 200);

  argv[0] = (char*)"make";

  glob_t makefile_list;
  glob(".../makefile", 0, 0, &makefile_list);

  for (i = 0; i<makefile_list.gl_pathc; i++)
  {
    char *mf = makefile_list.gl_pathv[i];
    strcpy(path, mf);
    char *lsl = strrchr(path, '/');
    if (lsl) *lsl = 0;

    if (strcmp(mf, "makefile") == 0)
      continue;

    printf("--------------------------------------- Making in %s\n", path);

    sprintf(file, "%s/%s", cwd, path);
    if (chdir(file))
    {
      printf("Cannot chdir to %s\n", file);
      continue;
    }
    if (spawnvp(P_WAIT, "make", argv))
      exit(1);

    FILE *oh = fopen("makefile.oh", "r");
    if (oh)
    {
      int last_was_nl = 1;
      int ch;

      while ((ch = fgetc(oh)) != EOF)
      {
	if (ch != '\n' && last_was_nl)
	  fprintf(oi, "OBJS += ");
	last_was_nl = (ch == '\n');
	if (ch == '&')
	{
	  fprintf(oi, "%s", path);
	  fprintf(rf, "%s", path);
	}
	else
	{
	  fputc(ch, oi);
	  fputc(ch, rf);
	}
      }
      fclose(oh);
    }
  }
  fclose(rf);
  fclose(oi);

  chdir(cwd);

  spawnlp(P_WAIT, "update", "update", "makefile.rf2", "makefile.rf", 0);
  remove("makefile.rf2");

  if (spawnvp(P_WAIT, "make", argv))
    exit(1);

  return 0;
}
