#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static void
die(const char *m)
{
  printf("Error: %s\n", m);
  exit(1);
}

int
main(void)
{
  int fid;
  char buf[512];
  struct stat st;

  fid = open("trunc.dat", O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (!fid)
    die("open o_creat");
  write(fid, buf, 512);
  close(fid);

  if (stat("trunc.dat", &st))
    die("stat");
  if (st.st_size == 0)
    die("wrong size");

  fid = open("trunc.dat", O_WRONLY | O_TRUNC, 0666);
  if (!fid)
    die("open o_trunc");
  close(fid);

  if (stat("trunc.dat", &st))
    die("second stat");
  if (st.st_size != 0)
    die("wrong size");

  remove("trunc.dat");

  return 0;
}
