#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>
#include <io.h>
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


void do_fcntl(long long int this_pos) {
  struct flock fl;
  struct flock64 fl64;
  int retval, fd;
  long int flen;
  long long int flen64;
  off_t fpos;
  offset_t fpos64;

  fd = open("tfcntl2.c", O_RDONLY);

  flen = filelength(fd);
  flen64 = lfilelength(fd);

  fpos = (off_t) this_pos;
  fpos64 = (offset_t) this_pos;

  fl.l_start = fpos;
  fl.l_len = 0;
  fl.l_whence = SEEK_SET;

  fl64.l_start = fpos64;
  fl64.l_len = 0L;
  fl64.l_whence = SEEK_SET;

  /* FAT16 tests */

  fpos64 = llseek(fd, 0L, SEEK_SET);

  errno = 0;
  fl.l_type = F_RDLCK;
  retval = fcntl(fd, F_SETLK, &fl);
  printf("F_SETLK/F_RDLCK/BOF:retval=%d,fl.l_start=%d\n", retval, fl.l_start);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  fl.l_type = F_UNLCK;
  retval = fcntl(fd, F_SETLK, &fl);
  printf("F_SETLK/F_UNLCK/BOF:retval=%d,fl.l_start=%d\n", retval, fl.l_start);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  fpos64 = llseek(fd, 0L, SEEK_END);

  errno = 0;
  fl.l_type = F_RDLCK;
  retval = fcntl(fd, F_SETLK, &fl);
  printf("F_SETLK/F_RDLCK/EOF:retval=%d,fl.l_start=%d\n", retval, fl.l_start);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  fl.l_type = F_UNLCK;
  retval = fcntl(fd, F_SETLK, &fl);
  printf("F_SETLK/F_UNLCK/EOF:retval=%d,fl.l_start=%d\n", retval, fl.l_start);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  fpos64 = llseek(fd, flen * 2L, SEEK_SET);

  errno = 0;
  fl.l_type = F_RDLCK;
  retval = fcntl(fd, F_SETLK, &fl);
  printf("F_SETLK/F_RDLCK/2*EOF:retval=%d,fl.l_start=%d\n", retval, fl.l_start);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  fl.l_type = F_UNLCK;
  retval = fcntl(fd, F_SETLK, &fl);
  printf("F_SETLK/F_UNLCK/2*EOF:retval=%d,fl.l_start=%d\n", retval, fl.l_start);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  /* FAT32 tests */

  fpos64 = llseek(fd, 0L, SEEK_SET);

  errno = 0;
  fl64.l_type = F_RDLCK;
  retval = fcntl(fd, F_SETLK64, &fl64);
  printf("F_SETLK64/F_RDLCK/BOF:retval=%d,fl64.l_start=%ld\n", retval, (long)fl64.l_start);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  fl64.l_type = F_UNLCK;
  retval = fcntl(fd, F_SETLK64, &fl64);
  printf("F_SETLK64/F_UNLCK/BOF:retval=%d,fl64.l_start=%ld\n", retval, (long)fl64.l_start);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  fpos64 = llseek(fd, 0L, SEEK_END);

  errno = 0;
  fl64.l_type = F_RDLCK;
  retval = fcntl(fd, F_SETLK64, &fl64);
  printf("F_SETLK64/F_RDLCK/EOF:retval=%d,fl64.l_start=%ld\n", retval, (long)fl64.l_start);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  fl64.l_type = F_UNLCK;
  retval = fcntl(fd, F_SETLK64, &fl64);
  printf("F_SETLK64/F_UNLCK/EOF:retval=%d,fl64.l_start=%ld\n", retval, (long)fl64.l_start);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  fpos64 = llseek(fd, flen64 * 2L, SEEK_SET);

  errno = 0;
  fl64.l_type = F_RDLCK;
  retval = fcntl(fd, F_SETLK64, &fl64);
  printf("F_SETLK64/F_RDLCK/2*EOF:retval=%d,fl64.l_start=%ld\n", retval, (long)fl64.l_start);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  errno = 0;
  fl64.l_type = F_UNLCK;
  retval = fcntl(fd, F_SETLK64, &fl64);
  printf("F_SETLK64/F_UNLCK/2*EOF:retval=%d,fl64.l_start=%ld\n", retval, (long)fl64.l_start);
  if ((retval < 0) && (errno != 0)) prdoserr(fd);

  close(fd);
}


int main(void) {
#if defined(F_SETLK) && defined(F_SETLKW)
  int fd;
  long long int flen64;

  fd = open("tfcntl2.c", O_RDONLY);

  flen64 = lfilelength(fd);

  close(fd);

  do_fcntl(0LL);
  do_fcntl(flen64 / 2LL);
  do_fcntl(flen64);
  do_fcntl(flen64 * 2LL);

  return 0;

#else
  exit(99);
#endif
}
