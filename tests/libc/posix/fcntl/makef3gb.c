#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <dos.h>
#include <libc/dosio.h>
#include <limits.h>

#define BIGBUFSIZE 500*1024

void prdoserr(int fd) {
  struct DOSERROR de;
  struct DOSERROR_STR se;
  int errsave;

  printf("fd: %i\n", fd);
  printf("errno: %i : %s\n", errno, strerror(errno));
  dosexterr(&de);
  errsave = de.exterror;
  de.exterror = _doserrno;
  dostrerr(&de, &se);
  printf("DOS errno: %i : %s\n", _doserrno, se.exterror_str);
  de.exterror = errsave;
  dostrerr(&de, &se);
  printf("DOS extended error to errno: %i : %s\n",
    __doserr_to_errno(de.exterror),strerror(__doserr_to_errno(de.exterror)));
  printf("Extended DOS error information:\n");
  printf("Extended error: %i : %s\n",de.exterror,se.exterror_str);
  printf("Class:          %02X : %s\n",de.class,se.class_str);
  printf("Action:         %02X : %s\n",de.action,se.action_str);
  printf("Error Locus:    %02X : %s\n",de.locus,se.locus_str);
}


int main(void) {
  int retval, i;
  long long int flen;
  FILE *f3GB;
  char bigbuf[BIGBUFSIZE];

  f3GB = fopen("fcntl3GB.dat", "w");

  for (i = 0; i <= BIGBUFSIZE; i++) bigbuf[i] = (char) i;

  for (flen = 0; flen < (long long)(3LL * (INT_MAX/2LL)); flen += BIGBUFSIZE)
  {
    retval = fwrite(bigbuf, sizeof(char), BIGBUFSIZE, f3GB);
    if (retval != BIGBUFSIZE) {
      printf("\nError writing 3GB file, fwrite returned %d!\n", retval);
      prdoserr(fileno(f3GB));
      fclose(f3GB);
      exit(1);
    }
    printf("Writing 3GB file, so far wrote %lld\r", flen);
  }

  printf("\nWrote 3GB file OK!\n");
  fclose(f3GB);

  return 0;
}
