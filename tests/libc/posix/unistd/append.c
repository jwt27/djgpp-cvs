#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#define FILE_NAME "append.dat"
char str[] = "hello, there\n";

int
main(void)
{
  char in = 0;
  int fd;
  int status = 0; /* Return value. */
  struct stat s;
  size_t len;

  fd = open( FILE_NAME, O_WRONLY|O_CREAT|O_TRUNC );
  write( fd, str, strlen(str) );
  close( fd );
  stat( FILE_NAME, &s );
  len = s.st_size;

  fd = open( FILE_NAME, O_APPEND|O_WRONLY);
  write( fd, str, strlen(str) );
  close( fd );
  stat( FILE_NAME, &s );
  if (s.st_size != len * 2)
  {
    printf("wrong size 1!\n");
    status++;
  }

  fd = open( FILE_NAME, O_APPEND|O_RDWR );
  lseek( fd, 1, SEEK_SET );
  read( fd, &in, 1 );
  if( in != str[1] )
  {
    printf( "Wrong character found: '%c', expected '%c'!\n", in, str[1] );
    status++;
  }
  write( fd, str, strlen(str) );
  close( fd );
  stat( FILE_NAME, &s );
  if( s.st_size != len * 3 )
  {
    printf("wrong size 2!\n");
    status++;
  }

  return status;
}
