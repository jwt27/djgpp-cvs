/* ---------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                 */
/* Distributed under the GPL, see COPYING for details.                    */
/* ---------------------------------------------------------------------- */
#include "interface.h"
#include "cpu.h"
#include <dpmi.h>

/* ---------------------------------------------------------------------- */
#ifdef DEBUG
int debug_flags = 0;
#endif
/* ---------------------------------------------------------------------- */
/* It seems that we can't call VCPI when interrupts are not safe, even if
   we have interrupts disabled.  */

bool interrupts_safe = 0;
/* ---------------------------------------------------------------------- */
/* djgpp programs run the server in "one-shot-mode" meaning that the server
   automatically unloads itself after the client terminates.  The
   alternative is "permanent".  */

bool one_shot_mode = 0;
/* ---------------------------------------------------------------------- */
/* Number of active clients.  */

int client_count = 0;
/* ---------------------------------------------------------------------- */
/* Interrupt Descriptor Table.  */

idt_entry *idt;
/* ---------------------------------------------------------------------- */
/* Software Interrupt Table.  */

ptr3232 *current_software_interrupt_table;
/* ---------------------------------------------------------------------- */
/* The name of the executable.  */

char *argv0;
/* ---------------------------------------------------------------------- */
/* The real mode data segment of the current client.  */
word16 current_client_handle = 0;

/* A pointer to the same area.  */
struct client_low_data *current_client_low;

/* A pointer to the larger extended memory data area.  */
struct client_high_data *current_client;

/* The handle of the primary client.  */
word16 primary_client_handle = 0;
/* ---------------------------------------------------------------------- */
/* Registers for short-term use during termination exercises.  */

__dpmi_regs termination_regs;
/* ---------------------------------------------------------------------- */
/* True if the transfer buffer was allocated explicitly.  */

bool special_transfer_buffer = 0;
/* ---------------------------------------------------------------------- */
