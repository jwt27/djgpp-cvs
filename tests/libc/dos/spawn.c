#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <go32.h>
#include <sys/farptr.h>
#include <dpmi.h>

extern char **environ;

void show(const char *m)
{
  __dpmi_regs r;
  r.x.sp = r.x.ss = r.x.flags = 0;
  r.x.ax = 0x5800;
  __dpmi_simulate_real_mode_interrupt(0x21, &r);
  printf("%05lx %s: %02x\n", _go32_info_block.linear_address_of_transfer_buffer, m, r.x.ax & 0xff);
}

int
main(int argc, char **argv)
{
  unsigned long i;
  int count=1;
  if (argv[1][0] == '-')
  {
    count = atoi(argv[1]+1);
    argc--;
    argv++;
  }
  show("before");
  _farsetsel(_go32_info_block.selector_for_linear_memory);
  for (i=0; i<_go32_info_block.size_of_transfer_buffer; i++)
    _farnspokeb(_go32_info_block.linear_address_of_transfer_buffer+i,
		0x11);
  for (i=0; i<count; i++)
  {
    int r = spawnlp(P_WAIT, argv[1], argv[1], argv[2], argv[3], argv[4], 0);
    printf("r = %d\n", r);
    perror(argv[1]);
  }
  show("after");
  return 0;
}
