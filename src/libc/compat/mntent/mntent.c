/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * This is implementation of getmntent() and friends for DJGPP v2.x.
 *
 * Copyright (c) 1995-98 Eli Zaretskii <eliz@is.elta.co.il>
 *
 * This software may be used freely so long as this copyright notice is
 * left intact.  There is no warranty on this software.
 *
 * ---------------------------------------------------------------------
 *
 * The primary motivation for these functions was the GNU df program,
 * which lists all the mounted filesystems with a summary of the disk
 * space available on each one of them.  However, they are also useful
 * on their own right.
 *
 * Unlike Unix, where all mountable filesystems can be found on special
 * file (and thus implementing these function boils down to reading that
 * file), with MS-DOS it's a mess.  Every type of drive has its own
 * interface; there are JOINed and SUBSTed pseudo-drives and RAM disks;
 * different network redirectors hook DOS in a plethora of incompatible
 * ways; a single drive A: can be mapped to either A: or B:, etc.  That
 * is why this implementation uses almost every trick in the book to get
 * at the intimate details of every drive.  Some places where you might
 * find these tricks are: ``Undocumented DOS, 2nd ed.'' by Schulman et al
 * and Ralf Brown's Interrupt List.
 *
 */
#include <libc/stubs.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <mntent.h>
#include <dir.h>
#include <dos.h>
#include <bios.h>
#include <dpmi.h>
#include <go32.h>
#include <libc/farptrgs.h>
#include <sys/movedata.h>
#include <libc/dosio.h>
#include <libc/unconst.h>
#include <stdlib.h>
#include <fcntl.h>

/* Macro to convert a segment and an offset to a "far offset" suitable
   for _farxxx() functions of DJGPP.  */
#ifndef MK_FOFF
#define MK_FOFF(s,o) ((int)((((unsigned long)(unsigned short)(s)) << 4) + \
                      (unsigned short)(o)))
#endif

#define CDS_JOIN     0x2000
#define CDS_VALID    0xc000
#define REMOVABLE    0
#define FIXED        1

/* Static variables.  */

static signed char   drive_number = -1;
static char          skip_drive_b = 0;
static char          drive_a_mapping = 0;
static char          w2k_bug;
static char          cds_drives   = 0;
static unsigned long cds_address;
static int           cds_elsize;
static unsigned short dos_mem_base, our_mem_base;
static struct mntent mntent;
static char          drive_string[128];
static char          *mnt_type;
static char          mnt_dir[128];
static char          mnt_fsname[128];
static char          dev_opts[] = "r ,dev=  ";

static char NAME_dblsp[] = "dblsp";
static char NAME_stac[] = "stac";
static char NAME_jam[] = "jam";
static char NAME_ram[] = "ram";
static char NAME_cdrom[] = "cdrom";
static char NAME_net[] = "net";
static char NAME_fd[] = "fd";
static char NAME_hd[] = "hd";
static char NAME_subst[] = "subst";
static char NAME_join[] = "join";
static char NAME_unknown[] = "???";

int _is_remote_drive(int);

/* Static helper functions.  */

/*
 * Get the entry for this disk in the DOS Current Directory Structure
 * (CDS).  In case of success, return this drive's attribute word; or
 * 0 in case of failure.  Fill the buffer at CURRDIR with the current
 * directory on that drive.
 * The pointer to the CDS array and the size of the array element
 * (which are DOS version-dependent) are computed when setmntent() is
 * called.
 */
static int
get_cds_entry(int drive_num, char *currdir)
{
  unsigned long  cds_entry_address;
  if (!cds_address)
    {
      *currdir = '\0';
      return 0;
    }

  /* The address of the CDS element for this drive.  */
  cds_entry_address = cds_address + (drive_num - 1)*cds_elsize;
  
  /* The current directory: 67-byte ASCIIZ string at the beginning
     of the CDS structure for our drive.  */
  movedata(dos_mem_base, cds_entry_address,
           our_mem_base, (unsigned int)currdir, 0x43);

  /* The drive attribute word is at the offset 43h, right after the
     current directory string.  NT doesn't support this.  */
  return
    cds_elsize == 0x47 ? 0 : _farpeekw(dos_mem_base, cds_entry_address + 0x43);
}

/*
 * For a PC with a single floppy drive, that drive can be referenced
 * as both A: and B:.  This function returns the logical drive number
 * which was last used to reference a physical drive, or 0 if the
 * drive has only one logical drive assigned to it (which means there
 * are two floppies in this system).
 */
static int
assigned_to(int drive_num)
{
  __dpmi_regs r;

  /* Issue Int 21h/AX=440Eh to get logical drive last used to
     reference the physical drive DRIVE_NUM.  */
  r.x.ax = 0x440e;
  r.h.bl = drive_num;
  __dpmi_int(0x21, &r);
  if (r.x.flags & 1)
    return 0;
  return r.h.al;
}

/* This function remaps the single physical floppy drive to
 * the logical drive passed as the argument.
 */
static int
assign_floppy_to(int drive_num)
{
  __dpmi_regs r;

  r.x.ax = 0x440f;
  r.h.bl = drive_num;
  __dpmi_int(0x21, &r);
  if (r.x.flags & 1)
    return -1;
  return 0;
}

/*
 * Check if the drive is compressed with DoubleSpace.  If it is,
 * get the host drive on which the Compressed Volume File (CVF)
 * resides, put the name of that CVF into MNT_FSNAME[] and return
 * non-zero.
 */
static int
get_doublespace_info(int drive_num)
{
  __dpmi_regs r;

  r.x.ax = 0x4a11;      /* DBLSPACE Int 2Fh API */
  r.x.bx = 1;
  r.h.dl = drive_num - 1; /* 0 = A: */
  __dpmi_int(0x2f, &r);

  if (r.x.ax == 0 && r.h.bl & 0x80)
    {
      int host = r.h.bl & 0x7f;

      /* Compressed drive, get the host and sequence number of the CVF.  */
      sprintf(mnt_fsname, "%c:\\DBLSPACE.%03u", 'A' + host, r.h.bh);
      mnt_type = NAME_dblsp;
      return 1;
    }
  else
    /* Error, or uncompressed drive (might be a host, but don't care).  */
    return 0;
}

/*
 * For a drive compressed with Stacker, get the name of the host
 * of its compressed volume file, fill MNT_FSNAME[] with its
 * name and return non-zero.  If this drive isn't controlled by
 * Stacker, return 0.
 */
static int
get_stacker_info(int drive_num)
{
  __dpmi_regs r;
  unsigned long stac_driver_ptr;
  unsigned char seq, host;

  /* Put a known DWORD into the Transfer Buffer.  If this drive
     isn't compressed with Stacker, it will remain unchanged.  */
  _farpokel(dos_mem_base, __tb, 0xbadabadaU);

  r.x.ax = 0x4404;      /* Stacker Get Driver Address function */
  r.h.bl = drive_num;
  r.x.cx = 4;
  r.x.ds = __tb >> 4;
  r.x.dx = __tb & 15;
  __dpmi_int(0x21, &r);

  if ((stac_driver_ptr = _farpeekl(dos_mem_base, __tb)) == 0xbadabadaU)
    return 0;

  /* This drive MIGHT be compressed with Stacker.  Construct a linear
     address of the far pointer into the Stacker device driver.  */
  stac_driver_ptr = ((stac_driver_ptr >> 12) & 0xffff0)
                    + (stac_driver_ptr & 0xffff);

  /* Sanity check: real-mode addresses are only 20 bit-long, so we can
     safely reject anything that's larger than FFFFFh, lest we get an
     illegal address abort when we try to peek at the signature below.
     Actually, it's enough to test for (FFFFFh - 55h - drive), because
     we need to get the host drive number at that offset. */
  if (stac_driver_ptr > 0x0000fffaa - (unsigned)drive_num)
    return 0;

  /* Stacker Anywhere returns pointer to 1 byte before the A55Ah
     signature (which is at offset 1Ah), while all other versions
     of Stacker point to the signature itself.  */
  if (_farpeekw(dos_mem_base,   stac_driver_ptr) != 0xa55a &&
      _farpeekw(dos_mem_base, ++stac_driver_ptr) != 0xa55a)
    return 0;
  stac_driver_ptr -= 0x1a; /* start of the device driver */

  /* Check for the "SWAP" signature.  */                /*  P A W S  */
  if (_farpeekl(dos_mem_base, stac_driver_ptr + 0x6c) != 0x50415753)
    return 0;

  /* We have indeed Stacker device driver.  Get the volume
     number (at offset 58h) and host drive (from the 26-byte
     table beginning at offset 70h).  */
  seq  = _farpeekb(dos_mem_base, stac_driver_ptr + 0x58);
  host = _farpeekb(dos_mem_base, stac_driver_ptr + 0x70 + drive_num - 1);
  sprintf(mnt_fsname, "%c:\\STACVOL.%03u", 'A' + host, seq);
  mnt_type = NAME_stac;
  return 1;
}

/*
 * For a drive compressed with Jam, get the full file name of
 * the compressed archive file, fill MNT_FSNAME[] with its
 * name and return non-zero.  If this drive isn't controlled by
 * Jam, return 0.
 *
 * (JAM is a shareware disk compressor software.)
 *
 * Contributed by Markus F.X.J. Oberhumer <markus.oberhumer@jk.uni-linz.ac.at>
 */
static int
get_jam_info(int drive_num)
{
  __dpmi_regs r;
  unsigned jam_version, offset;

  r.x.ax = 0x5200;              /* Jam Get Version */
  __dpmi_int(0x2f, &r);
  if (r.h.ah != 0x80)           /* JAM.SYS is not installed */
    return 0;
  jam_version = r.x.bx;         /* v1.25 == 0x125 */
  if (jam_version < 0x110)      /* version sanity check */
    return 0;
  /* Sanity check of the size of the JAMINFO structure.  */
  if (r.x.cx < 0x115 || r.x.cx > __tb_size)
    return 0;
  if (jam_version >= 0x120 && r.x.cx < 0x124)
    return 0;

  r.x.ax = 0x5201;              /* Jam Get Compressed Drive Information */
  r.h.dl = drive_num;
  r.x.ds = __tb >> 4;
  r.x.bx = __tb & 15;
  __dpmi_int(0x2f, &r);
  if (r.h.ah != 0)              /* drive is not a Jam drive */
    return 0;

  /* Check that the drive is mounted (attached).  */
  r.h.ah = 0x32;                /* Get Device Parameter Block function */
  r.h.dl = drive_num;
  __dpmi_int(0x21, &r);
  if (r.h.al != 0)              /* drive is not mounted (or other error) */
    return 0;
       
  /* Copy the full name of the Jam archive file.  */
  offset = (jam_version >= 0x120) ? 0x38 : 0x2a;
  movedata(dos_mem_base, __tb + offset,
           our_mem_base, (unsigned) mnt_fsname, 127);
  mnt_fsname[127] = 0;

  mnt_type = NAME_jam;
  return 1;
}

/*
 * Get the network name which corresponds to a drive DRIVE_NUM.
 * Ideally, _truename() (Int 21h/AH=60h) should return the same
 * string, but some network redirectors don't put a full UNC
 * name into the CDS, and others bypass the CDS altogether.
 */
static int
get_netredir_entry(int drive_num)
{
  __dpmi_regs r;
  unsigned long tb = __tb;

  union
  {
      unsigned char ch[2];
      unsigned short s;
  } devname;

  int i = -1;

  r.x.ds = r.x.es = (tb >> 4);
  r.x.si = tb & 15;
  r.x.di = r.x.si + 16;

  /* Loop for all the valid list indices, comparing the local device
     name with our disk drive letter, until we find one which matches.

     We could cache entries which we've seen, but (1) I never said
     this will be the fastest function ever; and (2) I can't be sure
     the network configuration won't change between two successive
     calls to getmntent().  */
  while (++i < 35)  /* at the maximum: 32 drives + 3 LPTn devices */
    {
      r.x.cx = 0;
      r.x.bx = i;
      r.x.ax = 0x5f02;
      __dpmi_int(0x21, &r);

      if (r.x.flags & 1)
        {
          if (r.x.ax == 0x12) /* end of list--bail out */
            return 0;

          else
            continue;
        }
      else if ((r.h.bh == 0 || r.h.bh == 2) && r.h.bl == 4)
        {
          /* We have a valid device which is a disk drive (BL = 4).
             Pull in the local device name and if that fits our
             drive number, get its network name.  */
          devname.s = _farpeekw(dos_mem_base, tb);

          /* The local device name may or may not include the
             colon (Ralf Brown's Interrupt List).  */
          if (devname.ch[0] == '@' + drive_num &&
              (devname.ch[1] == ':' || devname.ch[1] == '\0'))
            {
              movedata(dos_mem_base, tb + 16,
                       our_mem_base, (unsigned)mnt_fsname, 128);
              return 1; /* found! */
            }
        }
    }
  return 0;
}

/*
 * Return 1 if a CD-ROM drive DRIVE_NUM is ready, i.e. there
 * is a disk in the drive and the tray door is closed.
 */
static int
cdrom_drive_ready(int drive_num)
{
  __dpmi_regs r;

  /* We need to avoid reading any information off the CD-ROM,
     because then the driver will dupe MSCDEX into thinking the
     disk isn't ever changed, and DOS calls get stale info after
     you change the disk.  (We cannot use DOS calls *before* this
     function, because QDPMI will crash the program if we issue
     a DOS call that hits the disk when the drive is empty or the
     disk is an audio disk.)

     So we use the Read Device Status command and look for the
     door open and drive empty bits (seems like door locked bit
     is not universally supported).  This still leaves an unsolved
     problem: an audio disk will be reported as a ready drive, and
     under QDPMI will crash the program when we access the drive.
     I don't see any way out of this mess (anybody?).  Well, at
     least with data disks we don't screw up DOS operations anymore.

     Gosh, why is it always so tricky with Microsoft software??  */

  union
  {
    struct 
    {
      unsigned int size   :16;
      unsigned int command:16;
      unsigned int offset :16;
      unsigned int segment:16;
      unsigned int bytes  :16;
    } buffer_parts;
    unsigned char buffer[0x14];
  } request_header;
  int status;
  unsigned dev_status;

  /* Construct the request header for the CD-ROM device driver.  */
  memset(request_header.buffer, 0, sizeof request_header.buffer);
  request_header.buffer[0] = sizeof request_header.buffer;
  request_header.buffer[2] = 3;     /* IOCTL READ command */
  request_header.buffer_parts.offset  = __tb_offset;
  request_header.buffer_parts.segment = __tb_segment;
  request_header.buffer[0x12] = 5;  /* number of bytes to transfer */

  /* Put control block into the transfer buffer.  */
  _farpokeb(_dos_ds, __tb, 6);	/* read device status */
  _farpokel(_dos_ds, __tb + 1, 0);	/* zero out the result field */

  /* Put request header into the transfer buffer and call the driver.  */
  dosmemput(request_header.buffer, sizeof (request_header.buffer), __tb + 5);
  r.x.ax = 0x1510;
  r.x.cx = drive_num - 1;
  r.x.es = __tb_segment;
  r.x.bx = __tb_offset + 5;
  __dpmi_int(0x2f, &r);
  status = _farpeekw(_dos_ds, __tb + 5 + 3);
  dev_status = _farpeekl(_dos_ds, __tb + 1);
  if (status == 0x100 && _farpeekw (_dos_ds, __tb + 5 + 0x12) == 5
      && (dev_status & 0x801) == 0) /* door open and drive empty bits */
    return 1;
  return 0;
}


/* Exported library functions.  */

FILE *
setmntent(const char *filename, const char *type)
{
  __dpmi_regs r;
  int cds_address_offset;
  /* Need TRUE DOS version. */
  unsigned short true_dos_version = _os_trueversion;

  dos_mem_base = _go32_info_block.selector_for_linear_memory;
  our_mem_base = _my_ds();
  w2k_bug = (_USE_LFN && _os_trueversion == 0x532);

  drive_number = 0;
  skip_drive_b = 0;

  /* Get and save the pointer to the CDS array and the size of
     the array element.  This is version-dependent.  */
  if (true_dos_version < 0x0300 /* CDS not supported for v2.x */
      || true_dos_version > 0x1000) /* dos emulation (OS/2) ? */
    {
      cds_elsize  = -1;
      cds_address = 0;
      cds_address_offset = 0; /* pacify -Wall */
    }
  else if (true_dos_version == 0x0300)
    {
      cds_address_offset = 0x17;
      cds_elsize = 0x51;
    }
  else if (true_dos_version == 0x0532) /* NT */
    {
      cds_address_offset = 0x16;
      cds_elsize = 0x47;
    }
  else
    {
      cds_address_offset = 0x16;
      cds_elsize = (true_dos_version >= 0x0400) ? 0x58 : 0x51;
    }

  if (cds_elsize > 0)
    {
      unsigned long cds_address_ptr;
      
      r.h.ah = 0x52;    /* DOS Get List of Lists call */
      __dpmi_int(0x21, &r);

      /* The pointer to the List of Lists is in ES:BX.  Compute the
         linear address of the pointer to the CDS, which is at version-
         dependent offset from the LoL start.  */
      cds_address_ptr = MK_FOFF(r.x.es, r.x.bx + cds_address_offset);

      /* Get the pointer and compute the linear address of the CDS array.  */
      cds_address = MK_FOFF(_farpeekw(dos_mem_base, cds_address_ptr + 2),
                            _farpeekw(dos_mem_base, cds_address_ptr));
    }

  return (FILE *) 1;
}

struct mntent *
getmntent(FILE *filep)
{
  if (drive_number == -1)
    return NULL;
  if (filep != (FILE *)1)
    {
      errno = EBADF;    /* fake errno for invalid handle */
      return NULL;
    }

  /* The number of drives known to DOS is returned by the DOS
     SetDisk (Int 21h/AH=0Eh) call.  Any drive beyond that
     number can only come from some device driver which bypasses
     the DOS network redirector interface and instead hooks Int 21h
     directly (Novell NetWare 3.x is a good example).  Such drives
     won't appear in the DOS Current Directory Structure (CDS).  */
  cds_drives = setdisk(0xffff);

  /* There may be a maximum of 32 block devices.  Novell Netware indeed
     allows for 32 disks (A-Z plus 6 more characters from '[' to '\'') */
  while (drive_number < 32)
    {
      char          *p, *q;
      char *truename_result;
      struct ffblk   mnt_ff;
      char           cds_path[128];
      unsigned short cds_flags = 0;
      int            drvstr_len;
      int            got_fsname = 0;

      drive_number++;
      mnt_dir[0] = '\0';

      /* On PCs with only one floppy drive, it can be mapped to either
         A: or B:, depending on how it was last referenced.  We will
         skip and not report the drive if it is actually mapped to a
         physical drive which we already reported.  This method both
         insures more accurate report and avoids the annoying message
         ``Insert disk...'' from the OS when _truename() below hits
         the disk.

         I assume there is only one such drive in the system, and that
         it is drive A:, otherwise the logic below will fail miserably.  */
      if (drive_number == 2 && skip_drive_b)
        continue;

      /* Windows 2000 dislikes drives higher than Z */
      if (drive_number > cds_drives && _os_trueversion == 0x532)
        continue;

      /* See whether drive A: is mapped to B: and if it is, raise the
	 flag to skip drive number 2 next time we are called (meaning
	 there is only one floppy drive).  */
      if (drive_number == 1 && (drive_a_mapping = assigned_to(1)) > 0)
	skip_drive_b = 1;

      /* Work around a possible bug in QDPMI: _truename() would sometimes
         crash the calling program when there is no disk in a (floppy)
         drive.  To avoid this, we have to check if the drive is empty
         with a BIOS call.  */
      if (drive_number <= 2)
        {
          unsigned char buf[512];
          int bios_status = 0, count = 0;
	  int drive_a_remapped = drive_a_mapping == 2;

	  /* When biosdisk is called, Windows 9X pops up the ugly
	     "Insert a Disk for Drive A:" prompt (and messes the
	     color palette while at that) if the single physical
	     floppy drive is currently mapped to drive B:.  So we
	     momentarily remap the drive back to A:, just for the
	     duration of the BIOS calls, to avoid that lossage.  */
	  if (drive_a_remapped)
	    assign_floppy_to (1);

          /* Int 13h/AH=02h returns 6 for disk changed, even if the
             disk isn't readable (e.g., unformatted).  Retry the
             operation after disk change, each time resetting the 
             drive, until something other than 6 is returned or we
             run out of our patience.  */
          while (++count < 10 && (bios_status =
                 biosdisk(2, drive_number - 1, 0, 0, 1, 1, buf)) == 6)
            biosdisk(0, drive_number - 1, 0, 0, 0, 0, NULL);

	  if (drive_a_remapped)
	    assign_floppy_to (2);

          /* If the loop ends with nonzero status, fail.  */
          if (bios_status != 0)
            continue;

	  /* If there's only one floppy drive, change DRIVE_NUMBER on
	     the fly to whatever logical drive it is currently mapped.  */
	  if (skip_drive_b)
	    drive_number = drive_a_mapping;
        }

      drvstr_len = sprintf(drive_string, "%c:\\", '@' + drive_number);
      mnt_type = NAME_unknown;

      /* For the ``File System Name'' we use one of the following:

         * X:\DBLSPACE.NNN or X:\STACVOL.NNN for drives compressed with
	   DblSpace or Stacker, where X: is the host drive of the
	   compressed volume and NNN is the volume sequence number;
         * The full filename of the compressed volume file for 
           a drive that is compressed with Jam;
         * What _truename() returns for the root directory, in case
           it isn't the usual ``X:\'';
         * The name of the volume label;
         * The string ``Drive X:'' (where X is the drive letter).

         These are used in the above order.  */

      /* See what 2160 can tell us.  If it fails, there ain't no such
         drive, as far as DOS is concerned.  Some DOS clones, like NT,
	 don't always upcase the drive letter, so we must do that here.  */
      truename_result = _truename(drive_string, mnt_fsname);
      if (truename_result && mnt_fsname[0]
	  && mnt_fsname[1] == ':' && islower((unsigned char)mnt_fsname[0]))
	mnt_fsname[0] = toupper((unsigned char)mnt_fsname[0]);

      /* Get some info from the DOS Current Directory Structure (CDS).
	 We've already hit the disk with _truename(), so CDS now
	 contains valid and up to date data.  Don't look at the CDS
	 for remote drives: they can't be JOINed anyway, and some
	 DOS clones, such as NT, don't put them into the CDS.  */
      if (drive_number <= cds_drives
	  && !_is_remote_drive (drive_number - 1))
        cds_flags = get_cds_entry(drive_number, cds_path);

      if (truename_result != NULL)
        {

          /* Determine the type of this drive, if not known already.

             We use one of the following types:

             fd     for floppy disks
             hd     for hard disks
             dblsp  for disks compressed with DoubleSpace
             stac   for disks compressed with Stacker
             jam    for disks compressed with Jam
             cdrom  for CD-ROM drives
             ram    for RAM disks
             subst  for SUBSTed directories
             join   for JOINed disks
             net    for networked drives
          */
          if (mnt_type[0] == '?')
            {
              int disk_type = _media_type(drive_number);

              if (_is_ram_drive(drive_number))
                mnt_type = NAME_ram;
              else if (_is_cdrom_drive(drive_number))
		{
		  /* Empty CD-ROM drives do NOT fail _truename(),
		     so we must see if there is a disk in the drive.  */
		  if (cdrom_drive_ready(drive_number))
		    mnt_type = NAME_cdrom;
		  else
		    continue;
		}
              /* _is_remote_drive() needs zero-based disk number  */
              else if (_is_remote_drive(drive_number - 1) == 1)
                mnt_type = NAME_net;
              else if (disk_type == REMOVABLE)
                mnt_type = NAME_fd;
              else if (disk_type == FIXED)
                mnt_type = NAME_hd;
            }

          /* If the canonicalized name isn't the boring ``X:\'' with X
             the original drive letter, then it is either in the UNC
             form (``\\MACHINE\PATH\'') or it's SUBSTed or JOINed drive,
             and the canonicalized name might actually say something
             interesting about this drive, so use it.  */
          if (mnt_fsname[1] != ':')
            got_fsname = 1;
          else if (mnt_fsname[0] != drive_string[0])
            {
              /* Detect SUBSTed drives.  This could also be found by
                 looking in the CDS, but we try to keep usage of
                 undocumented features at the bare minimum.
                 For SUBSTed drives, the root directory will be returned
                 by _truename() as some directory on another drive.  */
              mnt_type = NAME_subst;
              got_fsname = 1;
            }
          else
            {
              /* Check for compressed drives (DoubleSpace, Stacker,
		 or Jam).  */
              got_fsname = get_doublespace_info(drive_number)
			   || get_stacker_info(drive_number)
			   || get_jam_info(drive_number);
            }
        }
      /* JOINed drives fail _truename().  I don't know how to check
         for them without using the CDS.  We will only trust the CDS
         data if bits 14 and 15 aren't both zero, and if the current
         directory path in the CDS isn't empty (this might happen
         after ``JOIN D: /d'').  */
      else if ((cds_flags & CDS_VALID) &&
               (cds_flags & CDS_JOIN)  &&
               cds_path[0] != '\0')
        {
          mnt_type = NAME_join;
          strcpy(drive_string, cds_path);
          drvstr_len = strlen(drive_string);
          strcat(drive_string, "\\");

          /* Don't set got_fsname, so that we get a chance to look for
             a volume label below.  */
        }
      else
        continue;   /* illegal (non-present) drive */

      /* Some network redirectors don't set a UNC path to be
         returned by _truename().  See if 215F02 can help.  */
      if ((!got_fsname || mnt_fsname[0] != '\\') && mnt_type == NAME_net)
        if (get_netredir_entry(drive_number))
          got_fsname = 1;
      /* MSCDEX makes `_truename' return a dull "\\X.\A.", so
	 try to get the label anyway.  */
      if (!got_fsname || mnt_type == NAME_cdrom)
        {
          /* Look for the volume label.  */
          int e = errno;
          int volume_found = 0;

          strcat(drive_string, "*.*");
          errno = 0;
          /* Windows 2000/XP can't find labels with LFN=y, so help it */
          if (w2k_bug)
            putenv(unconst("LFN=n",char *));
          volume_found = findfirst(drive_string, &mnt_ff, FA_LABEL) == 0;
          /* Floppies and other disks written by Windows 9X include
             entries that have volume label bit set, but they are
             actually parts of some LFN entry.  We only accept volume
             labels which have their HS bits reset; otherwise we
             pretend we never saw that label.  */
          while (volume_found &&
		 (mnt_ff.ff_attrib & (FA_HIDDEN|FA_SYSTEM)) != 0)
            {
              volume_found = 0;
              errno = ENMFILE;
	      volume_found = findnext(&mnt_ff) == 0;
            }
          if (w2k_bug)
            putenv(unconst("LFN=y",char *));

          if (volume_found)
            {
              errno = e;

              /* Got label.  Strip out extraneous '.' separator, if present. */
              strcpy(mnt_fsname, mnt_ff.ff_name);
              if (strlen(mnt_fsname) > 8 && mnt_fsname[8] == '.')
                {
                  /* Overlapping copy, don't use strcpy() */
                  p = mnt_fsname + 8;
                  q = p + 1;
                  while ((*p++ = *q++) != '\0');
                }

              got_fsname = 1;
            }
	  /* CD-ROM without a label is taken as an empty CD drive or an
	     audio disk, and not reported.  This is because MSCDEX doesn't
	     fail `_truename' for these cases and some DOS clones, such as
	     NT, don't even emulate the drive empty bit reliably.
	     I have never seen a CD-ROM without a label.  Anybody?  */
	  else if (mnt_type == NAME_cdrom)
	    got_fsname = 0;
          else if (errno == ENMFILE || errno == ENOENT)
            {
	      /* Some device drivers for removable media (like JAZ on NT DOS
		 box) pass all above tests when the drive is empty.  Force
		 them to hit the disk with DOS Get Free Disk Space function,
		 and if that fails, treat that drive as non-existing.  */
	      if (mnt_type == NAME_fd)
		{
		  __dpmi_regs r;

		  r.h.ah = 0x36;
		  r.h.dl = drive_number;
		  __dpmi_int(0x21, &r);
		  if ( (r.x.ax & 0xffff) == 0xffff ) /* aha! an impostor! */
		    continue;
		}
              /* Valid drive, but no label.  Construct default
                 filesystem name.  If drive A: is mapped to B:,
                 call it ``Drive A:''.  */
              (void) strcpy(mnt_fsname, "Drive  :");
              mnt_fsname[6] = (drive_number == 2 && drive_a_mapping == 2)
                              ? 'A'
                              : '@' + drive_number;
              got_fsname = 1;
            }
        }

      if (got_fsname)
        {
          char xdrive[3];

          /* Remove the '*.*' from the drive string for use as mnt_dir */
          drive_string[drvstr_len] = '\0';
          if (mnt_dir[0] == '\0')
            strcpy(mnt_dir, drive_string);

          /* Format mnt_dir[] for beauty.  */
          for (p = mnt_dir; *p; p++)
            {
              if (*p == '\\')
                *p = '/';
              else
                *p = tolower((unsigned char)*p);
            }

          /* Should we convert ``\\HOST\PATH'' into ``HOST:PATH''?  */
          mntent.mnt_fsname = mnt_fsname;
          mntent.mnt_dir    = mnt_dir;
          mntent.mnt_type   = mnt_type;
          mntent.mnt_opts   = dev_opts;

          /* Make CD-ROM drives read-only, others read-write.  */
          if (mnt_type == NAME_cdrom)
            dev_opts[1] = 'o';
          else
            dev_opts[1] = 'w';

          /* Include "dev=XX" in the mnt_opts field, where XX should
             be consistent with what stat() returns in st_dev.  */
          sprintf(xdrive, "%02x", mnt_type != NAME_subst
                                  ? drive_number - 1
                                  : mnt_fsname[0] - 'A');
          dev_opts[7] = xdrive[0];
          dev_opts[8] = xdrive[1];
          mntent.mnt_freq   = -1;
          mntent.mnt_passno = -1;
          mntent.mnt_time   = -1;

          return &mntent;
        }

      /* Go try next drive, if any left.  */
    }
  
  return NULL;
}

int
addmntent(FILE *filep, const struct mntent *mnt)
{
  return 1;
}

char *
hasmntopt(const struct mntent *mnt, const char *opt)
{
  return strstr(mnt->mnt_opts, opt);
}

int
endmntent(FILE *filep)
{
  if (filep != (FILE *)1)
    {
      errno = EBADF;    /* fake errno for invalid handle */
      return 0;
    }
  drive_number = 0;
  skip_drive_b = 0;
  return 1;
}

#ifdef  TEST

#if defined(MNT_MNTTAB)
#define MNTTAB_FILE MNT_MNTTAB
#elif defined(MNTTABNAME)
#define MNTTAB_FILE MNTTABNAME
#else
#define MNTTAB_FILE "/etc/mnttab"
#endif

int main(void)
{
  FILE *mp = setmntent(MNTTAB_FILE, "r");
  struct mntent *me;
  int i = 0;

  if (!mp)
    {
      fprintf(stderr, "setmntent() failed\n");
      return 1;
    }

  while ((me = getmntent(mp)) != NULL)
    printf("%d:  %-35s %s\t%s\t%s\n", i++,
           me->mnt_fsname, me->mnt_type, me->mnt_opts, me->mnt_dir);

  return 0;
}

#endif


