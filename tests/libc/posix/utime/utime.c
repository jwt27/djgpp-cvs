#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>

void
die(const char *m)
{
  printf("Error: %s\n", m);
  exit(1);
}

int
main(void)
{
  time_t now;
  struct stat s_before, s_after;
  int fid;
  struct utimbuf tb;

  time(&now);
  now &= ~1; /* DOS doesn't record seconds' LSB */

  sleep(4);

  fid = open("utime.dat", O_WRONLY|O_CREAT|O_TRUNC, 0666);
  if (fid < 0)
    die("cannot create utime.dat");
  close(fid);

  stat("utime.dat", &s_before);
  printf("created file %d seconds old\n", s_before.st_mtime - now);

  tb.actime = tb.modtime = now + 100;
  if (utime("utime.dat", &tb))
    die("cannot set utime");

  stat("utime.dat", &s_after);
  printf("utime'd file %d seconds old\n", s_after.st_mtime - now);

  remove("utime.dat");

  return 0;
}
