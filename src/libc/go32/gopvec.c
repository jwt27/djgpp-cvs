/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dpmi.h>

int _go32_dpmi_get_protected_mode_interrupt_vector(int vector, _go32_dpmi_seginfo *info)
{
  return __dpmi_get_protected_mode_interrupt_vector(vector, (__dpmi_paddr *)&info->pm_offset);
}

int _go32_dpmi_set_protected_mode_interrupt_vector(int vector, _go32_dpmi_seginfo *info)
{
  return __dpmi_set_protected_mode_interrupt_vector(vector, (__dpmi_paddr *)&info->pm_offset);
}
