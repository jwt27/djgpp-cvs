#include <stdio.h>
#include <sys/vfs.h>

int
main(void)
{
  int i;
  char dl[3];
  dl[1] = ':';
  dl[2] = 0;
  for (i=2; i<26; i++)
  {
    struct statfs s;
    dl[0] = i+'a';
    printf("%d: ", i);
    fflush(stdout);
    if (statfs(dl, &s) == -1)
      printf("drive %s invalid\n", dl);
    else
      printf("drive %s type=%ld bsize=%ld blocks=%ld bfree=%ld avail=%ld files=%ld ffree=%ld\n",
	     dl, s.f_type, s.f_bsize, s.f_blocks, s.f_bfree, s.f_bavail, s.f_files, s.f_ffree);
  }
  return 0;
}
