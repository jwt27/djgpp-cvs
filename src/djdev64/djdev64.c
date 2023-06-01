#include <dlfcn.h>
#include <stdio.h>
#include "djdev64.h"

int djdev64_open(const char *path)
{
  void *dlh = dlopen(path, RTLD_LOCAL | RTLD_NOW);
printf("DJ64 called, dlhandle %p on %s\n", dlh, path);
  if (!dlh)
    fprintf(stderr, "cannot dlopen %s: %s\n", path, dlerror());
void *dmg = dlsym(dlh, "dosmemget");
fprintf(stderr, "dosmemget at %p\n", dmg);
  return 0;
}

int djdev64_call(int handle, int libid, int fn, unsigned char *sp)
{
    // TODO
    return 0;
}

void djdev64_close(int handle)
{
    // TODO
}
