/* ---------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                 */
/* Distributed under the GPL, see COPYING for details.                    */
/* ---------------------------------------------------------------------- */
typedef struct client_memory_area
{
  struct client_memory_area *next;
  word32 size;
  word32 *page_tables;
  word32 linear;
} client_memory_area;
/* ---------------------------------------------------------------------- */
/* Data area describing the current client.  Careful with alignment!  */

struct client_high_data
{
  user_descriptor ldt[ldt_selector_count];	/* LDT.  */
  char ldt_types[ldt_selector_count];	/* Usage of LDT entries.  */
  tss_segment *tss;			/* Task.  */
  ptr3232 interrupt_table[0x100];	/* Software interrupts.  */
  ptr3232 exception_table_prot[0x20];	/* Exception handlers, protected.  */
  ptr3232 exception_table_real[0x20];	/* Exception handlers, real.  */
  word8 exception_handler_type[0x20];   /* 16/32, 0x0202/0x0210, ... */
  ptr3232 callback_entry[CALLBACK_COUNT]; /* Entry points for callbacks.  */
  ptr3232 callback_data[CALLBACK_COUNT];/* Data areas for callbacks.  */
  void *locked_stack;			/* User stack for hardware ints.  */
  void *server_stack, *server_stack_end;/* Server stack.  */
  client_memory_area *memory;		/* List of allocated areas.  */
  word16 psp;				/* Client's PSP.  */
  word16 org_env_seg;			/* Original environment segment.  */
  word16 previous_primary_client;	/* Back link to last client.  */
  word16 locked_ss;			/* SS for current locked stack.  */
  word32 locked_esp;			/* ESP for current locked stack.  */
  bool on_locked_stack;			/* Client's curr. of locked stack?  */
  bool is_32bit;
};
/* ---------------------------------------------------------------------- */
/* The is the layout of the client-supplied data area.  This should hold
   only things that must reside in real mode, and a pointer to the high
   data area.  */

struct client_low_data
{
  word8 real_mode_stack[REAL_MODE_STACK_SIZE];
  word8 callback_code[CALLBACK_COUNT][CALLBACK_SIZE];
  struct
    {
      word16 pushd1	__attribute__ ((packed));
      word32 esp	__attribute__ ((packed));
      word16 pushd2	__attribute__ ((packed));
      word32 eip	__attribute__ ((packed));
      word8  callf	__attribute__ ((packed));
      word16 ofs	__attribute__ ((packed));
      word16 seg	__attribute__ ((packed));
      word8  align[3]	__attribute__ ((packed));
    } callback_common;
  struct client_high_data *high_data;
};
/* ---------------------------------------------------------------------- */
#define CLIENT_HANDLE_TO_LOW(h) \
  ((struct client_low_data *) LINEAR_TO_PTR ((h) << 4))

#define CLIENT_HANDLE_TO_HIGH(h) (CLIENT_HANDLE_TO_LOW(h)->high_data)
/* ---------------------------------------------------------------------- */
