#include <stdio.h>
#include <pc.h>
#include <sys/types.h>
#include <dpmi.h>

volatile int new_tc;

void tic_handler()
{
  new_tc ++;
}

int main()
{
  int old_tc = 0, iter=0;
  _go32_dpmi_seginfo old_handler, new_handler;

  printf("grabbing timer interrupt\n");
  _go32_dpmi_get_protected_mode_interrupt_vector(8, &old_handler);

  new_handler.pm_offset = (int)tic_handler;
  new_handler.pm_selector = _go32_my_cs();
  _go32_dpmi_chain_protected_mode_interrupt_vector(8, &new_handler);

  printf("Trapping timer tics, press any key to stop\n");
  while (!kbhit())
  {
    if (old_tc != new_tc)
    {
      printf("iter = %d,  tics = %d\n", iter, new_tc);
      old_tc = new_tc;
    }
    iter++;
  }
  getkey();

  printf("releasing timer interrupt\n");
  _go32_dpmi_set_protected_mode_interrupt_vector(8, &old_handler);

  return 0;
}
