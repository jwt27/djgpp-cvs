/* stat() vs fstat() */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SFMT	"%-10s %-10s %-10s\n"
#define DFMT	"%-10s %10d %10d\n"
#define XFMT	"%-10s 0x%08x 0x%08x\n"
#define OFMT	"%-10s 0%09o 0%09o\n"

void
try(const char *f)
{
  struct stat ss;
  struct stat fs;
  int o;

  o = open(f, O_RDONLY);
  if (o < 0) return;

  stat(f, &ss);
  fstat(o, &fs);

/*  close(o); */

  printf("\nFile: %s\n", f);
  printf(SFMT, "field", "stat", "fstat");
  printf(DFMT, "st_ino", ss.st_ino, fs.st_ino);
  printf(OFMT, "st_mode", ss.st_mode, fs.st_mode);
  printf(DFMT, "st_mtime", ss.st_mtime, fs.st_mtime);
}

int
main(void)
{
  try("svsf.c");
  try(".");
  try("/");
  return 0;
}
