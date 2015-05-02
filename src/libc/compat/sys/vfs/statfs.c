/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <string.h>
#include <dpmi.h>
#include <go32.h>
#include <dos.h>
#include <libc/farptrgs.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/vfs.h>
#include <libc/dosio.h>

/* Returns: 1 == OK, successful setting of variables. 
            0 == no cdrom found, variables unchanged. */
static int 
use_AX0x1510( int drive_number, long *blocks, long *free, long *bsize )
{
  __dpmi_regs regs;

  /* For a CD-ROM drive, Int 21h/AX=3600h gives incorrect info.
     Use CD-ROM-specific calls if they are available.
     
     Int 2Fh/AX=1510h gives us a way of doing IOCTL with the
     CD-ROM device driver without knowing the name of the
     device (which is defined by the CONFIG.SYS line that
     installs the driver and can therefore be arbitrary).  */
  
  regs.x.ax = 0x150b; /* is this drive supported by CD-ROM driver? */
  regs.x.cx = drive_number;
  __dpmi_int(0x2f, &regs);
  if ((regs.x.flags & 1) == 0 && regs.x.bx == 0xadad && regs.x.ax != 0)
  {
    union {
      struct {
        unsigned int size   :16;
        unsigned int command:16;
        unsigned int offset :16;
        unsigned int segment:16;
        unsigned int bytes  :16;
      } buffer_parts;
      unsigned char buffer[0x14];
    } request_header;
    int status, i = 2;

    /* Construct the request header for the CD-ROM device driver.  */
    memset(request_header.buffer, 0, sizeof request_header.buffer);
    request_header.buffer[0] = sizeof request_header.buffer;
    request_header.buffer[2] = 3;     /* IOCTL READ command */
    request_header.buffer_parts.offset  = __tb_offset;
    request_header.buffer_parts.segment = __tb_segment;
    request_header.buffer[0x12] = 4;  /* number of bytes to transfer */

    /* When the disk was just changed, we need to try twice.  */
    do {
      /* Put control block into the transfer buffer.  */
      _farpokeb (_dos_ds, __tb, 7); /* read sector size */
      _farpokeb (_dos_ds, __tb + 1, 0); /* cooked mode */
      _farpokew (_dos_ds, __tb + 2, 0); /* zero out the result field */

      /* Put request header into the transfer buffer and call the driver.  */
      dosmemput (request_header.buffer, sizeof (request_header.buffer), __tb + 4);
      regs.x.ax = 0x1510;
      regs.x.cx = drive_number;
      regs.x.es = __tb_segment;
      regs.x.bx = __tb_offset + 4;
      __dpmi_int (0x2f, &regs);
      status = _farpeekw (_dos_ds, __tb + 7);
      *bsize  = _farpeekw (_dos_ds, __tb + 2);
    } while (--i && (status & 0x800f) == 0x800f); /* disk changed */

    if (status == 0x100 && _farpeekw (_dos_ds, __tb + 4 + 0x12) == 4)
    {
      request_header.buffer[0x12] = 5; /* number of bytes to transfer */
      /* Put control block into the transfer buffer.  */
      _farpokeb (_dos_ds, __tb, 8); /* read volume size */
      _farpokel (_dos_ds, __tb + 1, 0); /* zero out the result field */

      /* Put request header into the transfer buffer and call the driver.  */
      dosmemput (request_header.buffer, sizeof (request_header.buffer), __tb + 5);
      regs.x.ax = 0x1510;
      regs.x.cx = drive_number;
      regs.x.es = __tb_segment;
      regs.x.bx = __tb_offset + 5;
      __dpmi_int (0x2f, &regs);
      if (_farpeekw (_dos_ds, __tb + 8) == 0x100
       && _farpeekw (_dos_ds, __tb + 5 + 0x12) == 5)
      {
	/* bsize has been set some lines above. */
	*free = 0;  /* no free space: cannot add data to CD-ROM */
	*blocks = _farpeekl (_dos_ds, __tb + 1);
	return 1;
      }
    }
  }

  return 0;
}

/* Returns: 0 == OK, successful setting of variables.
            -1 == call failed, errno set. */
static int
use_AH0x36( int drive_number, long *blocks, long *free, long *bsize )
{
  __dpmi_regs regs;

  /* Get free space info from DOS.  */
  regs.h.ah = 0x36;  /* DOS Get Free Disk Space call */
  regs.h.dl = drive_number + 1;
  __dpmi_int(0x21, &regs);
  
  /* Check for errors */
  if ((regs.x.ax & 0xffff) == 0xffff)
  {
    errno = ENODEV;
    return -1;
  }
  *bsize = regs.x.cx * regs.x.ax;  /* bytes per cluster. */
  *free = regs.x.bx;               /* number of free clusters. */
  *blocks = regs.x.dx;             /* total clusters on drive. */

  return 0;
}


int
statfs(const char *path, struct statfs *buf)
{
  __dpmi_regs regs;
  int cdrom_calls_used = 0;
  int drive_number;
  long blocks = 0;
  long free = 0;
  long bsize = 0;

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

  /* Try cdrom call first. */
  cdrom_calls_used = use_AX0x1510( drive_number, &blocks, &free, &bsize );

  if( 7 <= _osmajor && _osmajor < 10 ) /* Are INT21 AX=7303 and/or 
					  INT21 AX=7302 supported? */
  {
    /* INT21 AX=7303 - Win9x - Get Extended Free Drive Space:
       INT21 AX=7302, Extended Drive Paramenter Block, seems to 
       report the largest block of free clusters when running under 
       Windows (this info is not confirmed), so I'm using this 
       service here. It expects a path on DS:DX and it should not 
       be an empty string or the service call will fail */
    if( path && !*path )
    {
      _put_path2( "/", 0x100 );
    }
    else
    {
      _put_path2( path, 0x100 );
    }

    /* The word at offset 0x2 (structure version) needs to be
       zeroed. */
    _farpokew( _dos_ds, __tb_offset + 0x2, 0 );

    regs.x.ax = 0x7303;
    regs.x.ds = regs.x.es = __tb_segment;
    regs.x.dx = __tb_offset + 0x100;
    regs.x.di = __tb_offset;
    regs.x.cx = 0x100; /* Buffer length. Actually ~70 bytes would be enough */
    __dpmi_int( 0x21, &regs );

    /* In case INT21 AX=7303 fails we try INT21 AX=7302 (the best we
       can do). */
    if( regs.x.flags & 1 )
    {
      /* Get free space info from Extended Drive Parameter Block. */
      regs.x.ax = 0x7302;
      regs.h.dl = drive_number + 1;
      regs.x.es = __tb_segment;
      regs.x.di = __tb_offset;
      regs.x.cx = 0x100; /* 256 bytes should be enough (RBIL says
			    0x3f). */
      __dpmi_int( 0x21, &regs );

      /* Errors? */
      if( regs.x.flags & 1 )
      {
	/* If INT21 AX=7302 fails too, we revert to the old code. */

	if( ! cdrom_calls_used )
	{
	  /* If the earlier call to useAX0x1510() failed we must use INT21
	     AH=63. */
	  if( use_AH0x36( drive_number, &blocks, &free, &bsize ) == -1 )
	  {
	    /* use_AH0x36() sets errno on failure. */
	    return -1;
	  }
	}
	else
	{
	  /* Values untouched since the call to useAX0x1510(). */
	}
      }
      else
      {
        free = _farpeekl (_dos_ds, __tb + 0x2 + 0x1f);
        bsize = _farpeekw (_dos_ds, __tb + 0x2 + 0x2) *
          ( _farpeekb (_dos_ds, __tb + 0x2 + 0x4) + 1 );

        /* -1, because this function was reporting 1 more cluster than
           CHKDSK. */
        blocks = _farpeekl( _dos_ds, __tb + 0x2 + 0x2d) - 1;
      }
    }
    else /* Use information from service AX=0x7303. */
    {
      /* Save block size value from INT21 AH=0x1510 call. */
      long cd_bsize = bsize;

      free   = _farpeekl (_dos_ds, __tb + 0x0c);
      bsize  = _farpeekl (_dos_ds, __tb + 0x08)
	* _farpeekl (_dos_ds, __tb + 0x04);
      blocks = _farpeekl (_dos_ds, __tb + 0x10);
      if( cdrom_calls_used && 2048 < bsize )
      {
	/* useAX0x1510() was successful, but the total and free size
	   is more correct from INT21 AX=7303 (from the point of
	   WINDOZE). However we should use the block size from
	   useAX0x1510() if it's not 2048. */
	blocks *= bsize/cd_bsize;
	free *= bsize/cd_bsize;
	bsize = cd_bsize;
      }
    }
  }
  else
  {
    /* DOZE version earlier than 7.0. Use old method. */
    if( ! cdrom_calls_used )
    {
      if( use_AH0x36( drive_number, &blocks, &free, &bsize ) == -1 )
      {
	/* use_AH0x36() sets errno on failure. */
	return -1;
      }
    }
  }

  /* Fill in the structure */
  buf->f_bavail = free;
  buf->f_bfree = free;
  buf->f_blocks = blocks;
  buf->f_bsize = bsize;
  buf->f_ffree = free;
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
