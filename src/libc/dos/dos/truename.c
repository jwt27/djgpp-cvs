/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * This is file TRUENAME.C
 *
 * Copyright (c) 1994, 1995 Eli Zaretskii <eliz@is.elta.co.il>
 *
 * This software may be used freely so long as this copyright notice is
 * left intact.  There is no warranty on this software.
 *
 */

#include <libc/stubs.h>
#include <libc/dosio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <go32.h>
#include <dpmi.h>
#include <dos.h>
#include <sys/stat.h>

#define MAX_TRUE_NAME FILENAME_MAX

/* Given a pathname in FILE, return it in canonical form:
   letters are uppercased, forward slashes converted to backslashes,
   asterisks converted to appropriate number of of question marks,
   file and directory names are truncated to 8.3 if necessary,
   "." and ".." are resolved, extra slashes removed, SUBSTed, JOINed
   and ASSIGNed drives resolved.  Character devices return as
   "X:/DEVNAME" (note the forward slash!), where X is the CURRENT
   drive and DEVNAME is the device name (e.g. CON).  This is exactly
   what DOS TRUENAME command does.  See Ralph Brown's Interrupt List
   for more details.

   The result is placed in BUF, if that's non-NULL; the buffer should
   be large enough to contain the largest possible pathname (128
   characters).  Otherwise, the space to hold the result will be
   malloc()'ed by the function; it is up to the user to release the
   buffer by calling free().  In any case, the pointer to the result
   is returned.

   In case of any failure, returns a NULL pointer and sets errno.
*/

static char *
_truename_internal(const char *file, char *buf, const int try_lfn)
{
  __dpmi_regs     regs;
  unsigned short  dos_mem_selector = _dos_ds;
  unsigned short  our_mem_selector = _my_ds();
  int             e                = errno;
  unsigned	  use_lfn	   = _USE_LFN;
  int		  first_time	   = 1;

  char true_name[MAX_TRUE_NAME];
  char file_name[MAX_TRUE_NAME], *name_start = file_name, *name_end;

  /* Fail for empty arguments.  */
  if (file == (char *)0)
    {
      errno = EFAULT;
      return (char *)0;
    }
  if (*file == 0)
    {
      errno = ENOTDIR;
      return (char *)0;
    }

  strncpy(file_name, file, MAX_TRUE_NAME);
  file_name[MAX_TRUE_NAME - 1] = '\0';

  /* Int 21h, func 60h works only for DOS 3.x and later (always with djgpp).
     According to ``Undocumented DOS'' (2nd ed., p.150), INT 21H/AH=60H
     doesn't like leading or trailing whitespace.  However, leading
     spaces in filenames will cause most (if not all) other DOS
     functions to fail also, and _truename() doesn't have to be holier
     than the Pope.  Therefore, we only weed out trailing whitespace
     here (although my testing indicates that DOS can handle it itself).
  */
  for (name_end = file_name + strlen(file_name) - 1;
       name_end >= file_name && isspace((unsigned char)*name_end); )
    *name_end-- = '\0';

  /* Transfer Buffer is always long enough to hold true name of the file. */

  /* Copy the original name to DOS buffer. */
  if ( name_start[1] == ':' &&
      (name_start[2] == '\\' || name_start[2] == '/') &&
       name_start[3] == '\0')
    {
      /* Under some versions of NetWare, an error is returned, if
         the input path is "D:\"; one should use "D:\." instead
         (Ralph Brown).  Is there any DOS call which NetWare did
         not screw up???  */

      name_start[3] = '.';
      name_start[4] = '\0';
    }
  _put_path(name_start);

  /* Call DOS INT 21H undocumented function 60h. */
  if (use_lfn)
  {
    regs.x.flags = 1;  /* Always set CF before calling a 0x71NN function. */
    regs.x.ax = 0x7160;
    /* Get Long Path Name (if there is one) and we want it. */
    regs.x.cx = try_lfn ? 2 : 0;
  }
  else
    regs.x.ax = 0x6000;

  /* According to Ralph Brown's Interrupt List, can't make the input
     and output buffers be the same, because it doesn't work for early
     versions of DR-DOS.  */
lfn_retry:
  regs.x.ds = regs.x.es = __tb_segment;
  regs.x.si = __tb_offset;
  regs.x.di = __tb_offset + MAX_TRUE_NAME;
  __dpmi_int(0x21, &regs);

  if (regs.x.ax == 0x7100)
  {
    /*  Never assume that the complete LFN API is implemented,
        so check that AX != 0x7100.  E.G.: MSDOS 6.22 and DOSLFN 0.40.
        If not supported fall back on 0x6000.  */
    use_lfn = 0;
    regs.x.ax = 0x6000;
    goto lfn_retry;
  }

  /* Now get the result from lower memory. */
  movedata(dos_mem_selector, __tb + MAX_TRUE_NAME,
           our_mem_selector, (unsigned int)true_name, MAX_TRUE_NAME);

  if (regs.x.flags & 1)
    {
      if (use_lfn && first_time)
	{
	  /* If the file doesn't exist, 217160/CX=2 fails.  Try again
	     with CX=0, so that this time it won't validate the path.  */
	  first_time = 0;
	  regs.x.ax = 0x7160;
	  regs.x.cx = 0;
	  goto lfn_retry;
	}
      errno = __doserr_to_errno(regs.x.ax);
      return (char *)0;
    }
  else
    {
      if (buf == (char *)0)
        buf = (char *)malloc(strlen(true_name)+1);
      if (buf == (char *)0)
        errno = ENOMEM;
      else
        {
          errno = e;
          strcpy(buf, true_name);
        }
      return buf;
    }
}

char *
_truename(const char *file, char *buf)
{
  return _truename_internal(file, buf, 1);
}

/* Sometimes we want the truename for a file that doesn't exist yet.
   In those cases Windows '98 seems to prefer returning an SFN truename.
   So if you want to use truenames in a comparison, it's safer to use
   the SFN truenames.
 */
char *
_truename_sfn(const char *file, char *buf)
{
  return _truename_internal(file, buf, 0);
}

#ifdef  TEST

/* A short test program.
   Be sure not to name the executable TRUENAME.EXE, because then
   it won't ever be called by COMMAND.COM which has an internal
   command with that name.
*/

#include <stdio.h>

int
main(int argc, char *argv[])
{
  char truebuf[MAX_TRUE_NAME];

  if (argc < 2)
    {
      fprintf(stderr, "Usage: %s filename\n", argv[0]);
      return 0;
    }
  else
    {
      char *s;

      printf("Arg:              \"%s\"\n", argv[1]);
      printf("Int 21h/AX=6000h: \"%s\"\n",
             (s = _truename(argv[1], truebuf))
                ? *s
                  ? s
                  : "(Empty)"
                : "(Null Ptr)");
      if (s == (char *)0 || *s == 0)
        perror("_truename");
    }

  return 0;
}

#endif
