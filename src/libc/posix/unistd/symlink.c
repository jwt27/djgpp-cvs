#include <libc/stubs.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <process.h>
#include <dpmi.h>
#include <go32.h>

static char EXE_SUFFIX[] = ".exe";
static char STUBIFY[]	 = "stubify.exe";
static char STUBEDIT[]	 = "stubedit.exe";

/* Return a pointer to the tail of the pathname.  */
static const char *
tail (const char *path)
{
  const char *p = path ? path + strlen (path) - 1 : path;

  if (p)
    {
      while (p > path && *p != '/' && *p != '\\' && *p != ':')
	p--;
      if (p > path)
	p++;
    }
  return p;
}

/* Support the DJGPP ``symlinks'' for .exe files.  */
int
symlink (const char *source, const char *dest)
{
  char src_abs[FILENAME_MAX+5], src_short[FILENAME_MAX+5];
  char dest_abs[FILENAME_MAX+5];
  char *np, ropt[FILENAME_MAX+15]; /* some extra for ``runfile='' */
  const char *src_base, *dest_base;

  _fixpath (source, src_abs);
  _fixpath (dest, dest_abs);
  src_base = tail (src_abs);
  dest_base = tail (dest_abs);

  /* DJGPP symlinks must be in the same directory.  */
  if (strnicmp (src_abs, dest_abs, src_base - src_abs))
    {
      errno = EXDEV;
      return -1;
    }

  /* Any file is already a link to itself.  */
  if (stricmp (src_abs, dest_abs) == 0)
    return 0;

  /* Allow to say `ln -s src dest' when we really
     mean `src.exe' and `dest.exe'  */
  np = src_abs + strlen (src_abs) - 4;
  if (stricmp (np, EXE_SUFFIX))
    strcat (src_abs, EXE_SUFFIX);

  /* Under LFN, we need the short version of the program name, since that
     is what the stub stores (and what a program gets in its argv[0]).  */
  if (_USE_LFN)
    {
      if (__file_exists (src_abs))
	{
	  /* File exists.  Get its 8+3 alias.  */
	  __dpmi_regs r;

	  dosmemput(src_abs, strlen (src_abs)+1, __tb);
	  r.x.ax = 0x7160;		/* Truename */
	  r.x.cx = 1;			/* Get short name */
	  r.x.ds = r.x.es = __tb / 16;
	  r.x.si = r.x.di = __tb & 15;
	  __dpmi_int(0x21, &r);
	  if (r.x.flags & 1 || r.x.ax == 0x7100)
	    /* Shouldn't happen: LFN *is* supported and file *does* exist.  */
	    {
	      errno = EIO;
	      return -1;
	    }
	  dosmemget (__tb, FILENAME_MAX, src_short);
	}
      else
	{
	  /* File doesn't exist.  Generate short name that would be used.
	     FIXME: this will lose if the generated name collides with
	     another file already in that directory; however, the only
	     alternative is to disallow symlinks to non-existing files.  */
	  char *p = strncpy (src_short, src_abs, src_base - src_abs);
	  _lfn_gen_short_fname (src_base, p + (src_base - src_abs));
	}
    }
  else
    strcpy (src_short, src_abs);

  /* Need the basename of SRC_SHORT sans the extension.  */
  strcpy (ropt, "runfile=");
  strcat (ropt, tail (src_short));
  for (np = ropt + strlen (ropt) - 1; np > ropt; np--)
    if (*np == '.')
      {
	*np = '\0';
	break;
      }

  /* `stubedit' needs its argument with the .EXE suffix explicit.  */
  np = dest_abs + strlen (dest_abs) - 4;
  if (stricmp (np, EXE_SUFFIX))
    strcat (dest_abs, EXE_SUFFIX);


  if (spawnlp (P_WAIT, STUBIFY, STUBIFY, "-g", dest_abs, (char *)0)
      || spawnlp (P_WAIT, STUBEDIT, STUBEDIT, dest_abs, ropt, (char *)0))
    return -1;
  return 0;
}
