/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
  char buf[1000];
  char fn[1000];
  char cmd[1000];
  int i;
  FILE *stubs, *as, *oh;

  /* Remove all the old files */
  for (i=0; ; i++)
  {
    sprintf(fn, "stub%04d.o", i);
    if (access(fn, F_OK))
      break;
    remove(fn);
  }
  remove("makefile.oh");
  oh = fopen("makefile.oh", "w");

  stubs = fopen(argv[1], "r");
  i = 0;
  while (fgets(buf, 1000, stubs))
  {
    if (strncmp(buf, "#define", 7))
      continue;
    sscanf(buf, "%*s %s", buf);
    if (strncmp(buf, "__dj_include", 10) == 0)
      continue;
    sprintf(fn, "stub%04d.o", i);
    i++;
    printf("\r%s = %s    ", fn, buf);

    sprintf(cmd, "as -o %s", fn);
    as = popen(cmd, "w");
    fprintf(as, ".file \"%s.stub\"; .global _%s; _%s:; jmp ___%s\n",
	    buf, buf, buf, buf);
    pclose(as);

    fprintf(oh, "&/%s\n", fn);
  }
  fclose(oh);
  fclose(stubs);
  printf("\n");

  return 0;
}
