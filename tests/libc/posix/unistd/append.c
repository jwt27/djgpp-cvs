#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define FILE_NAME "append.dat"
char str[] = "hello, there\n";

static void
die (const char *progname, const int line)
{
  char buf[PATH_MAX + 1 + 4 + 1]; /* progname + colon + up to 4 digits + nul */

  snprintf(buf, sizeof(buf), "%s:%d", progname, line);
  perror(buf);
  exit(EXIT_FAILURE);
}

int
main (int argc, char *argv[])
{
  char in = 0;
  int fd;
  int status = 0; /* Return value. */
  struct stat s;
  size_t len;

  fd = open( FILE_NAME, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR );
  if (fd < 0)
    die(argv[0], __LINE__);
  if ( write( fd, str, strlen(str) ) < 0 )
    die(argv[0], __LINE__);
  close( fd );
  stat( FILE_NAME, &s );
  len = s.st_size;

  fd = open( FILE_NAME, O_APPEND|O_WRONLY);
  if (fd < 0)
    die(argv[0], __LINE__);
  if ( write( fd, str, strlen(str) ) < 0 )
    die(argv[0], __LINE__);
  close( fd );
  stat( FILE_NAME, &s );
  if (s.st_size != (int)len * 2)
  {
    printf("wrong size 1!\n");
    status++;
  }

  fd = open( FILE_NAME, O_APPEND|O_RDWR );
  if (fd < 0)
    die(argv[0], __LINE__);
  lseek( fd, 1, SEEK_SET );
  read( fd, &in, 1 );
  if( in != str[1] )
  {
    printf( "Wrong character found: '%c', expected '%c'!\n", in, str[1] );
    status++;
  }
  if ( write( fd, str, strlen(str) ) < 0 )
    die(argv[0], __LINE__);
  close( fd );
  stat( FILE_NAME, &s );
  if( s.st_size != (int)len * 3 )
  {
    printf("wrong size 2!\n");
    status++;
  }

  return status;
}
