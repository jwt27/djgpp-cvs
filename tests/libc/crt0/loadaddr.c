#include <stdio.h>
#include <go32.h>

int
main(void)
{
  printf("physical address = 0x%05lx\n",
	 _go32_info_block.linear_address_of_transfer_buffer);
  return 0;
}
