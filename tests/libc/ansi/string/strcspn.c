#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int nfail = 0;
static int nfiles = 0;

char *
readline (FILE *fp)
{
  static char *line = NULL;
  static size_t line_size = 0;
  size_t nread = 0;
  int c;
  char *lp = line;

  while ((c = getc (fp)) != EOF)
    {
      if (++nread >= line_size)	/* >= : we need one extra place for '\0' */
	{
	  line_size += 256;
	  line = realloc (line, line_size);
	  if (line == NULL)
	    {
	      perror ("readline");
	      exit (1);
	    }
	  lp = line + nread - 1;
	}

      *lp++ = c;
      if (c == '\n')
	break;
    }

  *lp = '\0';

  return (c == EOF ? NULL : line);
}

void test_strcpsn (int lno, const char *file, const char *string)
{
  if (!string || !*string)
    printf ("%d: file %s, line %d: EMPTY\n", ++nfail, file, lno);
  else if (string[strcspn (string, "\n")] == '\0')
    printf ("%d: file %s, line %d: NO NEWLINE\n", ++nfail, file, lno);
}

void test_file (const char *file_name)
{
  FILE *fp = fopen (file_name, "r");
  int lineno = 0;
  char *s;

  if (!fp)
    {
      perror (file_name);
      return;
    }

  nfiles++;

  while ((s = readline (fp)) != NULL)
    test_strcpsn (++lineno, file_name, s);

  if (fclose (fp))
    perror (file_name);
}

int main (int argc, char *argv[])
{
  int i;

  if (argc < 2)
    {
      printf ("Usage %s file ...\n", argv[0]);
      return 0;
    }

  for (i = 1; i < argc; i++)
    test_file (argv[i]);

  if (nfail)
    printf ("%d lines failed :-(\n", nfail);
  else if (nfiles)
    printf ("SUCCESS!! (%d files read)\n", nfiles);

  return 0;
}
