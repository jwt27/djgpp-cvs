/* ---------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                 */
/* Distributed under the GPL, see COPYING for details.                    */
/* ---------------------------------------------------------------------- */
#include "code32.h"
#include "clients.h"
/* ---------------------------------------------------------------------- */
void
shutdown (int code)
{
  if (special_transfer_buffer)
    {
      termination_regs.h.ah = DOS_DEALLOCATE_MEMORY;
      termination_regs.x.es = transfer_buffer_seg;
      server_int (INT_DOS, &termination_regs);
    }

  termination_regs.h.al = code;
  termination_regs.x.flags = 0x3002;
  asm volatile ("movl $temp_high_stack,%esp");
  malloc_shutdown ();
  server_jump (code_seg, (word32) &exit_server, &termination_regs);
}
/* ---------------------------------------------------------------------- */
void
unload (int code)
{
  /* CAUTION -- stack change.  */
  word16 psp;
  char *patch;

  if (special_transfer_buffer)
    {
      termination_regs.h.ah = DOS_DEALLOCATE_MEMORY;
      termination_regs.x.es = transfer_buffer_seg;
      server_int (INT_DOS, &termination_regs);
    }

  /* Grab the prefix seg of the current process so we can replace its
     command line area with a small piece of code to unload the real
     mode part of the server.  */
  termination_regs.h.ah = DOS_GET_PSP;
  server_int (INT_DOS, &termination_regs);
  psp = termination_regs.x.bx;

  /* Patch in the segment to jump to.  */
  *((word16 *) LINEAR_TO_PTR (low_code_linear + (int)&unload_server_seg)) =
    psp;

  /* Place termination code in calling program's psp.  */
  patch = LINEAR_TO_PTR ((psp << 4) + DOS_CMDLINE_OFFSET);
  *patch++ = OP_INT;
  *patch++ = INT_DOS;
  *patch++ = OP_MOV_AX;
  *patch++ = code;
  *patch++ = DOS_EXIT;
  *patch++ = OP_INT;
  *patch++ = INT_DOS;

  asm volatile ("movl $temp_high_stack,%esp");
  malloc_shutdown ();
  termination_regs.x.flags = 0x3002;
  server_jump (code_seg, (word32) &unload_server, &termination_regs);
}
/* ---------------------------------------------------------------------- */
void
tsr (void)
{
  __dpmi_regs regs;
  int i;

  /* Close stdin, stdout, [not stderr], stdprn, and stdaux.  */
  for (i = 0; i < 5; i++)
    if (i != DOS_STDERR_FILENO)
      {
	regs.h.ah = DOS_CLOSE_FILE;
	regs.x.bx = i;
	server_int (INT_DOS, &regs);
      }

  /* We don't need most of the program to remain in low memory.  */
  regs.x.ax = (DOS_TSR << 8) + 0;
  regs.x.dx = (low_memory_end + 0x100) >> 4;
  while (1) server_int (INT_DOS, &regs);
}
/* ---------------------------------------------------------------------- */
int
unload_safe (void)
{
  /* If something else has hooked one of the interrupt vectors we have
     then we cannot remove the server from memory at this time.  */
  termination_regs.x.ax = (DOS_GET_INT_VEC << 8) + 0x2f;
  server_int (INT_DOS, &termination_regs);
  if (termination_regs.x.es != code_seg ||
      termination_regs.x.bx != (word16)&real_2f) return 0;
  if (old15)
    {
      termination_regs.x.ax = (DOS_GET_INT_VEC << 8) + 0x15;
      server_int (INT_DOS, &termination_regs);
      if (termination_regs.x.es != code_seg ||
	  termination_regs.x.bx != (word16)&real_15) return 0;
    }
  return 1;
}
/* ---------------------------------------------------------------------- */
void
parse_extra_cmdline (void)
{
  switch (parse_command_line (unload_safe () ? 0 : 2))
    {
    case 0:
      termination_regs.h.al = 0;
      break;
    case 2:
      MESSAGE ("Error: server already resident.\r\n");
      /* Fall through.  */
    default:
    case 1:
      termination_regs.h.al = 1;
      break;
    case 3:
      unload (0);
    }
  termination_regs.h.ah = DOS_EXIT;
  while (1) server_int (INT_DOS, &termination_regs);
}
/* ---------------------------------------------------------------------- */
static void
shrink_real_mode_part (void)
{
  __dpmi_regs regs;

  /* At this point we might consider moving the code segment.  There are
     several possibilities:
     1. There might be room for the code in an UMB block, possibly the
        one that holds the VCPI page table.
     2. If (a) there is a page break at offset X, (b) the VCPI page table
        is loaded low, and (c) X is less than the amount needed to round
	low_memory_end up to a full page; then the page table could be
	placed before the code instead of after it.  */

  /* Round to full paragraph.  */
  low_memory_end = (low_memory_end + 0xf) & ~0xf;

  if (vcpi_present && vcpi_memory_handle < 0xa000)
    {
      /* The attempt to load the page table high failed for some reason.
	 Since we are shrinking the real mode part we want to have the page
	 table right after our program, not where it is now because that
	 would leave a hole.  */

      int seg = (code_seg + (low_memory_end >> 4) + ((PAGE_SIZE - 1) >> 4))
	& ~((PAGE_SIZE - 1) >> 4);
      regs.x.ax = VCPI_INTERFACE;
      regs.x.es = seg;
      regs.x.di = 0;
      regs.x.ds = code_seg;
      regs.x.si = (word16) &vcpi_descriptors;
      server_int (INT_VCPI, &regs);
      low_memory_end = ((seg - code_seg) << 4) + PAGE_SIZE;
      regs.x.ax = VCPI_GET_PHYMEMADR;
      regs.x.cx = seg >> (PAGE_SIZE_LOG - 4);
      server_int (INT_VCPI, &regs);
      regs.x.dx &= ~(PAGE_SIZE - 1);
      map_page_table (regs.d.edx | PT_P | PT_W | PT_U, 0x00000000);

      regs.h.ah = DOS_DEALLOCATE_MEMORY;
      regs.x.es = vcpi_memory_handle;
      server_int (INT_DOS, &regs);
      *((word16 *) LINEAR_TO_PTR (low_code_linear + (int)&vcpi_memory_handle))
	= vcpi_memory_handle = 0;
    }

  /* Allocate transfer buffer.  Comes after relocating VCPI buffer so that
     we get page alignment for free in case the VCPI buffer is relocated
     in low memory.  */
  if (dos_major >= 5)
    {
      /* We might get lucky and get the buffer in UMBs.  */
      regs.x.ax = DOS_SET_ALLOCATION_STRATEGY;
      regs.x.bx = 0x0040;  /* First fit in high memory only.  */
      server_int (INT_DOS, &regs);

      regs.x.ax = DOS_SET_UMB_STATUS;
      regs.x.bx = 1;  /* Link UMBs.  */
      server_int (INT_DOS, &regs);

      regs.h.ah = DOS_ALLOCATE_MEMORY;
      regs.x.bx = PAGE_SIZE >> 4;
      server_int (INT_DOS, &regs);
      if (~regs.x.flags & (1 << FLAG_CF))
	{
	  *((word16 *)
	    LINEAR_TO_PTR (low_code_linear + (int)&transfer_buffer_seg)) =
	      transfer_buffer_seg = regs.x.ax;
	  special_transfer_buffer = 1;
	}

      regs.x.ax = DOS_SET_UMB_STATUS;
      regs.x.bx = umb_status;
      server_int (INT_DOS, &regs);

      regs.x.ax = DOS_SET_ALLOCATION_STRATEGY;
      regs.x.bx = allocation_strategy;
      server_int (INT_DOS, &regs);
    }
  if (transfer_buffer_seg == 0)
    {
      /* For whatever reason we didn't get one.  Use memory after code.  */
	*((word16 *)
	  LINEAR_TO_PTR (low_code_linear + (int)&transfer_buffer_seg)) =
	    transfer_buffer_seg = code_seg + (low_memory_end >> 4);
      low_memory_end += PAGE_SIZE;
    }

  /* We don't need most of the program to remain in low memory.  */
  regs.h.ah = DOS_RESIZE_MEMORY;
  regs.x.bx = (low_memory_end + 0x100) >> 4;
  regs.x.es = prefixseg;
  server_int (INT_DOS, &regs);
}
/* ---------------------------------------------------------------------- */
static void
discard_environ (void)
{
  word16 *envsegptr, envseg;

  envsegptr = LINEAR_TO_PTR ((prefixseg << 4) + DOS_ENVIRONMENT_OFFSET);
  envseg = *envsegptr;

  /* Might be zero, I think, if run from config.sys.  */
  if (envseg != 0)
    {
      __dpmi_regs regs;
      char *p;

      /* Get argv[0].  */
      p = LINEAR_TO_PTR (envseg << 4);
      while (p[0] != 0 || p[1] != 0) p++;
      p += 2;
      if (p[0] == 0 && p[1] == 0)
	argv0 = "dpmi";
      else
	{
	  p += 2;
	  argv0 = xmalloc (strlen (p));
	  strcpy (argv0, p);
	}

      *envsegptr = 0;
      regs.h.ah = DOS_DEALLOCATE_MEMORY;
      regs.x.es = envseg;
      server_int (INT_DOS, &regs);
    }
}
/* ---------------------------------------------------------------------- */
static void
setup_idt (void)
{
  int i;
  pseudo_descriptor *low_idt_rec;

  idt = xmalloc (sizeof (idt_entry) * 0x100);
  for (i = 0 ; i < 0x100; i++)
    {
      idt[i].offset0 = i * 4 + (word16)&interrupt_entries;
      idt[i].selector = gdt_code32_sel;
      idt[i].stype = 0xef00; /* Trap Gate, DPL=3 */
      idt[i].offset1 = 0;
    }
  low_idt_rec = LINEAR_TO_PTR (low_code_linear + (word32)&idt_rec);
  idt_rec.limit = low_idt_rec->limit = 0x100 * sizeof (idt_entry) - 1;
  idt_rec.base = low_idt_rec->base = PTR_TO_LINEAR (idt);
  __cpu_set_idt (&idt_rec);
}
/* ---------------------------------------------------------------------- */
void
init_server32 (void)
{
  malloc_init ();

  discard_environ ();
  shrink_real_mode_part ();
  setup_idt ();
  *((word16 *) LINEAR_TO_PTR (low_code_linear + (int)&client_low_data_needs)) =
    (sizeof (struct client_low_data) + 0xf) >> 4;

  switch (parse_command_line (1))
    {
    case 0: shutdown (0);
    default: case 1: shutdown (1);
    case 2: tsr ();
#if 0
    case 3: unload (0); /* Shouldn't happen here.  */
#endif
    }
}
/* ---------------------------------------------------------------------- */
