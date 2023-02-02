/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdint.h>
#include <libc/stubs.h>
#include <sys/movedata.h>

void movedata(unsigned _source_selector, unsigned _source_offset,
	       unsigned _dest_selector, unsigned _dest_offset,
	       uint32_t _length)
{
  __movedata(_source_selector,
	    _source_offset,
	    _dest_selector,
	    _dest_offset,
	    _length);
}
