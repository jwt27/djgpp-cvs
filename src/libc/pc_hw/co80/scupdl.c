/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdint.h>
#include "sc.h"

void	
ScreenUpdateLine(const void *_virtual_screen_line, int _row)
{
  movedata(_my_ds(), (uintptr_t)_virtual_screen_line,
	   dossel, co80 + ScreenCols() * 2 * _row,
	   ScreenCols() * 2);
}
