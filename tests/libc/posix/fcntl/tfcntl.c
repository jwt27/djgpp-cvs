#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <dos.h>
#include <libc/dosio.h>

static void prdoserr(int fd) {
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

static const char *flags2str(const int flags) {
  static char buf[256];
  int need_comma = 0;

  buf[0] = '\0';

  switch(flags & O_ACCMODE) {
  case O_RDONLY: strcat(buf, "O_RDONLY"); need_comma++; break;
  case O_WRONLY: strcat(buf, "O_WRONLY"); need_comma++; break;
  case O_RDWR:   strcat(buf, "O_RDWR");   need_comma++; break;

  default:
    break;
  }

  if (flags & O_APPEND) {
    if (need_comma) {
      strcat(buf, ", ");
      need_comma--;
    }

    strcat(buf, "O_APPEND");
    need_comma++;
  }

  if (flags & O_NONBLOCK) {
    if (need_comma) {
      strcat(buf, ", ");
      need_comma--;
    }

    strcat(buf, "O_NONBLOCK");
    need_comma++;
  }

  return(buf);
}

int main(void) {
  int retval, fd;

#if defined(F_SETLK) && defined(F_SETLKW)
  struct flock fl;
  struct flock64 fl64;
#endif /* F_SETLK && F_SETLKW */

  fd = open("tfcntl.c", O_RDONLY);
  if (fd < 0) {
    perror("tfcntl");
    return(EXIT_FAILURE);
  }

#if defined(F_SETLK) && defined(F_SETLKW)
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
  puts("--- F_SETLKW tests ---");

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
  puts("--- FAT32 tests ---");

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
  puts("--- F_SETLKW64 tests ---");

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
#endif /* F_SETLK && F_SETLKW */

  /* Other function tests (non-lock-related) */
  puts("--- Other tests ---");

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
  printf("F_GETFL:retval=%04x = %s\n", retval, flags2str(retval));
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  close(fd);

  /* F_GETFL tests */
  puts("--- F_GETFL tests ---");

  /* Open a file for read-only */
  fd = open("tfcntl.c", O_RDONLY);
  if (fd < 0) {
    perror("tfcntl");
    return(EXIT_FAILURE);
  }

  errno = 0;
  retval = fcntl(fd, F_GETFL);
  printf("F_GETFL:retval=%04x = %s\n", retval, flags2str(retval));
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  close(fd);

  /* Open a file for write-only */
  remove("tfcntl.wo");
  fd = open("tfcntl.wo", O_CREAT|O_WRONLY);
  if (fd < 0) {
    perror("tfcntl");
    return(EXIT_FAILURE);
  }

  errno = 0;
  retval = fcntl(fd, F_GETFL);
  printf("F_GETFL:retval=%04x = %s\n", retval, flags2str(retval));
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  close(fd);

  /* Try opening a file for write-only using creat(). */
  remove("tfcntl.wo");
  fd = creat("tfcntl.wo", S_IWUSR);
  if (fd < 0) {
    perror("tfcntl");
    return(EXIT_FAILURE);
  }

  errno = 0;
  retval = fcntl(fd, F_GETFL);
  printf("F_GETFL:retval=%04x = %s\n", retval, flags2str(retval));
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  close(fd);

  /* Open a file for read-write, append */
  remove("tfcntl.rwa");
  fd = open("tfcntl.rwa", O_CREAT|O_RDWR|O_APPEND);
  if (fd < 0) {
    perror("tfcntl");
    return(EXIT_FAILURE);
  }

  errno = 0;
  retval = fcntl(fd, F_GETFL);
  printf("F_GETFL:retval=%04x = %s\n", retval, flags2str(retval));
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  close(fd);

  return(EXIT_SUCCESS);
}
