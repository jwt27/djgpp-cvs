/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* 1998 -- Modified by Nils van den Heuvel (n.heuvel@wxs.nl) */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
  int f;
  FILE *of;
  unsigned char buf[4096];
  int rbytes;
  int col=0, i;
  if (argc < 4)
  {
    printf("usage: %s infile symname outfile\n", basename(argv[0]));
    exit(1);
  }
  f = open(argv[1], O_RDONLY|O_BINARY);
  if (f < 1)
  {
    printf("%s: ",basename(argv[0]));
    perror(argv[1]);
    exit(1);
  }

  of = fopen(argv[3], "w");

  if (of == NULL)
  {
    printf("%s: ",basename(argv[0]));
    perror(argv[3]);
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
