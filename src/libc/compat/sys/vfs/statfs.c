/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dpmi.h>
#include <errno.h>
#include <sys/vfs.h>
#include <ctype.h>

int
statfs(const char *path, struct statfs *buf)
{
  __dpmi_regs regs;
  int drive_number;

  /* Get the drive number */
  if (isalpha(path[0]) && path[1] == ':')
    drive_number = (path[0] & 0x1f) - 1;
  else
  {
    regs.h.ah = 0x19;
    __dpmi_int(0x21, &regs);
    drive_number = regs.h.al;
  }

  /* Get free space info */
  regs.h.ah = 0x36;		/* DOS Get Free Disk Space call */
  regs.h.dl = drive_number + 1;
  __dpmi_int(0x21, &regs);

  /* Check for errors */
  if ((regs.x.ax & 0xffff) == 0xffff)
  {
    errno = ENODEV;
    return -1;
  }

  /* Fill in the structure */
  buf->f_bavail = regs.x.bx;
  buf->f_bfree = regs.x.bx;
  buf->f_blocks = regs.x.dx;
  buf->f_bsize = regs.x.cx * regs.x.ax;
  buf->f_ffree = regs.x.bx;
  buf->f_files = regs.x.dx;
  buf->f_type = 0;
  buf->f_fsid[0] = drive_number;
  buf->f_fsid[1] = MOUNT_UFS;
  buf->f_magic = FS_MAGIC;

  return 0;
}
