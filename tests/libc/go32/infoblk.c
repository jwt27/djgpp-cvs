#include <stdio.h>
#include <go32.h>
#include <dpmi.h>

static const char *
mname(int m)
{
  switch (m)
  {
    case _GO32_RUN_MODE_UNDEF:
      return "undefined";
    case _GO32_RUN_MODE_RAW:
      return "raw";
    case _GO32_RUN_MODE_XMS:
      return "xms";
    case _GO32_RUN_MODE_VCPI:
      return "vcpi";
    case _GO32_RUN_MODE_DPMI:
      return "dpmi";
    default:
      return "unrecognized";
  }
}

int
main(void)
{
  printf("  size_of_this_structure_in_bytes = 0x%08lx (%ld)\n",
    _go32_info_block.size_of_this_structure_in_bytes,
    _go32_info_block.size_of_this_structure_in_bytes);
  printf("  linear_address_of_primary_screen = 0x%08lx (%ld)\n",
    _go32_info_block.linear_address_of_primary_screen,
    _go32_info_block.linear_address_of_primary_screen);
  printf("  linear_address_of_secondary_screen = 0x%08lx (%ld)\n",
    _go32_info_block.linear_address_of_secondary_screen,
    _go32_info_block.linear_address_of_secondary_screen);
  printf("  linear_address_of_transfer_buffer = 0x%08lx (%ld)\n",
    _go32_info_block.linear_address_of_transfer_buffer,
    _go32_info_block.linear_address_of_transfer_buffer);
  printf("  size_of_transfer_buffer = 0x%08lx (%ld)\n",
    _go32_info_block.size_of_transfer_buffer,
    _go32_info_block.size_of_transfer_buffer);
  printf("  pid = 0x%08lx (%ld)\n",
    _go32_info_block.pid,
    _go32_info_block.pid);
  printf("  master_interrupt_controller_base = 0x%02x (%d)\n",
    _go32_info_block.master_interrupt_controller_base,
    _go32_info_block.master_interrupt_controller_base);
  printf("  slave_interrupt_controller_base = 0x%02x (%d)\n",
    _go32_info_block.slave_interrupt_controller_base,
    _go32_info_block.slave_interrupt_controller_base);
  printf("  selector_for_linear_memory = 0x%08x (%d)\n",
    _go32_info_block.selector_for_linear_memory,
    _go32_info_block.selector_for_linear_memory);
  printf("  linear_address_of_stub_info_structure = 0x%08lx (%ld)\n",
    _go32_info_block.linear_address_of_stub_info_structure,
    _go32_info_block.linear_address_of_stub_info_structure);
  printf("  linear_address_of_original_psp = 0x%08lx (%ld)\n",
    _go32_info_block.linear_address_of_original_psp,
    _go32_info_block.linear_address_of_original_psp);
    
  printf("  run mode is %s (info is 0x%04x (%d.%d))\n",
    mname(_go32_info_block.run_mode),
    _go32_info_block.run_mode_info,
    _go32_info_block.run_mode_info >> 8,
    _go32_info_block.run_mode_info & 0xff);

  return 0;
}
