/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <sys/nearptr.h>
#include <sys/exceptn.h>
#include <crt0.h>
#include <dpmi.h>
#include <go32.h>

/* Functions to enable "near" pointer access to DOS memory under DPMI
   CW Sandmann 7-95  NO WARRANTY: WARNING, since these functions disable
   memory protection, they MAY DESTROY EVERYTHING ON YOUR COMPUTER! */

int __djgpp_nearptr_enable(void)
{
  if(!__dpmi_set_segment_limit(_my_ds(), 0xffffffffU)) {
    __dpmi_set_segment_limit(__djgpp_ds_alias, 0xffffffffU);
    _crt0_startup_flags |= _CRT0_FLAG_NEARPTR;
    return 1;
  }
  return 0;
}


void __djgpp_nearptr_disable(void)
{
  _crt0_startup_flags &= ~_CRT0_FLAG_NEARPTR;
  __dpmi_set_segment_limit(_my_ds(), __djgpp_selector_limit);
  __dpmi_set_segment_limit(__djgpp_ds_alias, __djgpp_selector_limit);
}
