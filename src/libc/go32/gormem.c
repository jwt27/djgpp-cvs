/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dpmi.h>

int _go32_dpmi_allocate_dos_memory(_go32_dpmi_seginfo *info)
{
  int ret_sel;
  int ret_seg;

  ret_seg = __dpmi_allocate_dos_memory(info->size, &ret_sel);
  if(ret_seg == -1) {
    info->size = ret_sel;	/* really max paragraphs */
    info->pm_selector = 0;	/* Fault if they don't check */
    return __dpmi_error;	/* Assume error was no memory */
  }
  info->rm_segment = ret_seg;
  info->pm_selector = ret_sel;
  return 0;
}

int _go32_dpmi_free_dos_memory(_go32_dpmi_seginfo *info)
{
  if(__dpmi_free_dos_memory(info->pm_selector) == -1)
    return __dpmi_error;
  return 0;
}

int _go32_dpmi_resize_dos_memory(_go32_dpmi_seginfo *info)
{
  int ret_max;
  if(__dpmi_resize_dos_memory(info->pm_selector, info->size, &ret_max)) {
    info->size = ret_max;
    return __dpmi_error;
  }
  return 0;
}
