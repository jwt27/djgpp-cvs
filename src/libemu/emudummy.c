/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* Dummy routines to enable building of emu387.dxe from libemu.a - since we
   can't do I/O, dummy the I/O routines out.  */
   
void vsprintf(void) {}
void _write(void) {}

int __djgpp_exception_state_ptr;
   
/* The _emu_dummy is just there to bring in __emu_entry from the library. */
/* void _emu_dummy(void) { _emu_entry(); } */
