/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdint.h>
#include "sc.h"

void	
ScreenUpdate(const void *_virtual_screen)
{
  movedata(_my_ds(), (uintptr_t)_virtual_screen,
	   dossel, co80,
	   ScreenRows() * ScreenCols() * 2);
}
