/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
  int f = open(argv[1], O_RDONLY|O_BINARY);
  FILE *of = fopen(argv[3], "w");
  unsigned char buf[4096];
  int rbytes;
  int col=0, i;
  if (argc < 4)
  {
    printf("usage: bin2byte infile symname outfile\n");
    exit(1);
  }
  fprintf(of, "unsigned char %s[] = {\n", argv[2]);
  while ((rbytes = read(f, buf, 4096)) > 0)
  {
    for (i=0; i<rbytes; i++)
    {
      fprintf(of, "%d,", buf[i]);
      if (col++ == 32)
      {
        fprintf(of, "\n");
        col = 0;
      }
    }
  }
  fprintf(of, "};\n");
  fclose(of);
  return 0;
}
