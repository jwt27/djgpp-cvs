/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <crt0.h>

__djgpp_sbrk_handle *__djgpp_memory_handle(unsigned address)
{
  __djgpp_sbrk_handle *d, *best;

  best = &__djgpp_memory_handle_list[0];
  for (d = best + 1; d->address; d++)
    if (address >= d->address && d->address > best->address)
      best = d;

  return best;
}
