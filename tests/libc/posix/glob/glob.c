#include <stdio.h>
#include <stdlib.h>
#include <glob.h>

char **
__ctr0_glob_function(void) {
  return 0;
}

void
usage(void)
{
  fprintf(stderr, "Usage: glob -m -c -e -s 'pattern'\n");
  exit(1);
}

int
main(int argc, char **argv)
{
  glob_t g;
  int rv, i;
  int flags = 0;

  g.gl_pathc = 0;
  g.gl_pathv = 0;
  g.gl_offs = 0;

  while (argc > 1 && argv[1][0] == '-')
    {
      switch (argv[1][1]) {
      case 'm':
	flags |= GLOB_MARK;
	break;
      case 'c':
	flags |= GLOB_NOCHECK;
	break;
      case 'e':
	flags |= GLOB_NOESCAPE;
	break;
      case 's':
	flags |= GLOB_NOSORT;
	break;
      default:
	usage();
      }
      argc--;
      argv++;
    }
  if (argc < 2)
    usage();

  printf("Pattern string: ``%s''\n", argv[1]);
  rv = glob(argv[1], flags, 0, &g);

  switch (rv)
    {
    case GLOB_ABORTED:
      printf("rv = GLOB_ABORTED\n");
      break;
    case GLOB_NOMATCH:
      printf("rv = GLOB_NOMATCH\n");
      break;
    case GLOB_NOSPACE:
      printf("rv = GLOB_NOSPACE\n");
      break;
    default:
      printf("rv = %d\n", rv);
      break;
    }

  for (i=0; i<g.gl_pathc; i++)
    printf("[%d] ``%s''\n", i, g.gl_pathv[i]);

  return 0;
}
