#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int main(void)
{
   char *tmp_file;
   struct stat file_info;
   if (!__file_exists("file1"))
   {
      fprintf(stderr, "Required data file not found\n");
      exit(1);
   }
   tmp_file = tmpnam(NULL);
   symlink("file1", tmp_file);
   if (lstat(tmp_file, &file_info))
   {
      fprintf(stderr, "Test 1 failed - unexpected lstat() failure\n");
      exit(1);
   }
   if (file_info.st_size != 510)
   {
      fprintf(stderr, "Test 1 failed - lstat() returns wrong link size\n");
      exit(1);
   }
   if (!S_ISLNK(file_info.st_mode))
   {
      fprintf(stderr, "Test 1 failed - lstat() does not set link bit for mode\n");
      exit(1);
   }
   if (file_info.st_mode & !S_IFLNK)
   {
      fprintf(stderr, "Test 1 failed - lstat() set other mode bits for link\n");
      exit(1);
   }
   printf("Test 1 passed\n");
   if (lstat("file1", &file_info))
   {
      fprintf(stderr, "Test 2 failed - unexpected lstat() failure\n");
      exit(1);
   }
   if (file_info.st_size != 10)
   {
      fprintf(stderr, "Test 2 failed - lstat() returns wrong file size\n");
      exit(1);
   }
   if (!S_ISREG(file_info.st_mode))
   {
      fprintf(stderr, "Test 2 failed - lstat() does not set regular file bit "
                      "for mode\n");
      exit(1);
   }
   if (file_info.st_mode & !S_IFREG)
   {
      fprintf(stderr, "Test 2 failed - lstat() sets other mode bits for file\n");
      exit(1);
   }
   printf("Test 2 passed\n");
   remove(tmp_file);
   return 0;
}
