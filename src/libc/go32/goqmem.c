/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dpmi.h>

unsigned long _go32_dpmi_remaining_physical_memory(void)
{
  _go32_dpmi_meminfo info;
  _go32_dpmi_get_free_memory_information(&info);
  if (info.available_physical_pages)
    return info.available_physical_pages * 4096;
  return info.available_memory;
}

unsigned long _go32_dpmi_remaining_virtual_memory(void)
{
  _go32_dpmi_meminfo info;
  _go32_dpmi_get_free_memory_information(&info);
  return info.available_memory;
}
