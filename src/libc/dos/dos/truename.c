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

char *
_truename(const char *file, char *buf)
{
  __dpmi_regs          regs;
  unsigned short       dos_mem_selector = _dos_ds;
  unsigned short       our_mem_selector = _my_ds();
  int                  e                = errno;

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
       name_end >= file_name && isspace(*name_end); )
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
  movedata(our_mem_selector, (unsigned int)name_start,
           dos_mem_selector, __tb, strlen(name_start) + 1);

  /* Call DOS INT 21H undocumented function 60h. */
  if(_USE_LFN) {
    regs.x.ax = 0x7160;
    regs.x.cx = 2;		/* Get Long Path Name */
  } else
    regs.x.ax = 0x6000;

  /* According to Ralph Brown's Interrupt List, can't make the input
     and output buffers be the same, because it doesn't work for early
     versions of DR-DOS.  */
  regs.x.ds = regs.x.es = __tb_segment;
  regs.x.si = __tb_offset;
  regs.x.di = __tb_offset + MAX_TRUE_NAME;
  __dpmi_int(0x21, &regs);

  /* Now get the result from lower memory. */
  movedata(dos_mem_selector, __tb + MAX_TRUE_NAME,
           our_mem_selector, (unsigned int)true_name, MAX_TRUE_NAME);

  if (regs.x.flags & 1)
    {
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
