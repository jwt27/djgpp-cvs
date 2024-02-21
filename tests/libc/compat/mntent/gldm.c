#include <stdio.h>
#include <dpmi.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dir.h>

int
main(void)
{
  int i;
  for (i=1; i<=26; i++)
  {
    __dpmi_regs r;

    r.x.ax = 0x440e;
    r.h.bl = i;
    __dpmi_int(0x21, &r);
    if (r.x.flags & 1)
    {
      if (r.x.ax == 15)
	continue;
      printf("%c: error %d", i+'@', r.x.ax);
    }
    else
    {
      if (r.h.al)
	printf("%c: -> %c:", i+'@', r.h.al + '@');
      else
        printf("%c: unshared", i+'@');
    }


    r.x.ax = 0x4408;
    r.h.bl = i;
    __dpmi_int(0x21, &r);
    if (r.x.flags & 1)
      printf(" r/f error");
    else if (r.x.ax & 1)
      printf(" fixed");
    else
      printf(" removable");

    r.x.ax = 0x4409;
    r.h.bl = i;
    __dpmi_int(0x21, &r);
    if (r.x.flags & 1)
      printf(" l/r error");
    else if (r.x.dx & 0x1000)
      printf(" remote");
    else
      printf(" local");

    r.x.ax = 0x150b;
    r.x.cx = i-1;
    __dpmi_int(0x2f, &r);
    if (r.x.bx != 0xadad)
      printf(" mscdex error");
    else if (r.x.ax)
    {
      char path[] = "c:/getmntent.dj";
      int fd;
      printf(" cdrom");

      path[0] = i + '@';
      errno = 0;
      fd = open(path, O_RDONLY);
      if (fd < 0 && errno != ENOENT)
	printf(" empty");
      else
	printf(" present");
      if (fd >= 0)
	close(fd);
    }

    printf("\n");
  }
  return 0;
}
