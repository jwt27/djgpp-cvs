#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FILE_NAME "fflush.dat"

#define LENGTH_OF_ZEROES (10240)

int
main(void)
{
  char ch;
  FILE *f;
  int i, ret;
  int status = 0; /* Return value. */

  f = fopen(FILE_NAME, "w");
  for( i = 0; i < LENGTH_OF_ZEROES; i++ )
  {
    fprintf(f, "%c", '\0');
  }
  fclose(f);

  f = fopen(FILE_NAME, "a");
  fprintf(f, "hello, there\n");
  /* fflush(f) tried here but no bug. */
  fclose(f);

  f = fopen(FILE_NAME, "a");
  fseek(f, 10, SEEK_SET);
  fprintf(f, "hello, there\n");
  /* fflush(f) tried here but no bug. */
  fclose(f);

  f = fopen(FILE_NAME, "r");
  for( i = 0; i < LENGTH_OF_ZEROES; i++ )
  {
    ret = fscanf(f, "%c", &ch);
    if( ret != 1 )
    {
      fprintf(stderr, "Failed to read expected 0 at index %d.\n", i);
      status++;
    }
    else if( ch != '\0' )
    {
      fprintf(stderr, "Unexpected char '%c' (%d) found while expecting 0 at index %d.\n", ch, ch, i);
      status++;
    }
  }
  fclose(f);

  return status;
}
