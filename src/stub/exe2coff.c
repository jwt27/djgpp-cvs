/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <io.h>
#include <unistd.h>
#include <ctype.h>


static void
exe2aout(char *fname)
{
  unsigned short header[3];
  int ifile;
  int ofile;
  char buf[4096];
  int rbytes;
  char *dot = strrchr(fname, '.');
  if (!dot || strlen(dot) != 4
      || tolower(dot[1]) != 'e'
      || tolower(dot[2]) != 'x'
      || tolower(dot[3]) != 'e')
  {
    fprintf(stderr, "%s: Arguments MUST end with a .exe extension\n", fname);
    return;
  }

  ifile = open(fname, O_RDONLY|O_BINARY);
  if (ifile < 0)
  {
    perror(fname);
    return;
  }
  read(ifile, header, sizeof(header));
  if (header[0] == 0x5a4d)
  {
    long header_offset = (long)header[2]*512L;
    if (header[1])
      header_offset += (long)header[1] - 512L;
    lseek(ifile, header_offset, 0);
    header[0] = 0;
    read(ifile, header, sizeof(header));
    if ((header[0] != 0x010b) && (header[0] != 0x014c))
    {
      fprintf(stderr, "`%s' does not have a COFF/AOUT program appended to it\n", fname);
      return;
    }
    lseek(ifile, header_offset, 0);
  }
  else
  {
    fprintf(stderr, "`%s' is not an .EXE file\n", fname);
    return;
  }
  
  *dot = 0;
  ofile = open(fname, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666);
  if (ofile < 0)
  {
    perror(fname);
    return;
  }
  
  while ((rbytes=read(ifile, buf, 4096)) > 0)
  {
    int wb = write(ofile, buf, rbytes);
    if (wb < 0)
    {
      perror(fname);
      break;
    }
    if (wb < rbytes)
    {
      fprintf(stderr, "`%s': disk full\n", fname);
      exit(1);
    }
  }
  close(ifile);
  close(ofile);
}

int
main(int argc, char **argv)
{
  int i;
  for (i=1; i<argc; i++)
    exe2aout(argv[i]);
  return 0;
}

