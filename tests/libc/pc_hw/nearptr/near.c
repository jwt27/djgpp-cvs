#include <sys/nearptr.h>
#include <stdio.h>
#include <stdlib.h>
#include <dpmi.h>
#include <pc.h>
#include <go32.h>	/* For ScreenPrimary */
#include <crt0.h>

#ifdef TESTCRT0
int _crt0_startup_flags = _CRT0_FLAG_NEARPTR;
#endif

int
main(void)
{
  short *screen;
  __dpmi_meminfo meminfo;
  ScreenClear();
  ScreenSetCursor(1,0);	/* Room to show direct write on row 0 below */
#ifdef TESTCRT0
  printf("Testing crt0 flags startup:\n");
  if(_crt0_startup_flags & _CRT0_FLAG_NEARPTR) {
#else
  if(__djgpp_nearptr_enable()) {
#endif
    printf("base: 0x%x\n",(unsigned)__djgpp_conventional_base);
    screen = (short *)(__djgpp_conventional_base+ScreenPrimary);
    screen[0] = 0x941;
    screen[1] = 0x0a42;
    screen[2] = 0x0b43;
    screen[3] = 0x0c44;
  } else {
    printf("Near pointers not available\n");
    exit(1);
  }
  /* Test to see if malloc moves us (rare, but happens) */
  malloc(1024*1024);
  printf("base: 0x%x\n",(unsigned)__djgpp_conventional_base);
  screen = (short *)(__djgpp_conventional_base+ScreenPrimary);
  screen[4] = 0x0d45;

  /* Allocating DPMI memory + a malloc should move all except CWSDPMI */
  meminfo.size = 1024*1024;
  __dpmi_allocate_memory(&meminfo);
  malloc(1024*1024);
  printf("base: 0x%x\n",(unsigned)__djgpp_conventional_base);
  screen = (short *)(__djgpp_conventional_base+ScreenPrimary);
  screen[5] = 0x0e21;

  printf("Disable Near pointers ... should get GPF\n");
  fflush(stdout);
  __djgpp_nearptr_disable();
  *(short *)(__djgpp_conventional_base+ScreenPrimary) = 0x961;
  return 0;
}
