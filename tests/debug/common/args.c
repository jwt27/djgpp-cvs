#include <stdio.h>
#include <stdlib.h>
#include <crt0.h>

int _crt0_startup_flags =
  _CRT0_FLAG_PRESERVE_UPPER_CASE
  |_CRT0_FLAG_USE_DOS_SLASHES;

void
main(int argc, char **argv)
{
  int i;

  FILE *f = fopen("nul", "r");
  fclose(f);

  printf("program: `%s'\n", argv[0]);
  printf("   args:");
  for (i=1; i<argc; i++)
    printf(" %s", argv[i]);
  printf("\n   $FOO: `%s'\n", getenv("FOO"));
  exit(0);
}
