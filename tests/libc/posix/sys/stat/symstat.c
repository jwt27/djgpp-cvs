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
   tmp_file = tempnam("./", "sl");
   symlink("file1", tmp_file);
   if (stat(tmp_file, &file_info))
   {
      perror("Test 1 failed - unexpected stat() failure ");
      exit(1);
   }
   if (file_info.st_size != 10)
   {
      fprintf(stderr, "Test 1 failed - stat() returns wrong file size\n");
      exit(1);
   }
   if (!S_ISREG(file_info.st_mode))
   {
      fprintf(stderr, "Test 1 failed - stat() does not set regular file bit "
                      "for mode\n");
      exit(1);
   }
   if (file_info.st_mode & !S_IFREG)
   {
      fprintf(stderr, "Test 1 failed - stat() sets other mode bits for file\n");
      exit(1);
   }
   printf("Test 1 passed\n");
   if (stat("file1", &file_info))
   {
      fprintf(stderr, "Test 2 failed - unexpected stat() failure\n");
      exit(1);
   }
   if (file_info.st_size != 10)
   {
      fprintf(stderr, "Test 2 failed - stat() returns wrong file size\n");
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
