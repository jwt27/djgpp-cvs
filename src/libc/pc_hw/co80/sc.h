/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <dpmi.h>
#include <go32.h>
#include <libc/farptrgs.h>
#include <sys/segments.h>
#include <pc.h>

#define dossel _go32_info_block.selector_for_linear_memory
#define co80 _go32_info_block.linear_address_of_primary_screen
