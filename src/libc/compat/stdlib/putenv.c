/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdlib.h>
#include <string.h>
#include <libc/bss.h>

/*

   This routine assumes that the environ array and all strings it
   points to were malloc'd.  Nobody else should modify the environment
   except crt1.c

*/

extern char **environ;
static int ecount = -1;
static int emax = -1;
static int putenv_bss_count = -1;

int
putenv(const char *val)
{
  int vlen = strlen(val);
  char *epos = strchr(val, '=');
  int nlen = epos - val + 1;
  int eindex;

  if (epos == 0)
    return -1;

  if (putenv_bss_count != __bss_count)
  {
    putenv_bss_count = __bss_count;
    for (ecount=0; environ[ecount]; ecount++);
    emax = ecount;
  }

  for (eindex=0; environ[eindex]; eindex++)
    if (strncmp(environ[eindex], val, nlen) == 0)
    {
      char *oval = environ[eindex];

      if (val[nlen] == 0) /* delete the entry */
      {
	free(oval);
	environ[eindex] = environ[ecount-1];
	environ[ecount-1] = 0;
	ecount--;
	return 0;
      }

      /* change existing entry */
      if (strcmp(environ[eindex]+nlen, val+nlen) == 0)
	return 0; /* they're the same */
      environ[eindex] = (char *)malloc(vlen+1);
      if (environ[eindex] == 0)
      {
	environ[eindex] = oval;
	return -1;
      }
      free(oval);
      strcpy(environ[eindex], val);
      return 0;
    }

  /* delete nonexisting entry? */
  if (val[nlen] == 0)
    return 0;

  /* create new entry */
  if (ecount >= emax)
  {
    char **enew;
    emax += 10;
    enew = (char **)malloc(emax * sizeof(char **));
    if (enew == 0)
      return -1;
    memcpy(enew, environ, ecount * sizeof(char *));
    free(environ);
    environ = enew;
  }

  environ[ecount] = (char *)malloc(vlen+1);
  if (environ[ecount] == 0)
    return -1;
  strcpy(environ[ecount], val);

  ecount++;
  environ[ecount] = 0;

  return 0;
}
