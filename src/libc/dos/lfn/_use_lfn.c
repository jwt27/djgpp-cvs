/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <crt0.h>
#include <sys/movedata.h>
#include <libc/bss.h>
#include <libc/environ.h>
#include <libc/farptrgs.h>
#include <libc/dosio.h>

static int      use_lfn_bss_count = -1;	/* if we are restarted (emacs) */
static unsigned filesystem_flags  = _FILESYS_UNKNOWN;
static char _lfnenv = 'y'; /* remember here $(LFN) */
static unsigned last_env_changed = 0;
static int	last_drive; /* drive *letter*, not *number*! */

/* Return the parameters of the filesystem where PATH resides.  */
unsigned
_get_volume_info (const char *path, int *maxfile, int *maxpath, char *fsystype)
{
  __dpmi_regs r;
  unsigned long tbuf_la  = __tb;
  unsigned long tbuf_seg = tbuf_la >> 4;
  unsigned	retval;

  if (path && *path)
  {
    _put_path(path);
    _farsetsel(_dos_ds);
    if (_farnspeekb(tbuf_la + 1) == ':')
    {
      tbuf_la += 3;
      if (_farnspeekb(tbuf_la - 1) != '\\')
	_farnspokeb(tbuf_la - 1, '\\');
      _farnspokeb(tbuf_la++, '\0');
    }
    else if (_farnspeekb(tbuf_la) == '\\' && _farnspeekb(tbuf_la + 1) == '\\')
    {
      /* FIXME: what should we do with the UNC pathnames like
	 "\\machine\vol\path"?  We need to know either their
	 DOS drive letter or where does the root directory path
	 ends.  For now, we assume the entire path is the root path.  */
      for (tbuf_la += 2; _farnspeekb(tbuf_la) != 0; tbuf_la++)
	;
      tbuf_la++;
    }
  }

  /* No explicit drive, use default drive.  */
  if (tbuf_la == __tb)
  {
    unsigned drv_no;

    /* FIXME: can `_dos_getdrive' fail (e.g. no floppy in drive)?  */
    _dos_getdrive(&drv_no);
    _farpokeb(_dos_ds, tbuf_la++, 'A' + drv_no - 1);
    _farpokeb(_dos_ds, tbuf_la++, ':');
    _farpokeb(_dos_ds, tbuf_la++, '\\');
    _farpokeb(_dos_ds, tbuf_la++, '\0');
  }

  r.x.flags = 1;	/* Always set CF before calling a 0x71NN function. */
  r.x.ax = 0x71a0;	/* Get Volume Information function */
  r.x.ds = tbuf_seg;	/* DS:DX points to root directory name */
  r.x.dx = 0;
  r.x.es = tbuf_seg;	/* ES:DI points to a buffer for filesys name */
  r.x.di = (tbuf_la - __tb) & 0xffff;
  r.x.cx = 32;		/* max size of filesystem name (Interrupt List) */
  __dpmi_int(0x21, &r);

  if (!(r.x.flags & 1) && (r.x.ax != 0x7100))
  {
    char *p = fsystype, c;

    retval   = r.x.bx;
    if (maxfile)
      *maxfile = r.x.cx;
    if (maxpath)
      *maxpath = r.x.dx;

    if (fsystype)
    {
      /* Only copy as much as required, in case the
	 buffer isn't large enough for 32 bytes.  */
      _farsetsel (_dos_ds);
      while ((c = _farnspeekb (tbuf_la++)))
	*p++ = c;
      *p = '\0';
    }
  }
  else
  {
    if (r.h.ah == 0x71)
    {
      errno = ENOSYS;
      retval = 0;	/* meaning none of the features supported */
    }
    else
    {
      /* If the call failed, but not because the OS doesn't support
	 the function (e.g., invalid drive), don't disable LFN.  */
      errno = __doserr_to_errno(r.x.ax);
      retval = _FILESYS_UNKNOWN;
    }
    if (maxfile)
      *maxfile = 13;
    if (maxpath)
      *maxpath = 80;
    if (fsystype)
      *fsystype = '\0';
  }

  return retval;
}

char
_use_lfn (const char *path)
{
  int same_drive_as_last_time;
  char old_lfn_flag = _lfnenv;

  if (_crt0_startup_flags & _CRT0_FLAG_NO_LFN)
  {
    /* Don't update the static counters, so the first time
       after NO_LFN flag is reset, the environment and the
       filesystem will be queried.  */
    return 0;
  }

  /* Forget everything we knew before we were dumped (Emacs).  */
  if (use_lfn_bss_count != __bss_count)
    {
      use_lfn_bss_count = __bss_count;
      filesystem_flags = _FILESYS_UNKNOWN;
      _lfnenv = 'y';
      last_drive = 0;
    }

  same_drive_as_last_time = 1;
  if (path && *path)
  {
    _put_path(path);
    /* FIXME: a UNC PATH will always force a call to `_get_volume_info'.  */
    if ((_farpeekb(_dos_ds, __tb + 1) == ':'
	 && toupper (_farpeekb(_dos_ds, __tb)) != last_drive)
	|| (*path == '\\' && path[1] == '\\'))
      same_drive_as_last_time = 0;
    else
    {
      unsigned drv_no;

      _dos_getdrive(&drv_no);
      if ((int)drv_no - 1 + 'A' != last_drive)
	same_drive_as_last_time = 0;
    }
  }

  if (!same_drive_as_last_time
      || last_env_changed  != __environ_changed
      || filesystem_flags  == _FILESYS_UNKNOWN)
  {
    /* check now the environment for $(LFN) */
    char *lfnenv;

    last_env_changed  = __environ_changed;

    lfnenv = getenv ("LFN");
    if(lfnenv && (tolower ((unsigned char)lfnenv[0]) == 'n'))
    {
      _lfnenv = 'n';
      last_drive = 0;
    }
    else
    {
      /* if $(LFN) was not set or != 'n' assume it as 'y' */
      _lfnenv = 'y';
    }
  }

  if (!same_drive_as_last_time || filesystem_flags == _FILESYS_UNKNOWN)
    filesystem_flags = _get_volume_info (path, 0, 0, 0);

  /* If _get_volume_info failed (e.g., an invalid drive letter),
     leave the previous LFN setting alone.  */
  if (filesystem_flags == _FILESYS_UNKNOWN)
  {
    _lfnenv = old_lfn_flag;
    last_drive = 0;
    return _lfnenv != 'n';
  }

          /* Does the filesystem LFN support ? */
  return ((filesystem_flags & _FILESYS_LFN_SUPPORTED) != 0 &&
          /* Is it not disabled by the user ? */
          _lfnenv != 'n');
}

#ifdef TEST

int main (int argc, char *argv[])
{
  char *path = ".";
  unsigned flags;
  char fsname[32];
  int  maxfile, maxpath;

  printf ("_USE_LFN reports %d at startup\n", _USE_LFN);
  if (argc > 1)
    path = argv[1];
  flags = _get_volume_info (path, &maxfile, &maxpath, fsname);
  printf ("Flags: %x, MaxFile: %d, MaxPath: %d, FSName: %s\n",
	  flags, maxfile, maxpath, fsname);
  _crt0_startup_flags |= _CRT0_FLAG_NO_LFN;
  printf ("_USE_LFN reports %d when _CRT0_FLAG_NO_LFN is set\n", _USE_LFN);
  _crt0_startup_flags &= ~_CRT0_FLAG_NO_LFN;
  putenv ("LFN=y");
  printf ("_USE_LFN reports %d when LFN is set to Y\n", _USE_LFN);
  putenv ("LFN=n");
  printf ("_USE_LFN reports %d when LFN is set to N\n", _USE_LFN);
  return 0;
}

#endif
