#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

int
main (int argc, char *argv[])
{
  const char regular_file[] = "tremove.reg";
  const char symlink_file[] = "tremove.sym";
  FILE *fp;

  /* Create the regular file */
  fp = fopen(regular_file, "wt");
  if (!fp)
    {
      printf("ERROR: Unable to open regular file '%s'!\n", regular_file);
      return(EXIT_FAILURE);
    }
  fclose(fp);

  /* Create the symlink to the regular file */
  if (symlink(regular_file, symlink_file) < 0)
    {
      printf("ERROR: Unable to create symlink '%s'!\n", symlink_file);
      perror("ERROR");
      return(EXIT_FAILURE);
    }

  /* Try to remove the symlink; check it's gone and that the regular file
   * is still there. */
  if (remove(symlink_file))
    {
      printf("ERROR: Unable to remove symlink '%s'!\n", symlink_file);
      return(EXIT_FAILURE);
    }

  {
    struct stat sbuf;

    if ((lstat(symlink_file, &sbuf) < 0) && (errno != ENOENT))
      {
	printf("ERROR: Couldn't lstat '%s'!\n", symlink_file);
	perror("ERROR");
	return(EXIT_FAILURE);
      }
  }

  /* Try to remove the regular file; check it's gone. */
  if (remove(regular_file))
    {
      printf("ERROR: Unable to remove regular file '%s'!\n", regular_file);
      return(EXIT_FAILURE);
    }

  if (!access(regular_file, R_OK))
    {
      printf("ERROR: Regular file '%s' still exists!\n", regular_file);
      return(EXIT_FAILURE);
    }

  puts("PASS");
  return(EXIT_SUCCESS);
}
