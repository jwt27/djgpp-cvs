/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <string.h>
#include <dpmi.h>
#include <go32.h>
#include <libc/farptrgs.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/vfs.h>
#include <libc/dosio.h>

int
statfs(const char *path, struct statfs *buf)
{
  __dpmi_regs regs;
  int drive_number;
  int cdrom_calls_used = 0;
  int blocks = 0;

  /* Get the drive number, including the case of magic
     names like /dev/c/foo.  */
  _put_path(path);
  drive_number = (_farpeekb(_dos_ds, __tb) & 0x1f) - 1;
  if (_farpeekb(_dos_ds, __tb + 1) != ':' || drive_number == -1)
  {
    regs.h.ah = 0x19;
    __dpmi_int(0x21, &regs);
    drive_number = regs.h.al;
  }

  /* For a CD-ROM drive, 213600 gives incorrect info.
     Use CD-ROM-specific calls if they are available.

     Int 2Fh/AX=1510h gives us a way of doing IOCTL with the
     CD-ROM device driver without knowing the name of the
     device (which is defined by the CONFIG.SYS line that
     installs the driver and can therefore be arbitrary).  */

  regs.x.ax = 0x150b;	/* is this drive supported by CD-ROM driver? */
  regs.x.cx = drive_number;
  __dpmi_int(0x2f, &regs);
  if ((regs.x.flags & 1) == 0 && regs.x.bx == 0xadad && regs.x.ax != 0)
  {
    unsigned char request_header[0x14];
    int status, i = 2, bsize = 0;

    /* Construct the request header for the CD-ROM device driver.  */
    memset (request_header, 0, sizeof request_header);
    request_header[0] = sizeof request_header;
    request_header[2] = 3;	/* IOCTL READ command */
    *(unsigned short *)&request_header[0xe]  = __tb_offset;
    *(unsigned short *)&request_header[0x10] = __tb_segment;
    request_header[0x12] = 4;	/* number of bytes to transfer */

    /* When the disk was just changed, we need to try twice.  */
    do {
      /* Put control block into the transfer buffer.  */
      _farpokeb (_dos_ds, __tb, 7);	/* read sector size */
      _farpokeb (_dos_ds, __tb + 1, 0);	/* cooked mode */
      _farpokew (_dos_ds, __tb + 2, 0);	/* zero out the result field */

      /* Put request header into the transfer buffer and call the driver.  */
      dosmemput (request_header, sizeof (request_header), __tb + 4);
      regs.x.ax = 0x1510;
      regs.x.cx = drive_number;
      regs.x.es = __tb_segment;
      regs.x.bx = __tb_offset + 4;
      __dpmi_int (0x2f, &regs);
      status = _farpeekw (_dos_ds, __tb + 7);
      bsize  = _farpeekw (_dos_ds, __tb + 2);
    } while (--i && (status & 0x800f) == 0x800f); /* disk changed */

    if (status == 0x100 && _farpeekw (_dos_ds, __tb + 4 + 0x12) == 4)
    {
      request_header[0x12] = 5;	/* number of bytes to transfer */
      /* Put control block into the transfer buffer.  */
      _farpokeb (_dos_ds, __tb, 8);	/* read volume size */
      _farpokel (_dos_ds, __tb + 1, 0);	/* zero out the result field */

      /* Put request header into the transfer buffer and call the driver.  */
      dosmemput (request_header, sizeof (request_header), __tb + 5);
      regs.x.ax = 0x1510;
      regs.x.cx = drive_number;
      regs.x.es = __tb_segment;
      regs.x.bx = __tb_offset + 5;
      __dpmi_int (0x2f, &regs);
      if (_farpeekw (_dos_ds, __tb + 8) == 0x100
	  && _farpeekw (_dos_ds, __tb + 5 + 0x12) == 5)
      {
	regs.x.ax = 1;		/* fake: sectors per cluster */
	regs.x.cx = bsize;
	regs.x.bx = 0;		/* no free space: cannot add data to CD-ROM */
	blocks  = _farpeekl (_dos_ds, __tb + 1);
	cdrom_calls_used = 1;
      }
    }
  }

  if (!cdrom_calls_used)
  {
    /* Get free space info from DOS.  */
    regs.h.ah = 0x36;		/* DOS Get Free Disk Space call */
    regs.h.dl = drive_number + 1;
    __dpmi_int(0x21, &regs);

    /* Check for errors */
    if ((regs.x.ax & 0xffff) == 0xffff)
    {
      errno = ENODEV;
      return -1;
    }
    blocks = regs.x.dx;
  }

  /* Fill in the structure */
  buf->f_bavail = regs.x.bx;
  buf->f_bfree = regs.x.bx;
  buf->f_blocks = blocks;
  buf->f_bsize = regs.x.cx * regs.x.ax;
  buf->f_ffree = regs.x.bx;
  buf->f_files = blocks;
  buf->f_type = 0;
  buf->f_fsid[0] = drive_number;
  buf->f_fsid[1] = MOUNT_UFS;
  buf->f_magic = FS_MAGIC;

  return 0;
}

#ifdef TEST

#include <stdio.h>
#include <errno.h>

int main (int argc, char *argv[])
{
  char *path = ".";
  struct statfs fsbuf;

  if (argc > 1)
    path = argv[1];
  errno = 0;

  if (statfs (path, &fsbuf) == 0)
    printf ("Results for `%s\':\n\nTotal blocks: %ld\nAvailable blocks: %ld\nBlock size: %ld\n",
	    path, fsbuf.f_blocks, fsbuf.f_bfree, fsbuf.f_bsize);
  if (errno)
    perror (path);
  return 0;
}

#endif
