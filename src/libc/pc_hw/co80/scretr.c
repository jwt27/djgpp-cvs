/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdint.h>
#include "sc.h"

void	
ScreenRetrieve(void *_virtual_screen)
{
  movedata(dossel, co80,
	   _my_ds(), (uintptr_t)_virtual_screen,
	   ScreenRows() * ScreenCols() * 2);
}

