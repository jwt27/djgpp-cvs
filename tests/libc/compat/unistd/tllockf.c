#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>
#include <dos.h>
#include <libc/dosio.h>

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
  int retval, fd;

  fd = open("tllockf.c", O_RDONLY);

  errno = 0;
  retval = llockf(fd, F_TEST, 0);
  printf("F_TEST:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  retval = llockf(fd, F_TLOCK, 0);
  printf("F_TLOCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  retval = llockf(fd, F_TLOCK, 0);
  printf("F_TLOCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  retval = llockf(fd, F_TEST, 0);
  printf("F_TEST:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  retval = llockf(fd, F_ULOCK, 0);
  printf("F_ULOCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  retval = llockf(fd, F_ULOCK, 0);
  printf("F_ULOCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  retval = llockf(fd, F_TEST, 0);
  printf("F_TEST:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  retval = llockf(fd, F_LOCK, 0);
  printf("F_LOCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  /* This call will lock the program in an endless loop,
     since it already has the lock.

     FIXME -- Need to add SIGINT interrupt handler and
              console output (syserr?) telling user to
              press CNTL-Break to continue the test.

     Commented out for now, replaced with message about bypass.

  errno = 0;
  retval = llockf(fd, F_LOCK, 0);
  printf("F_LOCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  */

  printf("F_LOCK:Bypassed to avoid lockup in endless loop.\n");

  errno = 0;
  retval = llockf(fd, F_TEST, 0);
  printf("F_TEST:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  retval = llockf(fd, F_ULOCK, 0);
  printf("F_ULOCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  retval = llockf(fd, F_TEST, 0);
  printf("F_TEST:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  close(fd);
  exit(0);
}
