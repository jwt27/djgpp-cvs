/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_debug_dbgcom_h_
#define __dj_include_debug_dbgcom_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

#include <setjmp.h>
#include <debug/tss.h>

typedef struct {
  unsigned long app_base;	/* linear base address of application */
  unsigned long dr[8];		/* debug registers, set when a_tss runs */
} ExternalDebuggerInfo;

extern ExternalDebuggerInfo edi;

typedef struct {
  unsigned short sig0;
  unsigned short sig1;
  unsigned short sig2;
  unsigned short sig3;
  unsigned short exponent:15;
  unsigned short sign:1;
} NPXREG;

typedef struct {
  unsigned long control;
  unsigned long status;
  unsigned long tag;
  unsigned long eip;
  unsigned long cs;
  unsigned long dataptr;
  unsigned long datasel;
  NPXREG reg[8];
} NPX;

extern NPX npx;

void save_npx (void); /* Save the FPU of the debugged program */
void load_npx (void); /* Restore the FPU of the debugged program */

void run_child(void);
int read_child(unsigned child_addr, void *buf, unsigned len);
int write_child(unsigned child_addr, void *buf, unsigned len);
void edi_init(jmp_buf start_state);
void cleanup_client(void);

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_debug_dbgcom_h_ */
