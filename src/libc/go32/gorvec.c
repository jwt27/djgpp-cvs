/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dpmi.h>

int _go32_dpmi_get_real_mode_interrupt_vector(int vector, _go32_dpmi_seginfo *info)
{
  return __dpmi_get_real_mode_interrupt_vector(vector, (__dpmi_raddr *)&info->rm_offset);
}

int _go32_dpmi_set_real_mode_interrupt_vector(int vector, _go32_dpmi_seginfo *info)
{
  return __dpmi_set_real_mode_interrupt_vector(vector, (__dpmi_raddr *)&info->rm_offset);
}
