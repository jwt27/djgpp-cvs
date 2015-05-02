/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2004 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <process.h>

extern char **_environ;

int spawnvp(int mode, const char *path, char *const argv[])
{
  return spawnvpe(mode, path, (char * const *)argv, _environ);
}
