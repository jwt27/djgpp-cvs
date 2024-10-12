/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2004 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <process.h>

//extern char *const *_environ;

int execvp(const char *path, char * const argv[])
{
  return spawnvpe(P_OVERLAY, path, argv, _environ);
}
