/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <time.h>
#include <dpmi.h>

unsigned int
sleep(unsigned int _seconds)
{
  clock_t start_time = clock();
  while (((unsigned)(clock() - start_time)) / CLOCKS_PER_SEC < _seconds)
    __dpmi_yield();
  return 0;
}
