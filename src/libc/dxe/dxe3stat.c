/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 Borca Daniel <dborca@yahoo.com>
 * Copyright (C) 2000 Andrew Zabolotny <bit@eltech.ru>
 * Partly based on work by Charles Sandmann and DJ Delorie.
 * Usage of this library is not restricted in any way.  
 * ABSOLUTELY no warranties.  Contributed to the DJGPP project.
 */

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dxe.h>
#include <unistd.h>

#ifdef __DJGPP__
#include <io.h>
#define CRLF "\r\n"
#define OUTPUT(s)	_write (STDERR_FILENO, s, strlen (s))
#else
#define CRLF "\n"
#define OUTPUT(s)	write (STDERR_FILENO, s, strlen (s))
#endif

static void _dlerrstatmod (const char *module)
{
  const char *err;
  OUTPUT (module);
  OUTPUT (": module loading failed (");
  err = dlerror ();
  OUTPUT (err);
  OUTPUT (")" CRLF);
  abort ();
}

static void _dlerrstatsym (const char *module, const char *symbol)
{
  OUTPUT (module);
  OUTPUT (": module does not contain required symbol `");
  OUTPUT (symbol);
  OUTPUT ("'" CRLF);
  abort ();
}

void (*dlerrstatmod) (const char *module) = _dlerrstatmod;
void (*dlerrstatsym) (const char *module, const char *symbol) = _dlerrstatsym;

void dlstatbind (const char *module, void **handle, char *stubs, char *syms)
{
  if (*handle) return;
  *handle = dlopen (module, RTLD_GLOBAL);
  if (!*handle) dlerrstatmod (module);
  while (*syms)
  {
    long symoff = (long)(long *)dlsym (*handle, syms);
    if (!symoff) dlerrstatsym (module, syms);
#if 1
    *(unsigned char *)stubs = 0xe9;	/* jmp long */
    *(long *)(stubs + 1) = symoff - (long)stubs - 5;
#else
    /* [dBorca]
     * This should allow variable access through indirect
     * pointing... if we only could tell functions from variables!
     */
    *(long *)(stubs) = symoff;
#endif
    stubs += 8;
    syms = strchr (syms, 0) + 1;
  }
}

void dlstatunbind (const char *module, void **handle, char *stubs, char *syms,
  long loader)
{
  (void)module; /* silence compiler warning */

  if (!*handle) return;
  while (*syms)
  {
    *(unsigned char *)stubs = 0xe8;	/* call */
    *(long *)(stubs + 1) = loader - (long)stubs - 5;
    stubs += 8;
    syms = strchr (syms, 0) + 1;
  }
  dlclose (*handle);
  *handle = NULL;
}
