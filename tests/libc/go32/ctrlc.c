#include <stdio.h>
#include <unistd.h>
#include <dpmi.h>
#include <go32.h>

extern short __djgpp_ds_alias;

void
psel(int s)
{
  unsigned long base;
  __dpmi_get_segment_base_address(s, &base);
  printf("selector 0x%04x : base=0x%lx, limit=0x%lx, flags=0x%x\n",
	 s, base, __dpmi_get_segment_limit(s),
	 __dpmi_get_descriptor_access_rights(s));
}

int
main(void)
{
  printf("cs = %x, ds = %x, dsa = %x\n", _my_cs(), _my_ds(), __djgpp_ds_alias);
  psel(_my_cs());
  psel(_my_ds());
  psel(__djgpp_ds_alias);
  asm("pushl ___djgpp_ds_alias; popl %ds");
  while (1)
  {
    int i, j;
    for (i=0; i<70; i++)
    {
      for (j=0; j<100; j++)
	write(1, "X\b", 2);
      write(1, "X\bX", 3);
    }
    for (i=0; i<70; i++)
    {
      write(1, "\b-\b", 3);
      for (j=0; j<100; j++)
	write(1, "-\b", 2);
    }
  }
}
