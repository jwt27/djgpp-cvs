/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>
#include "sc.h"

void
ScreenVisualBell(void)
{
  int j = ScreenRows() * ScreenCols();
  int _nblinks = 2;
  _farsetsel(dossel);
  while (_nblinks--)
  {
    unsigned sp = co80;
    int i = j;
    do {
      _farnspokew(sp, _farnspeekw(sp) ^ 0x7f00);
      sp += 2;
    } while (--i);
    delay(100);
  }
}
