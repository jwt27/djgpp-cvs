/* Copyright (c) 1995-98 Eli Zaretskii <eliz@is.elta.co.il> */

#include <dpmi.h>
#include <go32.h>
#include <libc/farptrgs.h>
#include <dos.h>

/* Macro to convert a segment and an offset to a "far offset" suitable
   for _farxxx() functions of DJGPP.  */
#ifndef MK_FOFF
#define MK_FOFF(s,o) ((int)((((unsigned long)(unsigned short)(s)) << 4) + \
                      (unsigned short)(o)))
#endif

/*
Description:

This function checks if drive number @var{drive} (1 == A:, 2 == B:,
etc.) is a RAM disk.  It is done by checking if the number of FAT
copies (in the Device Parameter Block) is 1, which is typical of 
RAM disks.  This doesn't @emph{have} to be so, but if it's good
enough for Andrew Schulman et al (@cite{Undocumented DOS, 2nd
edition}), we can use this as well.  

Return Value: 
1 if the drive is a RAM drive, otherwise 0.  
*/
int
_is_ram_drive(int drive_num)
{
  __dpmi_regs r;

  r.h.ah = 0x32;        /* Get Device Parameter Block function */
  r.h.dl = drive_num;
  __dpmi_int(0x21, &r);

  if (r.h.al == 0)
    {
      /* The pointer to DPB is in DS:BX.  The number of FAT copies is at
         offset 8 in the DPB.  */
      char fat_copies = _farpeekb(_dos_ds, MK_FOFF(r.x.ds, r.x.bx) + 8);

      return fat_copies == 1;
    }
  return 0;
}
