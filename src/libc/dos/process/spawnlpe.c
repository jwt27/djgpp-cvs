/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <process.h>
#include <libc/dosexec.h>

int spawnlpe(int mode, const char *path, const char *argv0, ... /*, const char **envp */)
{
  scan_ptr();
  return spawnvpe(mode, path, (char * const *)&argv0, (char * const *)ptr);
}
