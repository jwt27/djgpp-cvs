/* Check the built in globbing (ie test *.c) and creation/accessed times */

#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char **argv)
{
  if(argc == 1) {
    printf("Usage: lfn [file list]\n");
    printf("       the file list may contain wildcards\n");
    exit(1);
  }
  argv++;
  printf("Name                      Created Date      Modified Date     Accessed Date\n");
  for(;*argv;argv++) {
    struct stat stat_buf;
    if(!stat(*argv, &stat_buf)) {
      struct tm *tm1;
      char ct[30],mt[30],at[30];
      tm1 = localtime(&stat_buf.st_ctime);
      sprintf(ct, "%02d/%02d/%2d %02d:%02d:%02d", tm1->tm_mon, tm1->tm_mday,
        tm1->tm_year, tm1->tm_hour, tm1->tm_min, tm1->tm_sec);
      tm1 = localtime(&stat_buf.st_mtime);
      sprintf(mt, "%02d/%02d/%2d %02d:%02d:%02d", tm1->tm_mon, tm1->tm_mday,
        tm1->tm_year, tm1->tm_hour, tm1->tm_min, tm1->tm_sec);
      tm1 = localtime(&stat_buf.st_atime);
      sprintf(at, "%02d/%02d/%2d %02d:%02d:%02d", tm1->tm_mon, tm1->tm_mday,
        tm1->tm_year, tm1->tm_hour, tm1->tm_min, tm1->tm_sec);
      printf("%-25s %s %s %s\n",*argv, ct, mt, at);
    } else
      printf("stat failed on %s\n",*argv);
  }
  return 0;
}
