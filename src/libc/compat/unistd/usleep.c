/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <time.h>
#include <dpmi.h>

unsigned int
usleep(unsigned int _useconds)
{
  clock_t cl_time;
  clock_t start_time = clock();

  /* 977 * 1024 is about 1e6.  The funny logic keeps the math from
     overflowing for large _useconds */
  _useconds >>= 10;
  cl_time = _useconds * CLOCKS_PER_SEC / 977;

  while (1)
  {
    clock_t elapsed = clock() - start_time;
    if (elapsed >= cl_time)
      break;
    __dpmi_yield();
  }
  return 0;
}
