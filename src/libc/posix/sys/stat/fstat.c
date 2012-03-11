/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* This is file FSTAT.C */
/*
 *   Almost a 100% U**X-compatible fstat() substitute.
 *
 * Usage:
 *
 *   That's easy: just put this in libc.a, and then call fstat() as usual.
 *
 * Rationale:
 *
 *   Many Unix-born programs make heavy use of fstat() library
 *   function to make decisions on files' equality, size, access
 *   attributes etc.  In the MS-DOS environment, many implementations
 *   of fstat() are crippled, because DOS makes it very hard to get to
 *   certain pieces of information about files and directories.  Thus
 *   porting a program to DOS is usually an exercise in #ifdef'ing.
 *   This implementation facilitates porting Unix programs to MS-DOS
 *   by providing an fstat() which is much more Unix-compatible than
 *   those of most DOS-based C compilers (e.g., Borland's).
 *   Specifically, the following issues are taken care of:
 *
 *      1. Mode bits are returned for the actual file, files are NOT
 *         reported read-only (as in Borland's library fstat()).
 *      2. Mode bits are set for all 3 groups (user, group, other).
 *      3. Device code (st_dev, st_rdev) is correctly reported (0 = 'A',
 *         1 = 'B' etc.).
 *      4. Character device names (such as /dev/con, lpt1, aux etc.) are
 *         treated as if they were on a special drive called `@:'
 *         (st_dev = -1).  The "character special" mode bit is set
 *         for these devices.
 *      5. The inode number (st_ino) is taken from the starting cluster
 *         number of the file.  If the cluster number is unavailable, it
 *         is invented using the file's name in a manner that minimizes
 *         the possibility of inventing an inode which already belongs
 *         to another file.  See below for details.
 *      6. Executable files are found based on files' extensions and
 *         magic numbers present at their beginning, and their execute
 *         bits are set.
 *
 *   Lossage:
 *
 *      Beautiful as the above sounds, this implementation does fail
 *      under certain circumstances.  The following is a list of known
 *      problems:
 *
 *      1. Files open on networked drives mounted by Novell Netware
 *         before revision 4.x cannot be traced using DOS System File
 *         Table.  Therefore, name, extension, file attributes and the
 *         drive letter are not available for these.  Until somebody
 *         tells me how this information can be obtained under Novell,
 *         nothing could be done here.  For the time being, these files
 *         will get st_dev of -2.
 *      2. For files which reside on networked drives, the inode number
 *         is always invented, because network redirectors usually do
 *         not bring that info with them.
 *      3. Empty files do not have a starting cluster number, because
 *         DOS doesn't allocate one until you actually write something
 *         to a file.  For these the inode is also invented.
 *      4. If the st_ino field is a 16 bit number, the invented inode
 *         numbers are from 65535 and down, assuming that most disks have
 *         unused portions near their end.  Valid cluster numbers are 16-bit
 *         unsigned integers, so a possibility of a clash exists, although
 *         the last 80 or more cluster numbers are unused on all drives
 *         I've seen.  If the st_ino is 32 bit, then invented inodes are
 *         all greater than 64k, which totally eliminates a possibility
 *         of a clash with an actual cluster number.
 *      5. As this implementation relies heavily on undocumented DOS
 *         features, it will fail to get actual file info in environments
 *         other than native DOS, such as DR-DOS, OS/2 etc.  For these,
 *         the function will return whatever info is available with
 *         conventional DOS calls, which is no less than any other
 *         implementation could do.  This fstat() might also fail for
 *         future DOS versions, if the layout of DOS System File Table
 *         is changed; however, this seems unlikely.
 *
 * Copyright (c) 1994-1996 Eli Zaretskii <eliz@is.elta.co.il>
 *
 * This software may be used freely so long as this copyright notice is
 * left intact.  There is no warranty on this software.
 *
 */

/*
 * Tested with DJGPP port of GNU C compiler, versions 1.11maint5 and 1.12m2,
 * under MS-DOS 3.3, 4.01, 5.0, 6.20 (with and without DoubleSpace) and
 * with networked drives under XFS 1.76, Novell Netware 3.22, and
 * TSoft NFS 0.24Beta.
 *
 */

#include <libc/stubs.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <io.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <dos.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <libc/fd_props.h>
#include <libc/getdinfo.h>

#include <dpmi.h>
#include <go32.h>
#include <libc/farptrgs.h>
#include <libc/bss.h>
#include <libc/symlink.h>
#include <sys/fsext.h>
#include <libc/fsexthlp.h>
#include "xstat.h"

/* Should we bother about executables at all? */
#define _STAT_EXECBIT       (_STAT_EXEC_EXT | _STAT_EXEC_MAGIC)

/* Should we bother about any access bits?  */
#define _STAT_ACCESS        (_STAT_WRITEBIT | _STAT_EXECBIT)

/* Do we need SFT info at all? */
#define _STAT_NEEDS_SFT     (_STAT_WRITEBIT | _STAT_EXEC_EXT | _STAT_INODE)

/*
 * Lower-level assist functions to get a starting cluster of a file,
 * which will serve as an inode number.  The starting cluster is
 * found in the System File Table entry (an internal structure of
 * DOS) which belongs to our file.
 *
 * Much of the following code is derived from file H2NAME.C, which
 * came with ``Undocumented DOS'', 1st edition.
 */

/* Array of SFT entry sizes as function of DOS version. */
static size_t sft_size_list[] = {0, 0x28, 0x35, 0x3b};

/* Actual size of SFT entry for the version of DOS under
   which we are running.  */
static size_t sft_size;

/* Static array to hold a copy of SFT entry.  Should be at
   least as long as the largest number in sft_size_list[].  */
static unsigned char sft_buf[0x40];

/* Linear address of pointer to Job File Table (JFT) which
   is an array of indices into the SFT which correspond to
   open file handles.  */
static unsigned long htbl_ptr_addr;

/* True version of DOS (not the one simulated by SETVER).  */
static unsigned short dos_major, dos_minor;

/* Segment and offset of first SFT sub-table.  All searches
   start from this address.  */
static unsigned short sft_start_seg, sft_start_off;

/* This holds the failure bits from last call to fstat_init(),
   so we can return them each time fstat_assist() is called.  */
static unsigned short fstat_init_bits;

/* Holds the last seen value of __bss_count, to be safe for
   restarted programs (emacs).  */
static int fstat_count = -1;

/* The address of the PSP of the caller.  */
static unsigned long  psp_addr;

/* Initialization routine, called once per program run.
 * Finds DOS version, SFT entry size and addresses of
 * program handle table and first SFT sub-table.
 */
static int
fstat_init(void)
{
  __dpmi_regs    regs;
  int            sft_ptr_addr;
  unsigned short true_dos_version;

  /* Each DOS program has a table of file handles which are used by
   * DOS open() call.  This table holds, for each handle which is in
   * use, the index into the System File Tables' list which contains
   * data about this handle.  The pointer to that handle table is found
   * at offset 34h in the program's PSP.
   */

  /* Linear address of pointer to the handle table.  We postpone
   * dereferencing this pointer to obtain the address of the handle
   * table until we're actually called for a specific handle, because
   * somebody could in the meanwhile change that address, e.g. by
   * calling INT 21h/AX=67h to enlarge the maximum number of file
   * handles.
   */
  htbl_ptr_addr = psp_addr + 0x34;

  /*
   * Find the pointer to the first subtable in the list of SFT's.
   * It is stored at offset 4 in the DOS List-of-Lists area, a ptr
   * to which is returned by the undocumented DOS function 52h.
   * We don't check FLAGS after 52h returns because Ralph Brown's
   * Interrupt List doesn't say FLAGS are set to indicate a
   * failure.
   */
  regs.h.ah = 0x52;
  __dpmi_int(0x21, &regs);

  /* Linear addres of pointer to SFT list.  */
  sft_ptr_addr = MK_FOFF(regs.x.es, regs.x.bx + 4);
  
  /* SFT entry size depends on DOS version.
     We need exact knowledge about DOS internals, so we need the
     TRUE DOS version (not the simulated one by SETVER), if that's
     available.  */
  true_dos_version = _os_trueversion;
  dos_major = true_dos_version >> 8;
  dos_minor = true_dos_version & 0xff;
  sft_size = sft_size_list[dos_major > 4 ? 3 : dos_major - 1];
  if (!sft_size)        /* unsupported DOS version */
    {
      _djstat_fail_bits |= _STFAIL_OSVER;
      return 0;
    }

  /* Segment and offset of start of SFT list.  */
  sft_start_off = _farpeekw(_dos_ds, sft_ptr_addr);
  sft_start_seg = _farpeekw(_dos_ds, sft_ptr_addr + 2);

  return 1;
}

/* Given a handle, copy contents of System File Table entry which
 * belongs to that handle into a local buffer, and return the index
 * into the SFT array where our entry was found.  In case of failure
 * to use SFT, return -2.  If FHANDLE is illegal, return -1.
 */
static short
get_sft_entry(int fhandle)
{
  unsigned long  sft_seg;
  unsigned short sft_off;
  unsigned long  htbl_addr;
  short          sft_idx, retval;
  unsigned short _my_ds_base = _my_ds();

  __dpmi_regs	 regs;
  _djstat_fail_bits = fstat_init_bits;

  /* Force initialization if we were restarted (emacs).  */
  if (fstat_count != __bss_count)
    {
      fstat_count = __bss_count;
      dos_major = 0;
    }

  /* Find the PSP address of the current process.  */
  regs.h.ah = 0x62;	/* Get PSP address */
  __dpmi_int(0x21, &regs);
  psp_addr = (unsigned long)regs.x.bx << 4;

  /* If first time called, initialize.  */
  if (!dos_major && !fstat_init())
    {
      fstat_init_bits = _djstat_fail_bits;
      return -2;
    }

  /* Test file handle for validity.
   * For DOS 3.x and later, the number of possible file handles
   * is at offset 32h in the PSP; for prior versions, it is 20.
   */
  if (fhandle < 0
      || fhandle >= (_osmajor < 3 ?
		     20
		     : _farpeekw(_dos_ds, psp_addr + 0x32)))
    return -1;

  /* Linear address of the handle table. */
  htbl_addr = MK_FOFF(_farpeekw(_dos_ds, htbl_ptr_addr + 2),
                      _farpeekw(_dos_ds, htbl_ptr_addr));

  /* Index of the entry for our file handle in the SFT array.  */
  retval = sft_idx = _farpeekb(_dos_ds, htbl_addr + fhandle);

  if (sft_idx < 0)      /* invalid file handle or bad handle table */
    {
      _djstat_fail_bits |= _STFAIL_SFTIDX;
      return -1;
    }

  /* Given the index into the SFT list, find our SFT entry.
   * The list consists of arrays (sub-tables) of entries, each sub-
   * table preceeded by a header.  The header holds a pointer to the
   * next sub-table in the list and number of entries in this sub-table.
   * The list is searched until the sub-table which contains our
   * target is found, then the sub-table entries are skipped until
   * we arrive at our target.
   */

  /* Segment (shifted 4 bits left) and offset of start of SFT list.  */
  sft_off = sft_start_off;
  sft_seg = MK_FOFF(sft_start_seg, 0);

  while (sft_off != 0xFFFF)
    {
      unsigned long entry_addr   = sft_seg + sft_off;
      short         subtable_len = _farpeekw(_dos_ds, entry_addr + 4);

      if (sft_idx < subtable_len)
        { /* Our target is in this sub-table.  Pull in the entire
           * SFT entry for use by fstat_assist().
           */
	  movedata(_dos_ds,
                   entry_addr + 6 + sft_idx * sft_size,
		   _my_ds_base, (unsigned int)sft_buf, sft_size);
          return retval;
        }
      /* Our target not in this subtable.
       * Subtract the number of entries in this sub-table from the
       * index of our entry, and proceed to next sub-table.
       */
      sft_idx -= subtable_len;
      sft_off  = _farpeekw(_dos_ds, entry_addr);
      sft_seg  = MK_FOFF(_farpeekw(_dos_ds, entry_addr + 2), 0);
    }

  /* Get here only by error, which probably means unsupported DOS version. */
  _djstat_fail_bits |= _STFAIL_SFTNF;
  return -2;
}

/* On LFN platforms, we can get all the 3 time-related fields.  */

static void
set_fstat_times (int fhandle, struct stat *stat_buf)
{
  if (_USE_LFN)
    {
      time_t access_time;
      unsigned int create_time;

      /* Access time is currently date only (time is zeroed).  */
      access_time = _file_time_stamp (_lfn_get_ftime (fhandle, _LFN_ATIME));
      if (access_time > stat_buf->st_atime)
	stat_buf->st_atime = access_time;

      /* Creation time might be zero if the file was created
	 by a DOS program which doesn't support LFN API.  */
      create_time = _lfn_get_ftime (fhandle, _LFN_CTIME);
      if (create_time)
	stat_buf->st_ctime = _file_time_stamp (create_time);
    }
}

/* fstat_assist() is where all the actual work is done.
 * It uses SFT entry, if available and its contents are verified.
 * Otherwise, it finds all the available info by conventional
 * DOS calls.
 */

static int
fstat_assist(int fhandle, struct stat *stat_buf)
{
  short          have_trusted_values = 1;
  unsigned int   dos_ftime;
  char           drv_no;
  short          dev_info;
  unsigned char  is_dev;
  unsigned char  is_remote;
  short          sft_idx = -1;
  unsigned short sft_fdate, sft_ftime;
  long           sft_fsize;
  unsigned short trusted_ftime = 0, trusted_fdate = 0;
  long           trusted_fsize = 0;
  int            is_link = 0;
  const char    *fd_name = NULL;
  const char    *filename = "";

  if ((dev_info = _get_dev_info(fhandle)) == -1)
    return -1;	/* errno set by _get_dev_info() */

  _djstat_fail_bits = 0;

  /* Get pointer to an SFT entry which holds data for our handle. */
  if ( (_djstat_flags & _STAT_NEEDS_SFT) == 0 &&
       (sft_idx = get_sft_entry(fhandle)) == -1)
    {
      errno = EBADF;
      return -1;
    }

  /* Initialize buffers. */
  memset(stat_buf, 0, sizeof(struct stat));
  dos_ftime = 0;

  /* Get some info about this handle by conventional DOS calls.  These
   * will serve as verification of SFT entry contents and also as
   * fall-back in case SFT method fails.
   */
  if (_getftime(fhandle, &dos_ftime) == 0 &&
      (trusted_fsize = __filelength(fhandle)) != -1L)
    {
      trusted_ftime = dos_ftime & 0xffff;
      trusted_fdate = dos_ftime >> 16;
    }
  else
    have_trusted_values = 0;

  if (dev_info & _DEV_CDEV)
    {
      is_dev = 1;
      is_remote = 0;	/* device can't be remote */
    }
  else
    {
      is_dev = 0;
      if (dev_info & _DEV_REMOTE)
	is_remote = 1;
      else
	is_remote = 0;
      
      if(!have_trusted_values && dev_info == 0 && _os_trueversion == 0x532)
        is_dev = 1;   /* Device under NT or Win2K with pre-open/lfn handle. */
    }

  /* First, fill the fields which are constant under DOS. */
  stat_buf->st_uid = getuid();
  stat_buf->st_gid = getgid();
  stat_buf->st_nlink = 1;

  /* Get the file name from the file descriptor properties (fd_props),
   * if possible, and fix it up. */
  fd_name = __get_fd_name(fhandle);
  if (fd_name != NULL)
    filename = fd_name;

  /* Get the block size for the device associated with `fhandle'. */
#ifndef  NO_ST_BLKSIZE
  if (*filename)
    {
      stat_buf->st_blksize = _get_cached_blksize(filename);
      if (stat_buf->st_blksize == -1)
	return -1; /* errno set by _get_cached_blksize() */
    }
  else
    {
      /* Fall back on transfer buffer size, if we can't determine file name
       * (which gives the drive letter and then the drive's cluster size). */
      stat_buf->st_blksize = __tb_size;
    }
#endif

  /* If SFT entry for our handle is required and available, we will use it.  */
  if ( (_djstat_flags & _STAT_NEEDS_SFT) == 0 && sft_idx >= 0)
    {
      /* Determine positions of data items in the SFT. */
      size_t fattr_ofs, name_ofs, ext_ofs, fsize_ofs, fdate_ofs,
             ftime_ofs, clust_ofs;

      switch (dos_major)
        {
          case 2:
              fattr_ofs  = 2;
              drv_no     = sft_buf[3] - 1;      /* 1 = 'A' etc. */
              name_ofs   = 4;
              ext_ofs    = 0x0b;
              fsize_ofs  = 0x13;
              fdate_ofs  = 0x17;
              ftime_ofs  = 0x19;
              clust_ofs  = 0x1c;
              is_remote  = 0;   /* DOS 2.x didn't have remote files */
              break;

          case 3:
              fattr_ofs  = 4;
              drv_no     = sft_buf[5] & 0x3f;
              if (dos_minor == 0)
                {
                  name_ofs = 0x21;
                  ext_ofs  = 0x29;
                }
              else      /* DOS 3.1 - 3.3x */
                {
                  name_ofs = 0x20;
                  ext_ofs  = 0x28;
                }
              clust_ofs  = 0x0b;
              ftime_ofs  = 0x0d;
              fdate_ofs  = 0x0f;
              fsize_ofs  = 0x11;
              break;

          default:      /* DOS 4 and up */
              fattr_ofs  = 4;
              drv_no     = sft_buf[5] & 0x3f;
              clust_ofs  = 0x0b;
              ftime_ofs  = 0x0d;
              fdate_ofs  = 0x0f;
              fsize_ofs  = 0x11;
              name_ofs   = 0x20;
              ext_ofs    = 0x28;

        }

      if (is_dev)
        {
          /* We have a character device.
           * We will pretend as if they all reside on a special
           * drive `@:', which is illegal in DOS, and just happens
           * to give a neat st_dev (= '@' - 'A') = -1.
           */

          stat_buf->st_dev = -1;
#ifdef  HAVE_ST_RDEV
          stat_buf->st_rdev = -1;
#endif

          if ( (_djstat_flags & _STAT_INODE) == 0 )
            {
              /* Character device names are all at most 8-character long. */
              short i   = 8;
              unsigned char *src = sft_buf + name_ofs;
              char dev_name[16], *dst = dev_name + 7;

              strcpy(dev_name, "@:\\dev\\        "); /* pad with 8 blanks */
              while (i-- && *src != ' ')             /* copy non-blank chars */
                *dst++ = *src++;

              stat_buf->st_ino = _invent_inode(dev_name, 0, 0);
            }

          /* Should we make printer devices write-only here? */
          stat_buf->st_mode |= (S_IFCHR | READ_ACCESS | WRITE_ACCESS);

          /* We will arrange things so that devices have current time in
           * the access-time and modified-time fields of struct stat.
           */
          stat_buf->st_atime = stat_buf->st_mtime = time(0);

          /* MS-DOS returns the time of boot when _getftime() is called
           * for character devices, but this is undocumented and
           * unsupported by DOS clones (e.g. DR-DOS).  It is also
           * inconsistent with our stat().  Therefore, we will arrange
           * things so that devices have zero (the beginning of times)
           * in creation-time field.
           */
          dos_ftime = 0;
          stat_buf->st_ctime = _file_time_stamp(dos_ftime);

          return 0;
        }

      /* Files are not allowed to fail DOS calls for their time
       * stamps and size.
       */
      else if (have_trusted_values)
        {
	  unsigned char *pname = sft_buf + name_ofs;

          /* This is a regular, existing file.  It cannot be a
           * directory, because DOS won't let us open() a directory.
           * Each file under MS-DOS is always readable by everyone.
           */
          stat_buf->st_mode |= (S_IFREG | READ_ACCESS);
          
          /* We will be extra careful in trusting SFT data: it must be
           * consistent with date, time and size of the file as known
           * from conventional DOS calls.
           */
          sft_fdate = *((unsigned short *)(sft_buf + fdate_ofs));
          sft_ftime = *((unsigned short *)(sft_buf + ftime_ofs));
          sft_fsize = *((long *)(sft_buf + fsize_ofs));

	  /* In addition, it seems that 32-bit File Access, at least
	   * in Windows 95, creates fake SFT entries for some files,
	   * which have bogus cluster numbers and all-blank file name
	   * and extension.  (It is unclear to me when exactly are
	   * these fake SFT entries created.)
	   * So, in addition, we check the file's name in the SFT to
	   * be non-blank, since file names in the FCB format cannot
	   * be all-blank, even on Windows 95.  */
	  while (pname < sft_buf + name_ofs + 8 + 3 && *pname == ' ')
	    pname++;

          if (pname < sft_buf + name_ofs + 8 + 3 &&
	      sft_ftime == trusted_ftime &&
              sft_fdate == trusted_fdate &&
              sft_fsize == trusted_fsize)
            { /* Now we are ready to get the SFT info. */
              char           sft_extension[4], *dst = sft_extension + 2;
              unsigned char *src = sft_buf + ext_ofs + 2;
              int i = 3;

              /* Get the file's extension.  It is held in the SFT entry
               * as a blank-padded 3-character string without terminating
               * zero.  Some crazy files have embedded blanks in their
               * extensions, so only TRAILING blanks are insignificant.
               */
              memset(sft_extension, 0, sizeof(sft_extension));;
              while (*src == ' ' && i--)    /* skip traling blanks */
                {
                  dst--; src--;
                }

              if (i >= 0)
                while (i--)                 /* move whatever left */
                  *dst-- = *src--;

              /* Build Unix-style file permission bits. */
              /* Check for symlink at first.  This test will fail if we 
               * don't have read access to file.
               */
              if (sft_fsize == 510)
              {
                 int old_errno = errno;
		 char buf[2];
		 int bytes_read = __internal_readlink(NULL, fhandle, buf, 1);
		 if (bytes_read != -1)
		 {
		    stat_buf->st_mode = S_IFLNK;
		    is_link = 1;
		 }
		 else
 		    errno = old_errno;
              }
              if (!is_link)
              {
                 if ( !(sft_buf[fattr_ofs] & 0x07) ) /* no R, S or H bits set */
                   stat_buf->st_mode |= WRITE_ACCESS;

                 /* Execute permission bits.  fstat() cannot be called on
                  * directories under DOS, so only executable programs/batch
                  * files should be considered.
                  */
                 if (_is_executable((const char *)0, fhandle, sft_extension))
                   stat_buf->st_mode |= EXEC_ACCESS;

                 /* DOS 4.x and above seems to know about named pipes. */
                 if (dos_major > 3 && (sft_buf[6] & 0x20))
                   stat_buf->st_mode |= S_IFIFO;
              }
              /* Device code. */
              stat_buf->st_dev = drv_no;
#ifdef  HAVE_ST_RDEV
              stat_buf->st_rdev = drv_no;
#endif

              /* The file's starting cluster number will serve as its
               * inode number.
               */
              if ( (_djstat_flags & _STAT_INODE) == 0 && !is_remote)
                stat_buf->st_ino = *((unsigned short *)(sft_buf + clust_ofs));

              /* If the cluster number returns zero (e.g., for empty files,
               * because DOS didn't allocate it a cluster yet) we have to
               * invent the inode using the file's name.  We will use the
               * index into the SFT as part of unique identifier for our
               * file, so a possibility of two files with the same name
               * but different paths getting the same inode number is
               * minimized.
               * If we have a remote file, we invent inode even if there
               * is a non-zero number in the SFT, because it usually is
               * bogus (a left-over from last local file handle which used
               * the same SFT entry).
               * Note that we invent the inode even if is_remote is -1
               * (i.e., IOCTL Func 0Ah failed), because that should mean
               * some network redirector grabs IOCTL functions in an
               * incompatible way.
               */
              if ( (_djstat_flags & _STAT_INODE) == 0 &&
                   (stat_buf->st_ino == 0 || is_remote))
                {
                  static char    name_pat[]   = " :sft-   \\            ";
                  char           name_buf[sizeof(name_pat)];
                  unsigned char *src_p        = sft_buf + name_ofs + 7;
                  char          *dst_p        = name_buf + 17;
                  int            j            = 8;
                  char          *name_end;
                  int            first_digit  = sft_idx / 100;
                  int            second_digit = (sft_idx - first_digit * 100) / 10;
                  int            third_digit  = sft_idx - first_digit * 100
                                                        - second_digit * 10;

                  /* Initialize the name buffer with zeroes, then
                   * put in the drive letter and ``sft-XXX'', where
                   * XXX is the index of our file entry in the SFT.
                   */
                  strcpy(name_buf, name_pat);
                  memset(name_buf + 10, 0, sizeof(name_buf) - 10);
                  name_buf[0] = drv_no + 'A';
                  name_buf[6] = first_digit  + '0';
                  name_buf[7] = second_digit + '0';
                  name_buf[8] = third_digit  + '0';

                  /* Copy filename from SFT entry to local storage.
                   * It is stored there in the infamous DOS format:
                   * both name and extension are blank-padded, and no dot.
                   * We cannot use strcpy, because the name might
                   * include embedded blanks.  Therefore we move the
                   * characters from the end towards the beginning.
                   */
                  while (*src_p == ' ' && j--)   /* skip traling blanks */
                    {
                      dst_p--;
                      src_p--;
                    }
                  name_end = dst_p + 1;

                  if (j >= 0)                  /* move whatever left */
                    while (j--)
                      *dst_p-- = *src_p--;

                  /* We've already got the extension.  If it is non-empty,
                   * insert a dot and copy the extension itself.
                   */
                  if (sft_extension[0])
                    {
                      *name_end++ = '.';
                      strcpy(name_end, sft_extension);
                    }
                  stat_buf->st_ino =
                    _invent_inode(name_buf, dos_ftime, sft_fsize);
                  _djstat_fail_bits |= _STFAIL_HASH;
                }

              /* Size, date and time. */
              stat_buf->st_size = sft_fsize;
              stat_buf->st_atime = stat_buf->st_ctime = stat_buf->st_mtime =
                _file_time_stamp(dos_ftime);

	      /* Additional time info for LFN platforms.  */
	      set_fstat_times (fhandle, stat_buf);
              return 0;
            }

          _djstat_fail_bits |= _STFAIL_BADSFT;

        }

      /* Regular file, but DOS calls to find its length and time stamp
       * failed.  This must be an illegal file handle, or something
       * else very, very funny...
       */
      else
        return -1;    /* errno set by filelength() or getftime() */

    }

  /* Can't get SFT itself or can't find SFT entry belonging to our file.
   * This is probably unsupported variety of DOS, or other (not-so-
   * compatible) OS.
   * For these we supply whatever info we can find by conventional calls.
   */
  if (have_trusted_values)
    {
      if (is_dev)
        {
          if (_djstat_flags & _STAT_INODE)
            {
              /* We need the name of the device to invent an inode for it.
               * We cannot get the REAL name, because SFT info is unavailable.
               * If IOCTL tells us this is one of the standard devices, we
               * can make an educated guess.  If not, we will invent inode
               * with no name.  This will at least ensure that no two calls
               * accidentally get the same inode number.
               * We will also pretend devices belong to a special drive
               * named `@'.
               */
              if (dev_info & (_DEV_STDIN|_DEV_STDOUT|_DEV_NUL|_DEV_CLOCK))
                {
                  char dev_name[16];

                  strcpy(dev_name, "@:\\dev\\");
                  if (dev_info & (_DEV_STDIN|_DEV_STDOUT))
                    strcat(dev_name, "CON     ");
                  else if (dev_info & _DEV_NUL)
                    strcat(dev_name, "NUL     ");
                  else if (dev_info & _DEV_CLOCK)
                    strcat(dev_name, "CLOCK$  ");

                  stat_buf->st_ino = _invent_inode(dev_name, 0, 0);
                }
              else
                stat_buf->st_ino = _invent_inode("", 0, 0);

              _djstat_fail_bits |= _STFAIL_HASH;
            }

          stat_buf->st_dev = -1;
#ifdef  HAVE_ST_RDEV
          stat_buf->st_rdev = -1;
#endif

          stat_buf->st_mode |= (S_IFCHR | READ_ACCESS | WRITE_ACCESS);

          stat_buf->st_atime = stat_buf->st_mtime = time(0);
          dos_ftime = 0;
          stat_buf->st_ctime = _file_time_stamp(dos_ftime);
        }
      else
        {
          /* Regular file.  We may have obtained this file's name
	   * from the file descriptor properties (fd_props).  Otherwise
	   * the inode will be arbitrary each time fstat is called.
	   * Sigh...
           */
          if ( (_djstat_flags & _STAT_INODE) == 0 )
            {
              _djstat_fail_bits |= _STFAIL_HASH;
              stat_buf->st_ino
		= _invent_inode(filename, dos_ftime, trusted_fsize);
            }

          if (trusted_fsize == 510)
          {
            int old_errno = errno;
            char buf[2];
            int bytes_read = __internal_readlink(NULL, fhandle, buf, 1);
            if (bytes_read != -1)
            {
              stat_buf->st_mode = S_IFLNK;
              is_link = 1;
            }
            else
              errno = old_errno;
          }
          if (!is_link)
          {
            /* Return the minimum access bits every file has under DOS. */
            stat_buf->st_mode |= (S_IFREG | READ_ACCESS);
            if (_djstat_flags & _STAT_ACCESS)
              _djstat_fail_bits |= _STFAIL_WRITEBIT;

            /* If we are runing on Windows 9X, NT 4.0 with LFN or 2000 or XP
               with LFN is enabled, try harder. Note that we deliberately do
               NOT use this call when LFN is disabled, even if we are on
               Windows, because then we open the file with function 3Ch, and
               such handles aren't supported by 71A6h call we use here.  */
            if (_USE_LFN)
            {
              __dpmi_regs r;

              r.x.flags = 1;	/* Always set CF before calling a 0x71NN function. */
              r.x.ax = 0x71a6;	/* file info by handle */
              r.x.bx = fhandle;
              r.x.ds = __tb >> 4;
              r.x.dx = 0;
              __dpmi_int(0x21, &r);
              if (!(r.x.flags & 1) && (r.x.ax != 0x7100) &&
                  !(_farpeekl(_dos_ds, __tb) & 0x07))
              {
                /*  Never assume that the complete LFN API is implemented,
                    so check that AX != 0x7100.  E.G.: MSDOS 6.22 and DOSLFN 0.40.  */
                stat_buf->st_mode |= WRITE_ACCESS; /* no R, S or H bits set */
              }
            }

            /* Executables are detected if they have magic numbers.  */
            if (!(_djstat_flags & _STAT_EXEC_MAGIC) &&
                _is_executable((const char *)0, fhandle, (const char *)0))
              stat_buf->st_mode |= EXEC_ACCESS;
          }
          /* Lower 6 bits of IOCTL return value give the device number. */
          stat_buf->st_dev = dev_info & 0x3f;
#ifdef  HAVE_ST_RDEV
          stat_buf->st_rdev = dev_info & 0x3f;
#endif

          /* Novell Netware does not return the drive number in the
           * lower 6 bits of dev_info.  But we cannot do anything with
           * that, since any value in these 6 bits could be correct...
           * In particular, 0 there means the A: drive.
           */
          stat_buf->st_size  = trusted_fsize;
          stat_buf->st_atime = stat_buf->st_ctime = stat_buf->st_mtime =
            _file_time_stamp(dos_ftime);

          /* Additional time info for LFN platforms.  */
          set_fstat_times(fhandle, stat_buf);
        }
      return 0;
    }

    /* Don't have even values from conventional DOS calls.
     * Give up completely on this funny handle.  ERRNO is already
     * set by filelength() and/or getftime().
     */
  else
    return -1;
}

/*
 * Main entry point.  This is a substitute for library fstat() function.
 */

int
fstat(int handle, struct stat *statbuf)
{
  int            e = errno;     /* save previous value of errno */
  __FSEXT_Function* func;
  int rv;

  if (!statbuf)
  {
    errno = EFAULT;
    return -1;
  }

  /* see if this is file system extension file */
  func = __FSEXT_get_function(handle);
  if (func && __FSEXT_func_wrapper(func, __FSEXT_fstat, &rv, handle, statbuf))
  {
     return rv;
  }

  /* See if this is a file descriptor for a directory. If so, just
   * use a normal stat call. */
  if (__get_fd_flags(handle) & FILE_DESC_DIRECTORY)
  {
    const char *filename = __get_fd_name(handle);

    if (filename)
      return stat(filename, statbuf);
  }

  if (fstat_assist(handle, statbuf) == -1)
  {
    return -1;      /* already have ERRNO set by fstat_assist() */
  }
  else
  {
    errno = e;
    return 0;
  }
}

#ifdef  TEST

#include <stdio.h>
#include <fcntl.h>

unsigned short _djstat_flags = 0;

int main(int argc, char *argv[])
{
  struct stat stat_buf;
  int fd = -1;
  int i;
  char *endp;

  argc--; argv++;
  _djstat_flags = (unsigned short)strtoul(*argv, &endp, 0);

  /* Display 4 standard handles which are already open. */
  for (i = 0; i <= 4; i++)
    {
      fstat(i, &stat_buf);
      fprintf(stderr, "handle-%d: %d %6u %o %d %d %ld %lu %s", i,
              stat_buf.st_dev,
              (unsigned)stat_buf.st_ino,
              stat_buf.st_mode,
              stat_buf.st_nlink,
              stat_buf.st_uid,
              (long)stat_buf.st_size,
              (unsigned long)stat_buf.st_mtime,
              ctime(&stat_buf.st_mtime));
      _djstat_describe_lossage(stderr);
    }

  if (!argc)
    return EXIT_SUCCESS;

  /* Now call fstat() for each command-line argument. */
  while (++argv, --argc)
    {
      if (fd >= 19)
        close(fd);
      fd = open(*argv, O_RDONLY);
      if (fd != -1 && !fstat(fd, &stat_buf))
        {
          fprintf(stderr, "%s (%d): %d %6u %o %d %d %ld %lu %s", *argv, fd,
                  stat_buf.st_dev,
                  (unsigned)stat_buf.st_ino,
                  stat_buf.st_mode,
                  stat_buf.st_nlink,
                  stat_buf.st_uid,
                  (long)stat_buf.st_size,
                  (unsigned long)stat_buf.st_mtime,
                  ctime(&stat_buf.st_mtime));
	  fprintf (stderr, "\t\t\tTimes: %lu %lu\n",
		   (unsigned long)stat_buf.st_atime,
		   (unsigned long)stat_buf.st_ctime);
	  fprintf(stderr, "\t\t\tBlock size: %d\n",
		  stat_buf.st_blksize);
          _djstat_describe_lossage(stderr);
        }
      else
        {
          fputs(*argv, stderr);
          perror(": failed to open/fstat");
          _djstat_describe_lossage(stderr);
        }
    }
  return EXIT_SUCCESS;
}

#endif  /* TEST */

