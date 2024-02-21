#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int
main (int argc, char *argv[])
{
  int i, n, last_fd;

  if (argc != 2)
    {
      fprintf (stderr, "Usage: %s #files\n", argv[0]);
      exit (-1);
    }

  n = atoi (argv[1]);

  last_fd = -1;
  for (i = 0; i < n; i++)
    {
      char name[256];
      int fd;

      sprintf (name, "/dev/env/DJDIR/tmp/foo%d", i);
      if ((fd = open (name, O_CREAT, 0777)) < 0)
	{
	  fprintf (stderr, "open number %d failed, last fd = %d.\n", i,
		   last_fd);
	  perror ("");
	  exit (-2);
	}
      last_fd = fd;
    }

  return 0;
}

