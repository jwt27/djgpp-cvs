/*
**  Usage: compare oldlib.armap newlib.armap
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "slist.h"

void
loadsyms(char *fn, StringList &sl)
{
  FILE *f;
  char line[1000];
  char sym[1000];
  f = fopen(fn, "r");
  if (f == 0)
  {
    perror(fn);
    exit(1);
  }
  fgets(line, 1000, f);
  fgets(line, 1000, f);
  while (fgets(line, 1000, f))
  {
    if (line[0] <= ' ')
      break;
    sscanf(line, "%s", sym);
    sl.add(sym);
  }
  fclose(f);
  sl.sort();
}

int
main(int argc, char **argv)
{
  StringList old_objs;
  StringList new_objs;
  int i, col=80, need_banner=1;

  loadsyms(argv[1], old_objs);
  loadsyms(argv[2], new_objs);

  for (i=0; i<old_objs.count; i++)
  {
    char *s = old_objs[i];
    if (s[0] != '_')
      continue;
    if (!new_objs.has(s))
    {
      if (need_banner)
      {
	need_banner = 0;
	printf("Symbols in old library, but not in new one:");
      }
      col += strlen(s) + 1;
      if (col > 77)
      {
	col = strlen(s)-1;
	putchar('\n');
      }
      else
	fputs("  ", stdout);
      fputs(s+1, stdout);
    }
  }
  if (!need_banner)
    putchar('\n');
  return 0;
}
