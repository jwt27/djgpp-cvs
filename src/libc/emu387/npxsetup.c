/* Copyright (C) 1994, 1995 Charles Sandmann (sandmann@clio.rice.edu)
 * FPU setup and emulation hooks for DJGPP V2.0
 * This file maybe freely distributed, no warranty. */

/* Note:  If this file is built with IMBED_EMU387 defined, the application 
   should be linked with -lemu to imbed the code in the image.  This 
   makes it easier to distribute an application with a single file.
   The alternate behavior is to dynamically load the image. */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <signal.h>
#include <setjmp.h>
#include <dpmi.h>
#include <libc/internal.h>
#include <sys/exceptn.h>
#include <float.h>
#include <dos.h> /* for _8087 */
#ifndef IMBED_EMU387
#include <sys/dxe.h>
static int (*_emu_entry)(jmp_buf exc);
#else
int _emu_entry(jmp_buf exc);
#endif

int _8087;

/* crt0.o references __emu387_load_hook just to pull in this object in libemu.a.
   Using -lemu -lc brings in the static objects instead of a dynamic link. */

int __emu387_load_hook;

/* The environment variable 387 can be used to disable a 387 which is present
   (for testing) by setting it to "n".  The presence can be reported to 
   stderr by setting 387 to "q" (query).  If 387 is set to "y", we assume the
   coprocessor is present without checking.
   If a 387 is not present under DPMI, we call a V1.0 DPMI extension to ask 
   that Exception 7 be sent to our process.  If we don't have a 387, we 
   attempt to load the EMU387.DXE and call it from the signal.
 */

static void nofpsig(int sig)
{
  if(_emu_entry(__djgpp_exception_state))
  {
    raise(SIGFPE);
    return;
  }
  longjmp(__djgpp_exception_state, __djgpp_exception_state->__eax);
}

#ifdef RESTORE_FPU
static void restore_DPMI_fpu_state(void)
{
  __dpmi_set_coprocessor_emulation(1);	/* Enable Coprocessor, no exceptions */
}
#endif

extern int _detect_80387(void);

void _npxsetup(char *argv0)
{
  char *cp;
  char have_80387;
#ifdef RESTORE_FPU
  static int veryfirst = 1;
#endif

  cp = getenv("387");
  if (cp && (tolower((unsigned char)cp[0]) == 'y'))
  {
    _8087 = 3;
    _control87(0x033f, 0xffff);	/* mask all numeric exceptions */
    return;
  }
  if (cp && (tolower((unsigned char)cp[0]) == 'n'))
    have_80387 = 0;
  else
  {
    /* This next function may fail, but that's OK.  This may fix the
       nested FPU client fault - DJ */
    __dpmi_set_coprocessor_emulation(1);
    have_80387 = _detect_80387();
  }
  _8087 = (have_80387 ? 3 : 0);

  if (cp && (tolower((unsigned char)cp[0]) == 'q')) {
    if (!have_80387)
      _write(2, "No ", 3);
    _write(2, "80387 detected.\r\n", 17);
  }

  if(have_80387) {
    /* mask all exceptions */
    _control87(0x033f, 0xffff);

  } else {
    /* Flags value 3 means coprocessor emulation, exceptions to us */
    if (__dpmi_set_coprocessor_emulation(3)) {
      _write(2, "Warning: Coprocessor not present and DPMI setup failed!\r\n", 57);
      _write(2, "         If application attempts floating operations system may hang!\r\n", 71);
    } else {
#ifndef IMBED_EMU387
      char emuname[512];
      cp = getenv("EMU387");
      if (!cp) {
        char *last, c;
        cp = last = emuname;
        while((c = *cp++ = *argv0++))
          if(c == '/' || c == '\\')
            last = cp;
        *last = 0;
        strcat(emuname,"emu387.dxe");
        cp = emuname;
      }
      _emu_entry = _dxe_load(cp);
      if (_emu_entry == 0)
        return;
#endif
#ifdef RESTORE_FPU
      if (veryfirst)
	{
	  veryfirst = 0;
	  atexit(restore_DPMI_fpu_state);
	}
#endif
      signal(SIGNOFP, nofpsig);
      /* mask all exceptions for the emulation case, too */
      _control87(0x033f, 0xffff);
    }
  }
}
