#include <stdio.h>
#include <mntent.h>

char ns[] = "";

int
main(void)
{
  struct mntent *m;
  FILE *f;
  f = setmntent(ns, ns);
  while ((m = getmntent(f)))
    printf("Drive %s, name %s\n", m->mnt_dir, m->mnt_fsname);
  endmntent(f);
  return 0;
}
