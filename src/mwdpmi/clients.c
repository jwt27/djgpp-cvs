/* ---------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                 */
/* Distributed under the GPL, see COPYING for details.                    */
/* ---------------------------------------------------------------------- */
#include "code32.h"
#include "clients.h"
#include "dpmi.h"
#include "ldt.h"
/* ---------------------------------------------------------------------- */
int
set_current_client (word16 new_client_handle, int force_reload)
{
  if (new_client_handle != current_client_handle)
    {
      ldt_descriptor *ldt_in_gdt;
      tss_descriptor *tss_in_gdt;
      word32 linear;

      current_client_handle = new_client_handle;
      current_client_low = CLIENT_HANDLE_TO_LOW (new_client_handle);
      current_client = current_client_low->high_data;

      linear = PTR_TO_LINEAR (current_client->ldt);
      ldt_in_gdt =
	LINEAR_TO_PTR (low_code_linear + (int)&gdt_low + gdt_ldt_sel);
      ldt_in_gdt->base0 = (word16)(linear);
      ldt_in_gdt->base1 = (word8)(linear >> 16);
      ldt_in_gdt->base2 = (word8)(linear >> 24);
      __cpu_set_ldtr (gdt_ldt_sel);

      linear = PTR_TO_LINEAR (current_client->tss);
      tss_in_gdt =
	LINEAR_TO_PTR (low_code_linear + (int)&gdt_low + gdt_tss_sel);
      tss_in_gdt->base0 = (word16)(linear);
      tss_in_gdt->base1 = (word8)(linear >> 16);
      tss_in_gdt->base2 = (word8)(linear >> 24);
      tss_in_gdt->stype &= ~2;  /* Mark not busy.  */
      __cpu_set_tr (gdt_tss_sel);

      /* Separate variable for fast access with no update-risky offsets
	 in assembler code.  */
      current_software_interrupt_table = current_client->interrupt_table;

      return 1;
    }

  /* Sometimes we must force reloading the Task and Local Descriptor
     registers.  */
  if (force_reload)
    {
      tss_descriptor *tss_in_gdt;
      tss_in_gdt =
	LINEAR_TO_PTR (low_code_linear + (int)&gdt_low + gdt_tss_sel);
      tss_in_gdt->stype &= ~2;  /* Mark not busy.  */

      __cpu_set_ldtr (gdt_ldt_sel);
      __cpu_set_tr (gdt_tss_sel);
    }

  return 0;
}
/* ---------------------------------------------------------------------- */
/* Release all ressources held by the current client.  */

void
client_free (void)
{
  if (current_client)
    {
      word16 *envsegptr;

      while (current_client->memory)
	client_free_memory ((word32)(current_client->memory));

      free (current_client->tss);
      free (current_client->locked_stack);
      free (current_client->server_stack);
      if (current_client->psp)
	{
	  envsegptr = LINEAR_TO_PTR ((current_client->psp << 4) +
				     DOS_ENVIRONMENT_OFFSET);
	  *envsegptr = current_client->org_env_seg;
	}

      free (current_client);
      current_client_low->high_data = current_client = 0;
      current_client_handle = 0;
      interrupts_safe = 0;
    }
}
/* ---------------------------------------------------------------------- */
/* Note that the segment registers in the following are word32s and not
   word16s.  gcc makes local copies in the latter case which we decidedly
   do not want in this function.  */

void
client_create (word32 edi, word32 esi, word32 ebp, word32 dummy,
	       word32 ebx, word32 edx, word32 ecx, word32 eax,
	       word32 esp, word32 ss,  word32 gs,  word32 fs,
	       word32 es,  word32 ds,  word32 eip, word32 cs,
	       word32 eflags)
{
  word16 *user_stack;
  __dpmi_regs regs;
  word16 *envsegptr;
  int new_cs, new_ds, new_es, new_ss, new_env_sel;
  int i, size;

  /* Make sure upper half is zero.  */
  ds = (word16)ds;
  es = (word16)es;
  fs = (word16)fs;
  gs = (word16)gs;
  ss = (word16)ss;

  /* "Pop" the return address from the user's stack.  */
  user_stack = LINEAR_TO_PTR ((ss << 4) + (esp & 0xffff));
  eip = *user_stack++, esp += 2;
  cs = *user_stack++, esp += 2;

  current_client_low = LINEAR_TO_PTR (es << 4);
  memset (current_client_low, 0, sizeof (struct client_low_data));
  current_client_low->high_data =
    current_client = malloc (sizeof (struct client_high_data));
  if (!current_client) goto error;
  memset (current_client, 0, sizeof (struct client_high_data));

  /* Place the common code for call-backs.  */
  current_client_low->callback_common.pushd1 = OP_PUSHD;
  current_client_low->callback_common.esp    = (word32)&callback_stack;
  current_client_low->callback_common.pushd2 = OP_PUSHD;
  current_client_low->callback_common.eip    = (word32)&callback_entry;
  current_client_low->callback_common.callf  = OP_CALLF;
  current_client_low->callback_common.ofs    = (word16)&go32;
  current_client_low->callback_common.seg    = code_seg;
#if (CALLBACK_COUNT - 1) * CALLBACK_SIZE >= 0x80
  callback_code_in_client_supplied_memory_is_incorrect;
#endif

  for (i = 0; i < 0x100; i++)
    {
      current_client->interrupt_table[i].offset = 4 * i + (int)&reflect;
      current_client->interrupt_table[i].sel = gdt_code32_sel;
    }
  current_client->interrupt_table[0x21].offset = (int)&prot_21_entry;
  current_client->interrupt_table[0x2f].offset = (int)&prot_2f_entry;
  current_client->interrupt_table[0x31].offset = (int)&prot_31_entry;

  /* FIXME: fill-in exception tables.  */

  current_client->tss = malloc (sizeof (tss_segment));
  if (!current_client->tss) goto error;
  memset (current_client->tss, 0, sizeof (tss_segment));
  current_client->tss->cr3 = page_dir_physical;
  current_client->tss->io_map = sizeof (tss_segment);
  /* Tss fill-in continued below.  */

  size = PAGE_SIZE;
  current_client->locked_stack = malloc (size);
  if (!current_client->locked_stack) goto error;
  /* Clear the stack because it is user-visible.  */
  memset (current_client->locked_stack, 0, size);
  current_client->locked_esp = (word32)current_client->locked_stack + size;
  current_client->locked_ss = gdt_data32_sel;

  size = 3 * PAGE_SIZE;
  current_client->server_stack = malloc (size);
  if (!current_client->server_stack) goto error;
  current_client->server_stack_end = current_client->server_stack + size;
  current_client->tss->ss0 = gdt_data32_sel;
  current_client->tss->esp0 = (word32)(current_client->server_stack_end);

  regs.h.ah = DOS_GET_PSP;
  server_int (INT_DOS, &regs);
  current_client->psp = regs.x.bx;
  envsegptr =
    LINEAR_TO_PTR ((current_client->psp << 4) + DOS_ENVIRONMENT_OFFSET);
  current_client->org_env_seg = *envsegptr;

  current_client->previous_primary_client = primary_client_handle;
  current_client->is_32bit = (eax & 1);

  if ((new_cs = ldt_allocate (1)) == -1) goto error;
  LDT_SET_TYPE (new_cs, 0xa, 16);
  LDT_SET_BASE (new_cs, cs << 4);
  LDT_SET_LIMIT (new_cs, 0xffff, 0);

  if ((new_ds = ldt_allocate (1)) == -1) goto error;
#ifdef NOT_NEEDED
  LDT_SET_TYPE (new_ds, 0x2, current_client->is_32bit ? 32 : 16);
#endif
  LDT_SET_BASE (new_ds, ds << 4);
  LDT_SET_LIMIT (new_ds, 0xffff, 0);

  if ((new_es = ldt_allocate (1)) == -1) goto error;
  current_client->ldt_types[new_es] = LDT_TYPE_SYSTEM;
  LDT_SET_TYPE (new_es, 0x2, 16);
  LDT_SET_BASE (new_es, (current_client->psp) << 4);
  LDT_SET_LIMIT (new_es, 0xffff, 0);  /* FIXME?  Specs say 0x100 */

  if ((new_env_sel = ldt_allocate (1)) == -1) goto error;
  current_client->ldt_types[new_env_sel] = LDT_TYPE_SYSTEM;
  LDT_SET_TYPE (new_env_sel, 0x2, 16);
  LDT_SET_BASE (new_env_sel, (current_client->org_env_seg) << 4);
  LDT_SET_LIMIT (new_env_sel, 0xffff, 0);

  if (ds == ss)
    new_ss = new_ds;
  else
    {
      if ((new_ss = ldt_allocate (1)) == -1) goto error;
#ifdef NOT_NEEDED
      LDT_SET_TYPE (new_ss, 0x2, current_client->is_32bit ? 32 : 16);
#endif
      LDT_SET_BASE (new_ss, ss << 4);
      LDT_SET_LIMIT (new_ss, 0xffff, 0);
    }

  /* Point of no return.  */
  set_current_client (es, 1);
  primary_client_handle = es;
  *envsegptr = LDT_SEL (new_env_sel);
  client_count++;
  if (current_client->is_32bit) esp = (word16)esp;
  fs = gs = 0;
  cs = LDT_SEL (new_cs);
  ds = LDT_SEL (new_ds);
  es = LDT_SEL (new_es);
  ss = LDT_SEL (new_ss);
  eflags = (eflags & (FLAGS_USER - (1 << FLAG_CF))) | (3 << FLAG_IOPL);
  /* Make sure the changed parameters are updated.  */
  BARRIER ();
  goto getout;

 error:
  if (client_count == 0 && one_shot_mode)
    {
      /* Minor bug here.  We should actually unload the server in this
	 case.  */
    }
  client_free ();
  /* The error code is bogus, but we don't have a choice per specs.  */
  eax = (eax & 0xffff0000) | DPMI_ERROR_DESCRIPTOR_UNAVAILABLE;
  eflags |= (1 << FLAG_CF);
  /* Make sure the changed parameters are updated.  */
  BARRIER ();
 getout:
}
/* ---------------------------------------------------------------------- */
void
client_terminate (int code)
{
  /* CAUTION -- stack change.  */

  termination_regs.h.ah = DOS_EXIT;
  termination_regs.h.al = code;
  /* Change stack because client_free () may release our current.  */
  asm volatile ("movl $temp_high_stack,%esp");

  /* Re-create the list of clients.  Either the terminating client is the
     primary client, orelse we have to fix the list in the middle.  */
  if (current_client_handle == primary_client_handle)
    primary_client_handle = current_client->previous_primary_client;
  else
    {
      struct client_high_data *client = current_client;
      while (client->previous_primary_client != primary_client_handle)
	client = CLIENT_HANDLE_TO_HIGH (client->previous_primary_client);
      client->previous_primary_client =
	CLIENT_HANDLE_TO_HIGH (client->previous_primary_client)
	  -> previous_primary_client;
    }
  client_free ();
  client_count--;

  if(client_count)
    set_current_client(primary_client_handle, 1);

  if (client_count == 0 && one_shot_mode && unload_safe ())
    unload (termination_regs.h.al);
  else
    while (1) server_int (INT_DOS, &termination_regs);
}
/* ---------------------------------------------------------------------- */
/* Allocate an area of SIZE bytes of address space.  If COMMIT then also
   allocate memory.  LINEAR will afterwards hold the linear address of the
   memory while HANDLE will hold a reference to the descriptor.  Returns
   zero or DPMI error code.  */

int
client_allocate_memory (word32 size, word32 *linear, word32 *handle,
			int commit)
{
  int gran_log = 2 * PAGE_SIZE_LOG - 2;
  int gran = 1 << gran_log;
  int pt_count = (size + (gran - 1)) / gran, pt_count_got = 0;
  int pg_count = (size + (PAGE_SIZE - 1)) / PAGE_SIZE, pg_count_got = 0;
  client_memory_area *area;
  int result = 0;
  word32 lin;

  if (size == 0) return DPMI_ERROR_INVALID_VALUE;

  area = malloc (sizeof (client_memory_area));
  if (area == 0) return DPMI_ERROR_MEMORY_UNAVAILABLE;
  memset (area, 0, sizeof (client_memory_area));

  *handle = (word32)area;
  area->size = size;

  /* Allocate linear address space for memory and page tables.  We will
     stick-in the page tables just before the client's memory.  */
  if (!(lin = area->linear = linear_alloc (size + pt_count * PAGE_SIZE)))
    result = DPMI_ERROR_LINEAR_UNAVAILABLE;
  else
    {
      /* Now allocate physical memory for page tables and map them.  */
      for (pt_count_got = 0 ; pt_count_got < pt_count; pt_count_got++)
	{
	  word32 pte;
	  int page = palloc ();

	  if (page == -1)
	    {
	      result = DPMI_ERROR_MEMORY_UNAVAILABLE;
	      goto error;
	    }
	  pte = (page << PAGE_SIZE_LOG) | (PT_P | PT_W | PT_U);
	  map_page_table (pte, lin);
	  /* For an explanation of this, see "doc".  */
	  if (pt_count_got == 0)
	    {
	      area->page_tables = LINEAR_TO_PTR (lin);
	      physical_poke (page << PAGE_SIZE_LOG, pte);
	      memset (area->page_tables + 1,
		      0,
		      PAGE_SIZE - sizeof (word32));
	    }
	  else
	    memset (area->page_tables + pt_count_got * (PAGE_SIZE / 4),
		    0,
		    PAGE_SIZE);
	  lin += PAGE_SIZE;
	}
      *linear = lin;

      if (commit)
	{
	  /* We are allocating committed memory so commit the pages.  */
	  word32 *pg_entry = area->page_tables + pt_count;

	  for (pg_count_got = 0 ; pg_count_got < pg_count; pg_count_got++)
	    if (commit_page (pg_entry++))
	      {
		result = DPMI_ERROR_MEMORY_UNAVAILABLE;
		goto error;
	      }
	}

      /* Success!  Update client's linear and link-in the new area.  */
      area->linear = *linear;
      area->next = current_client->memory;
      current_client->memory = area;
      return 0;
    }

 error:
  /* Free allocated pages for client memory.  */
  {
    word32 *pg_entry = area->page_tables + pt_count + (pg_count_got - 1);
    while (pg_count_got--)
      uncommit_page (pg_entry--);
  }

  /* Free page tables.  */
  while (pt_count_got--)
    pfree (ptr_to_physical (area->page_tables + pt_count_got * (PAGE_SIZE / 4))
	   / PAGE_SIZE);

  /* Free linear address space.  */
  if (area->linear) linear_free (area->linear, size);

  /* Free memory area descriptor.  */
  free (area);

  /* Return error code to caller.  */
  return result;
}
/* ---------------------------------------------------------------------- */
static int
check_memory_handle (word32 handle, client_memory_area **prev)
{
  client_memory_area *p = current_client->memory;

  *prev = 0;
  while (p)
    if ((word32)p == handle)
      return 1;
    else
      *prev = p, p = p->next;
  return 0;
}
/* ---------------------------------------------------------------------- */
int
client_free_memory (word32 handle)
{
  int gran = 1 << (2 * PAGE_SIZE_LOG - 2);
  int pt_count, pg_count;
  client_memory_area *prev, *area;
  word32 *pg_entry;

  if (!check_memory_handle (handle, &prev))
    return DPMI_ERROR_INVALID_HANDLE;

  area = (client_memory_area *)handle;
  pt_count = (area->size + (gran - 1)) / gran;
  pg_count = (area->size + (PAGE_SIZE - 1)) / PAGE_SIZE;

  /* Free allocated pages for client memory.  */
  pg_entry = area->page_tables + pt_count + (pg_count - 1);
  while (pg_count--)
    uncommit_page (pg_entry--);

  /* Restore correct linear address.  */
  area->linear -= pt_count * PAGE_SIZE;

  /* Free page tables.  */
  while (pt_count--)
    pfree (ptr_to_physical (area->page_tables + pt_count * (PAGE_SIZE / 4))
	   / PAGE_SIZE);

  /* Free linear address space.  */
  linear_free (area->linear, area->size);

  /* Unlink area from client's chain.  */
  if (prev)
    prev->next = area->next;
  else
    current_client->memory = area->next;
  free (area);
  return 0;
}
/* ---------------------------------------------------------------------- */
int
client_resize_memory (word32 *handle, word32 *linear, word32 newsize,
		      int commit)
{
  int gran = 1 << (2 * PAGE_SIZE_LOG - 2);
  int old_pt_count, old_pg_count, new_pt_count, new_pg_count, move_pg_count;
  int pg;
  client_memory_area *prev, *old_area, *new_area;
  int err;
  word32 new_handle;

  if (!check_memory_handle (*handle, &prev))
    return DPMI_ERROR_INVALID_HANDLE;
  if (newsize == 0) return DPMI_ERROR_INVALID_VALUE;

  old_area = (client_memory_area *)*handle;
  old_pt_count = (old_area->size + (gran - 1)) / gran;
  old_pg_count = (old_area->size + (PAGE_SIZE - 1)) / PAGE_SIZE;

  /* Allocate a new area uncomitted.  */
  err = client_allocate_memory (newsize, linear, &new_handle, 0);
  if (err) return err;
  new_area = (client_memory_area *)new_handle;
  new_pt_count = (newsize + (gran - 1)) / gran;
  new_pg_count = (newsize + (PAGE_SIZE - 1)) / PAGE_SIZE;

  if (commit)
    {
      /* Commit new pages.  */
      for (pg = old_pg_count; pg < new_pg_count; pg++)
	if (commit_page (new_area->page_tables + new_pt_count + pg))
	  {
	    client_free_memory (new_handle);
	    return DPMI_ERROR_MEMORY_UNAVAILABLE;
	  }
    }

  /* Move page table entries.  */
  move_pg_count = (new_pg_count > old_pg_count) ? old_pg_count : new_pg_count;
  for (pg = 0 ; pg < move_pg_count ; pg++)
    {
      new_area->page_tables [new_pt_count + pg]
	= old_area->page_tables [old_pt_count + pg];
      old_area->page_tables [old_pt_count + pg] = 0;
    }

  /* Free the old area together with the extra client pages it may still
     hold.  */
  client_free_memory (*handle);

  *handle = new_handle;
  return 0;
}
/* ---------------------------------------------------------------------- */
int
client_memory_get_info (word32 handle, word32 *size, word32 *linear)
{
  client_memory_area *prev, *area;
  if (!check_memory_handle (handle, &prev))
    return DPMI_ERROR_INVALID_HANDLE;

  area = (client_memory_area *)handle;
  *size = area->size;
  *linear = area->linear;
  return 0;
}
/* ---------------------------------------------------------------------- */
