/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libc/bss.h>
#include <libc/environ.h>

/*

   This routine assumes that the _environ array and all strings it
   points to were malloc'd.  Nobody else should modify the environment
   except crt1.c

*/

extern char **_environ;
static char **prev_environ = NULL; /* to know when it's safe to call `free' */
static int ecount = -1;
static int emax = -1;
static int putenv_bss_count = -1;


/* This gets incremented every time some variable in the
   environment is added, deleted, or changes its value.
   It is meant to be used by functions that depend on values
   of environment variables, but don't want to call `getenv'
   unnecessarily (example: `__use_lfn').

   Users should compare this variable with their static
   variable whose value is initialized to zero; thus this
   variable begins with 1 so first time users look they will
   call `getenv'.  */
unsigned __environ_changed = 1;

int
putenv(char *val)
{
  int vlen = strlen(val);
  char *epos = strchr(val, '=');
  /* Feature: VAL without a `=' means delete the entry.  GNU `putenv'
     works that way, and `env' from GNU Sh-utils counts on this behavior.  */
  int nlen = epos ? epos - val + 1 : vlen;
  int eindex;
  
  /* Force recomputation of the static counters.

     The second condition of the next if clause covers the case that
     somebody pointed _environ to another array, which invalidates
     `ecount' and `emax'.  This can be used to change the environment
     to something entirely different, or to effectively discard it
     altogether.  GNU `env' from Sh-utils does just that.  */
  if (putenv_bss_count != __bss_count
      || _environ      != prev_environ)
  {
    for (ecount=0; _environ[ecount]; ecount++);
    emax = ecount;
    /* Bump the count to a value no function has yet seen,
       even if they were dumped with us.  */
    __environ_changed++;
    if (putenv_bss_count != __bss_count)
    {
      putenv_bss_count = __bss_count;
      prev_environ = _environ;	/* it's malloced by crt1.c */
    }
  }

  for (eindex=0; _environ[eindex]; eindex++)
    if (strncmp(_environ[eindex], val, nlen) == 0
	&& (epos || _environ[eindex][nlen] == '='))
    {
      char *oval = _environ[eindex];
      int olen = strlen(oval);

      if (val[nlen] == 0 && !epos) /* delete the entry */
      {
	free(oval);
	_environ[eindex] = _environ[ecount-1];
	_environ[ecount-1] = 0;
	ecount--;
	__environ_changed++;
	return 0;
      }

      /* change existing entry */
      if (strcmp(_environ[eindex]+nlen, val+nlen) == 0)
	return 0; /* they're the same */

      /* If new is the same length as old, reuse the same
	 storage.  If new is shorter, call realloc to shrink the
	 allocated block: this causes less memory fragmentation.  */
      if (vlen != olen)
      {
	if (vlen > olen)
	  _environ[eindex] = (char *)malloc(vlen+1);
	else	/* vlen < olen */
	  _environ[eindex] = (char *)realloc(oval, vlen+1);
	if (_environ[eindex] == 0)
	{
	  _environ[eindex] = oval;
	  errno = ENOMEM;
	  return -1;
	}
	if (vlen > olen)
	  free(oval);
      }
      strcpy(_environ[eindex], val);
      __environ_changed++;
      return 0;
    }

  /* delete nonexistant entry? */
  if (val[nlen] == 0 && !epos)
    return 0;

  /* create new entry */
  if (ecount >= emax)
  {
    char **enew;
    emax += 10;
    enew = (char **)malloc((emax+1) * sizeof(char *));
    if (enew == 0)
    {
      errno = ENOMEM;
      return -1;
    }
    memcpy(enew, _environ, ecount * sizeof(char *));
    /* If somebody set _environ to another array, we can't
       safely free it.  Better leak memory than crash.  */
    if (_environ == prev_environ)
      free(_environ);
    _environ = enew;
    prev_environ = _environ;
  }

  _environ[ecount] = (char *)malloc(vlen+1);
  if (_environ[ecount] == 0)
  {
    errno = ENOMEM;
    return -1;
  }
  strcpy(_environ[ecount], val);

  ecount++;
  _environ[ecount] = 0;

  __environ_changed++;

  return 0;
}
