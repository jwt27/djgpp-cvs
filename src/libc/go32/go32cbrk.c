/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <sys/exceptn.h>
#include <go32.h>

void _go32_want_ctrl_break(int yes)	/* Yes means counting */
{
  int ctrl_break_counting = __djgpp_hwint_flags & 2;
  if (yes)
  {
    if (ctrl_break_counting)
      return;
    __djgpp_cbrk_count = 0;
    __djgpp_hwint_flags |= 2;
  }
  else
  {
    if (!ctrl_break_counting)
      return;
    __djgpp_cbrk_count = 0;
    __djgpp_hwint_flags &= ~2;
  }
}

unsigned _go32_was_ctrl_break_hit(void)
{
  unsigned cnt;
  _go32_want_ctrl_break(1);
  cnt = __djgpp_cbrk_count;
  __djgpp_cbrk_count = 0;
  return cnt;
}
