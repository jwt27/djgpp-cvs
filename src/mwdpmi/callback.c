/* ---------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                 */
/* Distributed under the GPL, see COPYING for details.                    */
/* ---------------------------------------------------------------------- */
#include "code32.h"
#include "clients.h"
#include "dpmi.h"
#include "ldt.h"
#include <dpmi.h>

/* ---------------------------------------------------------------------- */
/* Offset for entry point of callback #no.  */

#define CALLBACK_OFS(no) \
   ((word32)(current_client_low->callback_code + (no)) - \
    (word32)current_client_low)
/* ---------------------------------------------------------------------- */
/* Extremely temporary stack used for call-backs.  */

struct callback_stack
{
  word8 spare[128];
  word32 edi, esi, ebp, dummy, ebx, edx, ecx, eax;
  word32 esp, ss, gs, fs, es, ds, eip, cs, eflags;
} callback_stack;
/* ---------------------------------------------------------------------- */
static void
callback_handler_part2 (int no, __dpmi_regs *data)
{
  current_client->on_locked_stack++;

#ifdef DEBUG
  if (DEBUG_TEST (DEBUG_CALLBACK))
    eprintf ("Callback %04x:%04x triggered.\r\n",
	     current_client_handle,
	     CALLBACK_OFS (no));
#endif
	
  while (1);
}
/* ---------------------------------------------------------------------- */
void
callback_handler (void)
{
  /* CAUTION: we are on small temporary stack.  */
  __dpmi_regs *data;
  void *stack =
    LINEAR_TO_PTR ((callback_stack.ss << 4) + (callback_stack.esp & 0xffff));
  int no;
  word16 cs, ip;

  /* Pop the callback number.  */
  no = *((word8 *)stack);
  callback_stack.esp += 2, stack += 2;

  /* cs / ip / touble. ??? */

  set_current_client (callback_stack.cs, 0);

  /* This requires that the data area be pointed to by an LDT descriptor.  */
  data = LINEAR_TO_PTR
    (LDT_GET_BASE (current_client->callback_data[no].sel >> 3) +
     current_client->callback_data[no].offset);

  /* Fill-in the client's real mode structure.  */
  data->d.edi = callback_stack.edi;
  data->d.esi = callback_stack.esi;
  data->d.ebp = callback_stack.ebp;
  data->d.ebx = callback_stack.ebx;
  data->d.edx = callback_stack.edx;
  data->d.ecx = callback_stack.ecx;
  data->d.eax = callback_stack.eax;
  data->x.flags = callback_stack.eflags;
  data->x.es = callback_stack.es;
  data->x.ds = callback_stack.ds;
  data->x.fs = callback_stack.fs;
  data->x.gs = callback_stack.gs;
  data->x.ip = callback_stack.eip;
  data->x.cs = callback_stack.cs;
  data->x.sp = callback_stack.esp;
  data->x.ss = callback_stack.ss;

  /* Switch to locked stack and continue with part two.  */
  asm volatile
    ("	movw	%w0, %%ss
	movl	%1, %%esp"
     : /* No output */
     : "r" (current_client->locked_ss),
       "r" (current_client->locked_esp));
  callback_handler_part2 (no, data);
  /* Does not reach this point.  */
}
/* ---------------------------------------------------------------------- */
int
callback_allocate (word16 entrysel, word32 entryofs,
		   word16 datasel, word32 dataofs,
		   word16 *seg, word16 *ofs)
{
  int i;

  for (i = 0; i < CALLBACK_COUNT; i++)
    if (current_client_low->callback_code[i][0] == 0)
      {
	current_client_low->callback_code[i][0] = OP_PUSHB;
	current_client_low->callback_code[i][1] = i;
	current_client_low->callback_code[i][2] = OP_JMP_SHORT;
	current_client_low->callback_code[i][3] =
	  (CALLBACK_COUNT - 1 - i) * CALLBACK_SIZE;
	current_client->callback_entry[i].sel = entrysel;
	current_client->callback_entry[i].offset = entryofs;
	current_client->callback_data[i].sel = datasel;
	current_client->callback_data[i].offset = dataofs;
	*seg = current_client_handle;
	*ofs = CALLBACK_OFS (i);
	return 0;
      }
  return DPMI_ERROR_CALLBACK_UNAVAILABLE;
}
/* ---------------------------------------------------------------------- */
int
callback_free (word16 seg, word16 ofs)
{
  int dist, no;

  if (seg != current_client_handle)
    return DPMI_ERROR_INVALID_CALLBACK;

  dist = ofs - CALLBACK_OFS (0);
  no = dist / CALLBACK_SIZE;

  if (dist % CALLBACK_SIZE != 0 || no >= CALLBACK_COUNT)
    return DPMI_ERROR_INVALID_CALLBACK;

  memset (current_client_low->callback_code + no, 0, CALLBACK_SIZE);
  return 0;
}
/* ---------------------------------------------------------------------- */
