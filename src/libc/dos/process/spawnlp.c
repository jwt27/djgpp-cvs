/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <process.h>

extern char **environ;

int spawnlp(int mode, const char *path, const char *argv0, ...)
{
  return spawnvpe(mode, path, (char * const *)&argv0, (char * const *)environ);
}
