/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <string.h>
#include <dpmi.h>

unsigned long _go32_dpmi_remaining_physical_memory(void)
{
  union a_union
  {
    _go32_dpmi_meminfo go32info;
    __dpmi_free_mem_info dpmiinfo;
  } info;

  _go32_dpmi_get_free_memory_information(&info.go32info);
  if (info.go32info.available_physical_pages)
    return info.go32info.available_physical_pages * 4096;
  return info.go32info.available_memory;
}

unsigned long _go32_dpmi_remaining_virtual_memory(void)
{
  union a_union
  {
    _go32_dpmi_meminfo go32info;
    __dpmi_free_mem_info dpmiinfo;
  } info;

  _go32_dpmi_get_free_memory_information(&info.go32info);
  return info.go32info.available_memory;
}
