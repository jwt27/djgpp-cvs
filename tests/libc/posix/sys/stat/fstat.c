/* Testsuite for fstat()
 * TODO: only symlink handlink aspect is tested. Other things should be
 * tested too.
 * Currently there are following tests:
 *   1. Opens simple file, positions somewhere, checks for symlinks, checks
 * file position.
 *   2. The same as above with symlinks
 */
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
   int fd = open("fstat.c", O_RDONLY);
   off_t file_pos;
   struct stat st;
   if (fd < 0)
   {
      perror("Failed to open fstat.c ");
      return 1;
   }
   lseek(fd, 65, SEEK_SET);
   file_pos = tell(fd);
   if (fstat(fd, &st))
   {
      perror("Failed to stat fstat.c ");
      return 1;
   }
   if (S_ISLNK(st.st_mode))
   {
      fprintf(stderr, "Test 1 failed - S_IFLNK bit should not be set\n");
      return 1;
   }
   if (file_pos != tell(fd))
   {
      fprintf(stderr, "Test 1 failed - file position changed\n");
      return 1;
   }
   printf("Test 1 passed\n");
   close(fd);
   symlink("fstat.c", "linkst");
   fd = open("linkst", O_RDONLY | O_NOLINK);
   if (fd < 0)
   {
      perror("Failed to open link to fstat.c ");
      return 1;
   }
   lseek(fd, 56, SEEK_SET);
   file_pos = tell(fd);
   if (fstat(fd, &st))
   {
      perror("Failed to stat link to fstat.c ");
      return 1;
   }
   if (!S_ISLNK(st.st_mode))
   {
      fprintf(stderr, "Test 2 failed - S_IFLNK bit should be set\n");
      return 1;
   }
   if (file_pos != tell(fd))
   {
      fprintf(stderr, "Test 2 failed - file position changed\n");
      return 1;
   }
   printf("Test 2 passed\n");
   close(fd);
   remove("linkst");
   return 0;
}
