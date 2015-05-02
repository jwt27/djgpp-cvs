/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2004 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <process.h>
#include <libc/dosexec.h>
#include <libc/unconst.h>

extern char * const *_environ;

int execlp(const char *path, const char *argv0, ...)
{
  return spawnvpe(P_OVERLAY, path, unconst(&argv0,char * const *), _environ);
}
