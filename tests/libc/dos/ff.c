#include <dir.h>
#include <stdio.h>
#include <string.h>

int
main(int argc, char **argv)
{
  struct ffblk ff;
  int done;

  printf("sizeof(ff) = %ld\n", sizeof(ff));
  printf("ff.ff_attrib = %d\n", (int)(&ff.ff_attrib) - (int)(&ff));
  printf("ff.ff_ftime = %d\n", (int)(&ff.ff_ftime) - (int)(&ff));
  printf("ff.ff_fdate = %d\n", (int)(&ff.ff_fdate) - (int)(&ff));
  printf("ff.ff_fsize = %d\n", (int)(&ff.ff_fsize) - (int)(&ff));
  printf("ff.ff_name = %d\n", (int)(&ff.ff_name) - (int)(&ff));
  memset(&ff, 0x11, sizeof(ff));
  done = findfirst(argv[1], &ff, -1);
  if (done) printf("%s: error\n", argv[1]);
  while (!done)
  {
#if 0
    int i;
    unsigned char *cp = (unsigned char *)(&ff);
    for (i=0; i<40; i++)
      printf("%02x ", cp[i]);
    printf("\n");
#else
    printf("%s\n", ff.ff_name);
#endif
    done = findnext(&ff);
  }
  return 0;
}
