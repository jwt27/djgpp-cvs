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
#if defined(F_SETLK) && defined(F_SETLKW)
  struct flock fl;
  struct flock64 fl64;
  int retval, fd;

  fd = open("tfcntl.c", O_RDONLY);
  fl.l_type = F_RDLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = fl.l_len = 0;

  fl64.l_type = F_RDLCK;
  fl64.l_whence = SEEK_SET;
  fl64.l_start = fl64.l_len = 0L;

  errno = 0;
  retval = fcntl(fd, F_SETLK, &fl);
  printf("F_SETLK/F_RDLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  retval = fcntl(fd, F_SETLK, &fl);
  printf("F_SETLK/F_RDLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  retval = fcntl(fd, F_GETLK, &fl);
  printf("F_GETLK/F_RDLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  fl.l_type = F_UNLCK;
  retval = fcntl(fd, F_SETLK, &fl);
  printf("F_SETLK/F_UNLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  fl.l_type = F_UNLCK;
  retval = fcntl(fd, F_SETLK, &fl);
  printf("F_SETLK/F_UNLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  fl.l_type = F_RDLCK;
  retval = fcntl(fd, F_GETLK, &fl);
  printf("F_GETLK/F_RDLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  /* F_SETLKW tests */

  errno = 0;
  fl.l_type = F_RDLCK;
  retval = fcntl(fd, F_SETLKW, &fl);
  printf("F_SETLKW/F_RDLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  /* This call will lock the program in an endless loop,
     since it already has the lock.

     FIXME -- Need to add SIGINT interrupt handler and
              console output (syserr?) telling user to
              press CNTL-Break to continue the test.

     Commented out for now, replaced with message about bypass.

  errno = 0;
  retval = fcntl(fd, F_SETLKW, &fl);
  printf("F_SETLKW/F_RDLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  */

  printf("F_SETLKW/F_RDLCK:Bypassed to avoid lockup in endless loop.\n");

  errno = 0;
  retval = fcntl(fd, F_GETLK, &fl);
  printf("F_GETLK/F_RDLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  fl.l_type = F_UNLCK;
  retval = fcntl(fd, F_SETLK, &fl);
  printf("F_SETLK/F_UNLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  fl.l_type = F_RDLCK;
  retval = fcntl(fd, F_GETLK, &fl);
  printf("F_GETLK/F_RDLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  /* FAT32 tests */

  errno = 0;
  fl64.l_type = F_RDLCK;
  retval = fcntl(fd, F_SETLK64, &fl64);
  printf("F_SETLK64/F_RDLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  retval = fcntl(fd, F_SETLK64, &fl64);
  printf("F_SETLK64/F_RDLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  retval = fcntl(fd, F_GETLK64, &fl64);
  printf("F_GETLK64/F_RDLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  fl64.l_type = F_UNLCK;
  retval = fcntl(fd, F_SETLK64, &fl64);
  printf("F_SETLK64/F_UNLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  fl64.l_type = F_RDLCK;
  retval = fcntl(fd, F_GETLK64, &fl64);
  printf("F_GETLK64/F_RDLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  /* F_SETLKW64 tests */

  errno = 0;
  fl64.l_type = F_RDLCK;
  retval = fcntl(fd, F_SETLKW64, &fl64);
  printf("F_SETLKW64/F_RDLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  /* This call will lock the program in an endless loop,
     since it already has the lock.

     FIXME -- Need to add SIGINT interrupt handler and
              console output (syserr?) telling user to
              press CNTL-Break to continue the test.

     Commented out for now, replaced with message about bypass.

  errno = 0;
  retval = fcntl(fd, F_SETLKW64, &fl64);
  printf("F_SETLKW64/F_RDLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  */

  printf("F_SETLKW64/F_RDLCK:Bypassed to avoid lockup in endless loop.\n");

  errno = 0;
  retval = fcntl(fd, F_GETLK64, &fl64);
  printf("F_GETLK64/F_RDLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  fl64.l_type = F_UNLCK;
  retval = fcntl(fd, F_SETLK64, &fl64);
  printf("F_SETLK64/F_UNLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  fl64.l_type = F_RDLCK;
  retval = fcntl(fd, F_GETLK64, &fl64);
  printf("F_GETLK64/F_RDLCK:retval=%d\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  /* Other function tests (non-lock-related) */

  errno = 0;
  retval = fcntl(fd, F_GETFD);
  printf("F_GETFD:retval=%04x\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  retval = fcntl(fd, F_SETFL, O_BINARY);
  printf("F_SETFL:retval=%04x\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  retval = fcntl(fd, F_GETFL);
  printf("F_GETFL:retval=%04x\n", retval);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  close(fd);
  exit(0);
#else
  exit(99);
#endif
}
