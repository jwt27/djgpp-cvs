#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <go32.h>
#include <dpmi.h>

void show(const char *m)
{
  __dpmi_regs r;
  r.x.sp = r.x.ss = r.x.flags = 0;
  r.x.ax = 0x5800;
  __dpmi_simulate_real_mode_interrupt(0x21, &r);
/*  printf("[%s: %02x] ", m, r.x.ax & 0xff);*/
}

int
main(int argc, char **argv)
{
  int rv;
  int count = 10;
  int base=0;
  int nbase=_go32_info_block.linear_address_of_transfer_buffer;

  if (argc > 1)
    count = atoi(argv[1]);
  if (argc > 2)
    base = atoi(argv[2]);

  printf("physical address = 0x%05x", nbase);
  if (base)
    printf(", using %05x bytes", nbase-base);
  printf(", transfer buffer at 0x%05lx",
	 _go32_info_block.linear_address_of_transfer_buffer);
    
  putchar('\n');
  show("before");

  if (--count)
  {
#if 1
    char cline[50], nbline[50];
    sprintf(cline, "%d", count);
    sprintf(nbline, "%d", nbase);
    rv = spawnl(P_WAIT, argv[0], argv[0], cline, nbline, 0);
    if (rv == -1)
      perror(argv[0]);
#else
    char cmd[100];
    sprintf(cmd, "%s %d %d", argv[0], count, nbase);
    rv = system(cmd);
#endif
    printf(" - returns %d\n", rv);
  }
  show("after");
  return 0;
}
