/* ---------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                 */
/* Distributed under the GPL, see COPYING for details.                    */
/* ---------------------------------------------------------------------- */
#include "code32.h"
#include "ldt.h"
#include "dpmi.h"
#include "clients.h"

#define ERROR(no) do { AX = (no); goto error; } while (0)

#define AX (*(word16 *)&eax)
#define AL (*(word8  *)&eax)
#define AH (*((word8  *)&eax + 1))
#define BX (*(word16 *)&ebx)
#define BL (*(word8  *)&ebx)
#define CX (*(word16 *)&ecx)
#define CL (*(word8  *)&ecx)
#define CH (*((word8  *)&ecx + 1))
#define DX (*(word16 *)&edx)
#define DI (*(word16 *)&edi)
#define SI (*(word16 *)&esi)
#define BP (*(word16 *)&ebp)
#define SP (*(word16 *)&esp)

#define DX_OR_EDX (current_client->is_32bit ? edx : (word16)edx)
#define SET_DX_OR_EDX(v) \
  if (current_client->is_32bit) edx = (v); else DX = (v)

#define DI_OR_EDI (current_client->is_32bit ? edi : (word16)edi)
#define SET_DI_OR_EDI(v) \
  if (current_client->is_32bit) edi = (v); else DI = (v)

#define SI_OR_ESI (current_client->is_32bit ? esi : (word16)esi)
#define SET_SI_OR_ESI(v) \
  if (current_client->is_32bit) esi = (v); else SI = (v)

/* If ss should be in GDT assume it is big.  */
#define BIG_STACK_P \
  ((ss & 4) ? \
   (current_client->ldt[ss >> 3].limit1 & 0x40) : \
   1)

#define SP_OR_ESP (BIG_STACK_P ? esp : SP)
/* ---------------------------------------------------------------------- */
/* Handy procedure for setting a series of selectors mapping a dos memory
   area in 64K chunks.  */

static void
set_dos_memory (int i0, int count, word32 base, word32 size)
{
  word32 limit = size ? size - 1 : 0;

  LDT_SET_BASE (i0, base);
  LDT_SET_LIMIT (i0, limit, 0);
  i0++, count--;

  while (count)
    {
      base += 0x10000;
      limit -= 0x10000;

      LDT_SET_BASE (i0, base);
      LDT_SET_LIMIT (i0, ((limit > 0xffff) ? 0xffff : limit), 0);

      i0++, count--;
    }
}
/* ---------------------------------------------------------------------- */
static __inline__ int
ldt_rights_ok (int gd, int typ)
{
  return
    ((typ & 0x10) &&
     ((typ >> 5) & 3) == RING &&
     ((typ & 0x80) == 0 || (gd & 0x20) == 0));
}
/* ---------------------------------------------------------------------- */
#ifdef DEBUG
static void
prot31_debug (void)
{
  __dpmi_regs regs;
  int again = 1;

  while (again)
    {
      regs.h.ah = DOS_READ_STDIN;
      server_int (INT_DOS, &regs);
      switch (regs.h.al)
	{
	case 'l':
	  ldt_print ();
	  break;
	case 'Q':
	  client_terminate (1);
	  break;
	default:
	  again = 0;
	}
    }
}
#endif
/* ---------------------------------------------------------------------- */
/* This routine is called with interrupts OFF.  */

void
prot_31 (word32 edi, word32 esi, word32 ebp, word32 dummy,
	 word32 ebx, word32 edx, word32 ecx, word32 eax,
	 word32 gs,  word32 fs,  word32 es,  word32 ds,
	 word32 eip, word32 cs,  word32 eflags,
	 word32 esp, word32 ss)
{
  int i, sel;

  if (DEBUG_TEST (DEBUG_31))
    {
      eprintf ("a=%08x b=%08x c=%08x d=%08x si=%08x di=%08x bp=%08x\r\n"
	       "cs:eip=%04x:%08x ds=%04x es=%04x fs=%04x gs=%04x "
	       "ss:esp=%04x:%08x\r\n\n",
	       eax, ebx, ecx, edx, esi, edi, ebp,
	       cs, eip, ds, es, fs, gs, ss, esp);
      if (DEBUG_TEST (DEBUG_31_PAUSE)) prot31_debug ();
    }
  /* As default signal "no error".  */
  eflags &= ~(1 << FLAG_CF);

  /* For speed we make a nested switch, group first, subfunction next.  */
  switch (AH)
    {
    /* -------------------------------------------------------------------- */
    case 0x00: /* LDT manipulation.  */
      switch (AL)
	{
	case DPMI_ALLOCATE_DESCRIPTORS - 0x0000:  /* 0.9 */
	  if (CX == 0) ERROR (DPMI_ERROR_INVALID_VALUE);
	  i = ldt_allocate (CX);
	  if (i == -1) ERROR (DPMI_ERROR_DESCRIPTOR_UNAVAILABLE);
	  AX = LDT_SEL (i);
	  BARRIER ();
	  goto show_ldt;

	case DPMI_FREE_DESCRIPTOR - 0x0000:  /* 0.9 */
	  sel = BX;
	  if (SEL_LDT_MODIFYABLE_P (sel))
	    {
	      i = sel >> 3;
	      current_client->ldt_types[i] = LDT_TYPE_FREE;
	      if ((ds >> 3) == i) ds = 0;
	      if ((es >> 3) == i) es = 0;
	      if ((fs >> 3) == i) fs = 0;
	      if ((gs >> 3) == i) gs = 0;
	      /* Trouble if CS or SS is freed.  */
	      BARRIER ();
	      goto show_ldt;
	    }
	  else
	    ERROR (DPMI_ERROR_INVALID_SELECTOR);

	case DPMI_SEGMENT_TO_DESCRIPTOR - 0x0000:  /* 0.9 */
	  for (i = 0; i < ldt_selector_count; i++)
	    if (current_client->ldt_types[i] == LDT_TYPE_SEGMENT &&
		LDT_GET_BASE (i) == (BX << 4))
	      {
		/* Found descriptor from previous similar call.  */
		AX = LDT_SEL (i);
		BARRIER ();
		return;
	      }
	  i = ldt_allocate (1);
	  if (i == -1) ERROR (DPMI_ERROR_DESCRIPTOR_UNAVAILABLE);
	  current_client->ldt_types[i] = LDT_TYPE_SEGMENT;
	  LDT_SET_BASE (i, BX << 4);
	  LDT_SET_BIT (i, 16);
	  LDT_SET_LIMIT (i, 0xffff, 0);
	  AX = LDT_SEL (i);
	  BARRIER ();
	  goto show_ldt;

	case DPMI_GET_SELECTOR_INCREMENT - 0x0000:  /* 0.9 */
	  AX = 8;
	  BARRIER ();
	  return;

	case DPMI_GET_SEGMENT_BASE - 0x0000:  /* 0.9 */
	  sel = BX;
	  if (SEL_LDT_INSPECTABLE_P (sel))
	    {
	      word32 base = LDT_GET_BASE (sel >> 3);
	      CX = base >> 16;
	      DX = base;
	      BARRIER ();
	      return;
	    }
	  else
	    ERROR (DPMI_ERROR_INVALID_SELECTOR);

	case DPMI_SET_SEGMENT_BASE - 0x0000:  /* 0.9 */
	  sel = BX;
	  if (SEL_LDT_MODIFYABLE_P (sel))
	    {
	      /* Paranoid checks go here.  */
	      LDT_SET_BASE (sel >> 3, (CX << 16) | DX);
	      /* Reload is automatic.  */
	      BARRIER ();
	      goto show_ldt;
	    }
	  else
	    ERROR (DPMI_ERROR_INVALID_SELECTOR);

	case DPMI_SET_SEGMENT_LIMIT - 0x0000:  /* 0.9 */
	  sel = BX;
	  if (SEL_LDT_MODIFYABLE_P (sel))
	    {
	      word32 limit = (CX << 16) | DX;
	      /* Paranoid checks go here.  */
	      if (limit > 0xfffff)
		if ((limit & (PAGE_SIZE - 1)) != (PAGE_SIZE - 1))
		  ERROR (DPMI_ERROR_INVALID_VALUE);
		else
		  LDT_SET_LIMIT (sel >> 3, limit >> PAGE_SIZE_LOG, 1);
	      else
		LDT_SET_LIMIT (sel >> 3, limit, 0);
	      /* Reload is automatic.  */
	      BARRIER ();
	      goto show_ldt;
	    }
	  else
	    ERROR (DPMI_ERROR_INVALID_SELECTOR);

	case DPMI_SET_ACCESS_RIGHTS - 0x0000:  /* 0.9 */
	  sel = BX;
	  if (SEL_LDT_MODIFYABLE_P (sel))
	    {
	      if (!ldt_rights_ok (CH, CL))
		ERROR (DPMI_ERROR_INVALID_VALUE);
	      /* Paranoid checks go here.  */
	      current_client->ldt[sel >> 3].stype = CL;
	      current_client->ldt[sel >> 3].limit1 &= 0x0f;
	      current_client->ldt[sel >> 3].limit1 |= (CH & 0xf0);
	      /* Reload is automatic.  */
	      BARRIER ();
	      goto show_ldt;
	    }
	  else
	    ERROR (DPMI_ERROR_INVALID_SELECTOR);

	case DPMI_CREATE_ALIAS - 0x0000:  /* 0.9 */
	  sel = BX;
	  if (SEL_LDT_INSPECTABLE_P (sel))
	    {
	      int i = ldt_allocate (1);
	      if (i == -1) ERROR (DPMI_ERROR_DESCRIPTOR_UNAVAILABLE);
	      memcpy (&(current_client->ldt[i]),
		      &(current_client->ldt[sel >> 3]),
		      sizeof (struct user_descriptor));
	      current_client->ldt[i].stype &= 0xf0;
	      current_client->ldt[i].stype |= 0x02;  /* RW, down, non-acc.  */
	      AX = LDT_SEL (i);
	      BARRIER ();
	      goto show_ldt;
	    }
	  else
	    ERROR (DPMI_ERROR_INVALID_SELECTOR);

	case DPMI_GET_DESCRIPTOR - 0x0000:  /* 0.9 */
	  sel = BX;
	  if (SEL_LDT_INSPECTABLE_P (es) && __cpu_verw (es) &&
	      SEL_LDT_INSPECTABLE_P (sel))
	    {
	      movedata (gdt_data32_sel,
			(word32)&(current_client->ldt[sel >> 3]),
			es, DI_OR_EDI,
			sizeof (struct user_descriptor));
	      BARRIER ();
	      return;
	    }
	  else
	    ERROR (DPMI_ERROR_INVALID_SELECTOR);

	case DPMI_SET_DESCRIPTOR - 0x0000:  /* 0.9 */
	  sel = BX;
	  if (SEL_LDT_INSPECTABLE_P (es) && __cpu_verr (es) &&
	      SEL_LDT_INSPECTABLE_P (sel))
	    {
	      user_descriptor desc;
	      movedata (es, DI_OR_EDI,
			gdt_data32_sel, (word32)(&desc),
			sizeof (struct user_descriptor));
	      if (!ldt_rights_ok (desc.limit1, desc.stype))
		ERROR (DPMI_ERROR_INVALID_VALUE);
	      /* Paranoid checks go here.  */
	      current_client->ldt[sel >> 3] = desc;
	      BARRIER ();
	      goto show_ldt;
	    }
	  else
	    ERROR (DPMI_ERROR_INVALID_SELECTOR);

	case DPMI_ALLOCATE_SPECIFIC_DESCRIPTOR - 0x0000:  /* 0.9 */
	  sel = BX;
	  if (SEL_LDT_INSPECTABLE_P (sel))
	    ERROR (DPMI_ERROR_DESCRIPTOR_UNAVAILABLE);
	  else
	    if (SEL_LDT_FREE_P (sel))
	      {
		current_client->ldt_types[sel >> 3] = LDT_TYPE_USER;
		LDT_SET_BASE (sel >> 3, 0x00000000);
		LDT_SET_LIMIT (sel >> 3, 0x00000, 0);
		current_client->ldt[sel >> 3].limit0 = 0;
		if (current_client->is_32bit)
		  current_client->ldt[sel >> 3].limit1 |= 0x40;
		current_client->ldt[sel >> 3].stype = (RING << 5) | 0x92;
		BARRIER ();
		goto show_ldt;
	      }
	    else
	      ERROR (DPMI_ERROR_INVALID_SELECTOR);

	case DPMI_GET_MULTIPLE_DESCRIPTORS - 0x0000:  /* 1.0 */
	case DPMI_SET_MULTIPLE_DESCRIPTORS - 0x0000:  /* 1.0 */
	  ERROR (DPMI_ERROR_UNSUPPORTED);

	default: goto bad_function;
	}

    show_ldt:
      /* When the LDT changes we go here.  */
      if (DEBUG_TEST (DEBUG_LDT))
	{
	  eprintf ("LDT:\r\n");
	  ldt_print ();
	  eprintf ("\r\n");
	}
      return;
    /* -------------------------------------------------------------------- */
    case 0x01: /* DOS memory allocation group.  */
      switch (AL)
	{
	case DPMI_ALLOCATE_DOS_MEMORY - 0x0100:  /* 0.9 */
	  {
	    __dpmi_regs regs;
	    int needed, doserr;

	    /* Allocate from DOS.  */
	    regs.h.ah = DOS_ALLOCATE_MEMORY;
	    regs.x.bx = BX;
	    server_int (INT_DOS, &regs);
	    doserr = (regs.x.flags & (1 << FLAG_CF)) ? regs.x.ax : 0;

	    if (doserr)
	      {
		BX = regs.x.bx;
		ERROR (doserr);
	      }

	    /* Number of selectors needed.  DPMI 1.0 specs are clear that
	       32-bit client never allocate for than 1.  This comes after
	       allocation of DOS memory so that a test allocation of 0xffff
	       will give the desired result.  */
	    needed =
	      current_client->is_32bit ? 1 : (BX ? (BX - 1) / 0x1000 : 1);
	    i = ldt_allocate (needed);

	    if (i == -1)
	      {
		regs.x.es = regs.x.ax;
		regs.h.ah = DOS_DEALLOCATE_MEMORY;
		server_int (INT_DOS, &regs);
		ERROR (DPMI_ERROR_DESCRIPTOR_UNAVAILABLE);
	      }

	    /* We have both memory and selectors.  */
	    if (DEBUG_TEST (DEBUG_MEMORY))
	      eprintf ("Dos memory allocated.  "
		       "Seg=%04x, size=%x, sels=%04x-%04x.\r\n",
		       regs.x.ax,
		       BX << 4,
		       LDT_SEL (i), LDT_SEL (i + needed - 1));
	    set_dos_memory (i, needed, regs.x.ax << 4, BX << 4);
	    AX = regs.x.ax;
	    DX = LDT_SEL (i);
	    current_client->ldt_types[i++] = LDT_TYPE_DOS;
	    while (--needed)
	      current_client->ldt_types[i++] = LDT_TYPE_DOS_CONT;
	    BARRIER ();
	    goto show_ldt;
	  }

	case DPMI_FREE_DOS_MEMORY - 0x0100:  /* 0.9 */
	  {
	    __dpmi_regs regs;
	    int count, seg;

	    sel = DX;
	    i = sel >> 3;
	    if (SEL_LDT_INSPECTABLE_P (sel) &&
		current_client->ldt_types[i] == LDT_TYPE_DOS)
	      {
		seg = LDT_GET_BASE (i) >> 4;
		count = current_client->is_32bit
		  ? 1 : ((LDT_GET_LIMIT (i) | 1) + 0xffff) >> 16;
		regs.h.ah = DOS_DEALLOCATE_MEMORY;
		regs.x.es = seg;
		server_int (INT_DOS, &regs);
		if (regs.x.flags & (1 << FLAG_CF)) ERROR (regs.x.ax);
		if (DEBUG_TEST (DEBUG_MEMORY))
		  eprintf ("Dos memory with selector %04x freed.\r\n",
			   sel);
		if ((ds >> 3) >= i && (ds << 3) < i + count) ds = 0;
		if ((es >> 3) >= i && (es << 3) < i + count) es = 0;
		if ((fs >> 3) >= i && (fs << 3) < i + count) fs = 0;
		if ((gs >> 3) >= i && (gs << 3) < i + count) gs = 0;
		while (count--)
		  current_client->ldt_types[i++] = LDT_TYPE_FREE;
		BARRIER ();
		goto show_ldt;
	      }
	    else
	      ERROR (DPMI_ERROR_INVALID_SELECTOR);
	  }

	case DPMI_RESIZE_DOS_MEMORY - 0x0100:  /* 0.9 */
	  ERROR (DPMI_ERROR_UNSUPPORTED);

	default: goto bad_function;
	}
    /* -------------------------------------------------------------------- */
    case 0x02: /* Interrupt and exception group.  */
      switch (AL)
	{
	case DPMI_GET_REAL_MODE_VECTOR - 0x0200:  /* 0.9 */
	  {
	    word16 *p = LINEAR_TO_PTR (BL * 4);

	    DX = p[0];
	    CX = p[1];
	    BARRIER ();
	    return;
	  }

	case DPMI_SET_REAL_MODE_VECTOR - 0x0200:  /* 0.9 */
	  {
	    word16 *p = LINEAR_TO_PTR (BL * 4);

	    p[0] = DX;
	    p[1] = CX;
	    if (DEBUG_TEST (DEBUG_INT))
	      eprintf ("Real-mode int 0x%02x set to %04x:%04x.\r\n",
		       BL, CX, DX);
	    BARRIER ();
	    return;
	  }

	case DPMI_GET_EXCEPTION_VECTOR - 0x0200:  /* 0.9 (old) */
	case DPMI_GET_EXTENDED_EXCEPTION_PROT - 0x0200:  /* 1.0 */
	  if (BL < 0x20)
	    {
	      CX = current_client->exception_table_prot[BL].sel;
	      SET_DX_OR_EDX (current_client->exception_table_prot[BL].offset);
	      BARRIER ();
	      return;
	    }
	  else
	    ERROR (DPMI_ERROR_INVALID_VALUE);

	case DPMI_SET_EXCEPTION_VECTOR - 0x0200:  /* 0.9 (old) */
	case DPMI_SET_EXTENDED_EXCEPTION_PROT - 0x0200:  /* 1.0 */
	  if (BL < 0x20)
	    {
	      sel = CX;
	      if (SEL_LDT_INSPECTABLE_P (sel) ||
		  sel == gdt_code32_sel || sel == gdt_code16_sel)
		{
		  current_client->exception_table_prot[BL].sel = CX;
		  current_client->exception_table_prot[BL].offset = DX_OR_EDX;
		  current_client->exception_handler_type[BL] = 0;  /* FIXME */
		  if (DEBUG_TEST (DEBUG_INT))
		    if (current_client->is_32bit)
		      eprintf ("Protected-mode exception 0x%02x handler "
			       "set to %04x:%08x.\r\n",
			       BL, CX, edx);
		    else 
		      eprintf ("Protected-mode exception 0x%02x handler "
			       "set to %04x:%04x.\r\n",
			       BL, CX, DX);
		  BARRIER ();
		  return;
		}
	      else
		ERROR (DPMI_ERROR_INVALID_SELECTOR);
	    }
	  else
	    ERROR (DPMI_ERROR_INVALID_VALUE);

	case DPMI_GET_INTERRUPT_VECTOR - 0x0200:  /* 0.9 */
	  CX = current_client->interrupt_table[BL].sel;
	  SET_DX_OR_EDX (current_client->interrupt_table[BL].offset);
	  BARRIER ();
	  return;

	case DPMI_SET_INTERRUPT_VECTOR - 0x0200:  /* 0.9 */
	  sel = CX;
	  if (SEL_LDT_INSPECTABLE_P (sel) ||
	      sel == gdt_code32_sel || sel == gdt_code16_sel)
	    {
	      current_client->interrupt_table[BL].sel = CX;
	      current_client->interrupt_table[BL].offset = DX_OR_EDX;
	      if (DEBUG_TEST (DEBUG_INT))
		if (current_client->is_32bit)
		  eprintf ("Protected-mode int 0x%02x set to %04x:%08x.\r\n",
			   BL, CX, edx);
		else 
		  eprintf ("Protected-mode int 0x%02x set to %04x:%04x.\r\n",
			   BL, CX, DX);
	      BARRIER ();
	      return;
	    }
	  else
	    ERROR (DPMI_ERROR_INVALID_SELECTOR);

	case DPMI_GET_EXTENDED_EXCEPTION_REAL - 0x0200:  /* 1.0 */
	  if (BL < 0x20)
	    {
	      CX = current_client->exception_table_real[BL].sel;
	      SET_DX_OR_EDX (current_client->exception_table_real[BL].offset);
	      BARRIER ();
	      return;
	    }
	  else
	    ERROR (DPMI_ERROR_INVALID_VALUE);

	case DPMI_SET_EXTENDED_EXCEPTION_REAL - 0x0200:  /* 1.0 */
	  if (BL < 0x20)
	    {
	      sel = CX;
	      if (SEL_LDT_INSPECTABLE_P (sel) ||
		  sel == gdt_code32_sel || sel == gdt_code16_sel)
		{
		  /* FIXME: this doesn't do anything but record the entry
		     point.  */
		  current_client->exception_table_real[BL].sel = CX;
		  current_client->exception_table_real[BL].offset = DX_OR_EDX;
		  if (DEBUG_TEST (DEBUG_INT))
		    if (current_client->is_32bit)
		      eprintf ("Real-mode exception 0x%02x handler "
			       "set to %04x:%08x.\r\n",
			       BL, CX, edx);
		    else 
		      eprintf ("Real-mode exception 0x%02x handler "
			       "set to %04x:%04x.\r\n",
			       BL, CX, DX);
		  BARRIER ();
		  return;
		}
	      else
		ERROR (DPMI_ERROR_INVALID_SELECTOR);
	    }
	  else
	    ERROR (DPMI_ERROR_INVALID_VALUE);

	default: goto bad_function;
	}
    /* -------------------------------------------------------------------- */
    case 0x03: /* Call real mode group.  */
      switch (AL)
	{
	case DPMI_SIMULATE_REAL_MODE_INT - 0x0300:  /* 0.9 */
	case DPMI_CALL_REAL_MODE_FAR - 0x0300:  /* 0.9 */
	case DPMI_CALL_REAL_MODE_IRET - 0x0300:  /* 0.9 */
	  sel = es;
	  if (SEL_LDT_INSPECTABLE_P (sel) && __cpu_verw (sel))
	    {
	      __dpmi_regs user_regs;
	      int parmcount = CX;

	      movedata (sel, DI_OR_EDI,
			gdt_data32_sel, (word32)&user_regs,
			sizeof (user_regs));

	      if (user_regs.x.ss == 0 && user_regs.x.sp == 0)
		{
		  user_regs.x.ss = current_client_handle;
		  user_regs.x.sp = REAL_MODE_STACK_SIZE;
		}

	      /* Check that we have room on the stack for parameters and
		 a little extra.  */
	      if (2 * (parmcount + 10) >= user_regs.x.sp)
		ERROR (DPMI_ERROR_INVALID_VALUE);

	      /* Copy user-parameters.  */
	      if (parmcount)
		{
		  void *data = LINEAR_TO_PTR ((user_regs.x.ss << 4) +
					      user_regs.x.sp - 2 * parmcount);
		  user_regs.x.sp -= 2 * parmcount;
		  movedata (ss, SP_OR_ESP,
			    gdt_data32_sel, (word32)data,
			    2 * parmcount);
		}

	      /* Make flags sane.  */
	      user_regs.x.flags = (user_regs.x.flags & FLAGS_USER) | 0x3002;

	      /* If pseudo-interrupt then fetch the handler's address.  */
	      if (AL == 0)
		{
		  i = BL;
		  user_regs.x.cs = *((word16 *) LINEAR_TO_PTR (i * 4 + 2));
		  user_regs.x.ip = *((word16 *) LINEAR_TO_PTR (i * 4));
		}

	      /* FIXME: This is bogus with respect to user parameters.  */
	      server_goto ((AL == 1) ? 2 : 1, &user_regs);

	      /* Restore registers except for ss:sp and cs:ip.  */
	      movedata (gdt_data32_sel, (word32)&user_regs,
			sel, DI_OR_EDI,
			sizeof (user_regs) - 8);
	      BARRIER ();
	      return;
	    }
	  else
	    ERROR (DPMI_ERROR_INVALID_SELECTOR);

	case DPMI_ALLOCATE_CALLBACK - 0x0300:  /* 0.9 */
	  {
	    word16 seg, ofs;
	    int err = callback_allocate (ds, SI_OR_ESI,
					 es, DI_OR_EDI,
					 &seg, &ofs);

	    if (err) ERROR (err);
	    CX = seg;
	    DX = ofs;
	    if (DEBUG_TEST (DEBUG_CALLBACK))
	      eprintf ("Callback allocated at %04x:%04x, "
		       "entry=%04x:%08x, data=%04x:%08x.\r\n",
		       seg, ofs,
		       ds, SI_OR_ESI,
		       es, DI_OR_EDI);
	    BARRIER ();
	    return;
	  }

	case DPMI_FREE_CALLBACK - 0x0300:  /* 0.9 */
	  {
	    int err = callback_free (CX, DX);

	    if (err) ERROR (err);
	    if (DEBUG_TEST (DEBUG_CALLBACK))
	      eprintf ("Callback at %04x:%04x freed.\r\n", CX, DX);
	    BARRIER ();
	    return;
	  }

	case DPMI_GET_STATE_ROUTINES - 0x0300:  /* 0.9 */
	  AX = 0;
	  SI = gdt_savestate_sel;
	  SET_DI_OR_EDI (0);  /* Value irrelevant -- call gate.  */
	  BX = current_client_handle;
	  CX = 0;  /* FIXME */
	  BARRIER ();
	  return;

	case DPMI_GET_RAW_SWITCH_ROUTINES - 0x0300:  /* 0.9 */
	  SI = gdt_goreal_sel;
	  SET_DI_OR_EDI (0);  /* Value irrelevant -- call gate.  */
	  BX = current_client_handle;
	  CX = 0;  /* FIXME */
	  BARRIER ();
	  return;

	default: goto bad_function;
	}
    /* -------------------------------------------------------------------- */
    case 0x04: /* Server info group.  */
      switch (AL)
	{
	case DPMI_GET_VERSION - 0x0400:  /* 0.9 */
	  AX = (DPMI_VERSION_MAJOR << 8) | DPMI_VERSION_MINOR;
	  BX = DPMI_MASK_32
	    | (vcpi_present ? DPMI_MASK_REFL_V86 : DPMI_MASK_REFL_REAL)
	      | DPMI_MASK_NO_VIRT_MEM;  /* FIXME, i.e. support it.  */
	  CL = cpu;
	  DX = 0xffff;  /* FIXME */
	  BARRIER ();
	  return;

	case DPMI_GET_CAPABILITIES - 0x0400:  /* 1.0 */
	  sel = es;
	  if (SEL_LDT_INSPECTABLE_P (sel) && __cpu_verw (sel))
	    {
	      word32 offset = DI_OR_EDI;

	      /* FIXME: check when VM is imlemented.  */
	      AX = (DPMI_MASK_PAGE_DIRTY |
		    DPMI_MASK_NO_EXCEPTION_RESTART |
		    DPMI_MASK_DEVICE_MAPPING |
		    DPMI_MASK_1M_MAPPING |
		    DPMI_MASK_DEMAND_ZERO_FILL |
		    DPMI_MASK_WRITE_PROTECT_CLIENT |
		    DPMI_MASK_NO_WRITE_PROTECT_HOST);
	      DX = CX = 0;  /* Reserved.  */
	      _farsetsel (sel);
	      _farnspokeb (offset++, DPMI_OEM_VERSION_MAJOR);
	      _farnspokeb (offset++, DPMI_OEM_VERSION_MINOR);
	      movedata (gdt_data32_sel, (word32)DPMI_OEM_VENDOR,
			sel, offset,
			1 + strlen (DPMI_OEM_VENDOR));
	      BARRIER ();
	      return;
	    }
	  else
	    ERROR (DPMI_ERROR_INVALID_SELECTOR);

	default: goto bad_function;
	}
    /* -------------------------------------------------------------------- */
    case 0x05: /* Memory group.  */
      switch (AL)
	{
	case DPMI_GET_MEMORY_INFORMATION - 0x0500:  /* 0.9 (old) */
	  sel = es;
	  if (SEL_LDT_INSPECTABLE_P (sel) && __cpu_verw (sel))
	    {
	      __dpmi_free_mem_info info;
	      int free_pages;

	      memset (&info, -1, sizeof (info));
	      memory_info (&free_pages);
	      info.largest_available_free_block_in_bytes
		= free_pages * 1023 / 1024;  /* Account for page tables.  */
	      movedata (gdt_data32_sel, (word32)&info,
			sel, DI_OR_EDI,
			sizeof (info));
	      BARRIER ();
	      return;
	    }
	  else
	    ERROR (DPMI_ERROR_INVALID_SELECTOR);

	case DPMI_ALLOCATE_MEMORY - 0x0500:  /* 0.9 */
	  {
	    word32 linear, handle;
	    word32 size = (BX << 16) | CX;
	    int err = client_allocate_memory (size, &linear, &handle, 1);

	    if (err) ERROR (err);
	    BX = linear >> 16;
	    CX = linear;
	    SI = handle >> 16;
	    DI = handle;
	    if (DEBUG_TEST (DEBUG_MEMORY))
	      eprintf ("Committed memory allocated.  "
		       "Size=%08x, linear=%08x, handle=%08x.\r\n",
		       size, linear, handle);
	    BARRIER ();
	    return;
	  }

	case DPMI_FREE_MEMORY - 0x0500:  /* 0.9 */
	  {
	    word32 handle = (SI << 16) | DI;
	    int err = client_free_memory (handle);

	    if (err) ERROR (err);
	    if (DEBUG_TEST (DEBUG_MEMORY))
	      eprintf ("Memory with handle %08x freed.\r\n", handle);
	    BARRIER ();
	    return;
	  }

	case DPMI_RESIZE_MEMORY - 0x0500:  /* 0.9 */
	  {
	    word32 new_size = (BX << 16) | CX;
	    word32 old_handle = (SI << 16) | DI;
	    word32 new_handle = old_handle;
	    word32 new_linear;
	    int err = client_resize_memory (&new_handle, &new_linear,
					    new_size, 1);
	    if (err) ERROR (err);
	    if (DEBUG_TEST (DEBUG_MEMORY))
	      eprintf ("Memory resized.  Handle %08x->%08x, "
		       "size=%08x, linear=%08x.\r\n",
		       old_handle, new_handle,
		       new_size,
		       new_linear);
	    BX = new_linear >> 16;
	    CX = new_linear;
	    SI = new_handle >> 16;
	    DI = new_handle;
	    BARRIER ();
	    return;
	  }

	case DPMI_ALLOCATE_LINEAR - 0x0500:  /* 1.0 */
	case DPMI_RESIZE_LINEAR - 0x0500:  /* 1.0 */
	case DPMI_GET_PAGE_ATTRIBUTES - 0x0500:  /* 1.0 */
	case DPMI_SET_PAGE_ATTRIBUTES - 0x0500:  /* 1.0 */
	case DPMI_MAP_DEVICE - 0x0500:  /* 1.0 optional */
	case DPMI_MAP_1MB - 0x0500:  /* 1.0 optional */
	  ERROR (DPMI_ERROR_UNSUPPORTED);

	case DPMI_GET_MEMORY_BLOCK_INFO - 0x0500:  /* 1.0 */
	  {
	    word32 handle = (SI << 16) | DI;
	    word32 size, linear;
	    int err = client_memory_get_info (handle, &size, &linear);

	    if (err) ERROR (err);
	    SI = size >> 16;
	    DI = size;
	    BX = linear >> 16;
	    CX = linear;
	    BARRIER ();
	    return;
	  }

	case DPMI_GET_MEMORY_INFO - 0x0500:  /* 1.0 */
	  ERROR (DPMI_ERROR_UNSUPPORTED);

	default: goto bad_function;
	}
    /* -------------------------------------------------------------------- */
    case 0x06: /* Paging control group.  */
      switch (AL)
	{
	case DPMI_LOCK_LINEAR - 0x0600:  /* 0.9 */
	case DPMI_UNLOCK_LINEAR - 0x0600:  /* 0.9 */
	case DPMI_MARK_1MB_PAGEABLE - 0x0600:  /* 0.9 */
	case DPMI_RELOCK_1MB - 0x0600:  /* 0.9 */
	  ERROR (DPMI_ERROR_UNSUPPORTED);

	case DPMI_GET_PAGE_SIZE - 0x0600:  /* 0.9 */
	  BX = PAGE_SIZE >> 16;
	  CX = PAGE_SIZE;
	  BARRIER ();
	  return;

	default: goto bad_function;
	}
    /* -------------------------------------------------------------------- */
    case 0x07: /* Paging hints group.  */
      switch (AL)
	{
	case DPMI_MARK_PAGING_CANDIDATE - 0x0700:  /* 0.9 */
	case DPMI_DISCARD_PAGES - 0x0700:  /* 0.9 */
	  ERROR (DPMI_ERROR_UNSUPPORTED);
	default: goto bad_function;
	}
    /* -------------------------------------------------------------------- */
    case 0x08: /* Physical mapping group.  */
      switch (AL)
	{
	case DPMI_PHYSICAL_ADDRESS_MAPPING - 0x0800:  /* 0.9 */
	case DPMI_FREE_PHYSICAL_ADDRESS_MAPPING - 0x0800:  /* 1.0 */
	  ERROR (DPMI_ERROR_UNSUPPORTED);
	default: goto bad_function;
	}
    /* -------------------------------------------------------------------- */
    case 0x09: /* Interrupt flag group.  */
      switch (AL)
	{
	case DPMI_DISABLE_INTERRUPTS - 0x0900:  /* 0.9 */
	  AL = (eflags >> FLAG_IF) & 1;
	  eflags &= ~(1 << FLAG_IF);
	  BARRIER ();
	  return;

	case DPMI_ENABLE_INTERRUPTS - 0x0900:  /* 0.9 */
	  AL = (eflags >> FLAG_IF) & 1;
	  eflags |= (1 << FLAG_IF);
	  BARRIER ();
	  return;

	case DPMI_GET_INTERRUPT_STATE - 0x0900:  /* 0.9 */
	  AL = (eflags >> FLAG_IF) & 1;
	  BARRIER ();
	  return;

	default: goto bad_function;
	}
    /* -------------------------------------------------------------------- */
    case 0x0a: /* API group.  */
      switch (AL)
	{
	case DPMI_GET_API_ENTRY_09 - 0x0a00:  /* 0.9 (old) */
	  /* Superseeded by 0x2f, function 0x168a.  */
	  ERROR (DPMI_ERROR_UNSUPPORTED);
	default: goto bad_function;
	}
    /* -------------------------------------------------------------------- */
    case 0x0b: /* Breakpoint group.  */
      switch (AL)
	{
	case DPMI_SET_BREAKPOINT - 0x0b00:  /* 0.9 */
	case DPMI_CLEAR_BREAKPOINT - 0x0b00:  /* 0.9 */
	case DPMI_GET_BREAKPOINT_STATE - 0x0b00:  /* 0.9 */
	case DPMI_RESET_BREAKPOINT_STATE - 0x0b00:  /* 0.9 */
	  ERROR (DPMI_ERROR_UNSUPPORTED);
	default: goto bad_function;
	}
    /* -------------------------------------------------------------------- */
    case 0x0c: /* TSR group.  */
      switch (AL)
	{
	case DPMI_INSTALL_RESIDENT - 0x0c00:  /* 1.0 */
	case DPMI_TSR - 0x0c00:  /* 1.0 */
	  ERROR (DPMI_ERROR_UNSUPPORTED);
	default: goto bad_function;
	}
    /* -------------------------------------------------------------------- */
    case 0x0d: /* Shared memory group.  */
      switch (AL)
	{
	case DPMI_ALLOCATE_SHARED_MEMORY - 0x0d00:  /* 1.0 */
	case DPMI_FREE_SHARED_MEMORY - 0x0d00:  /* 1.0 */
	case DPMI_SERIALIZE_SHARED_MEMORY - 0x0d00:  /* 1.0 */
	case DPMI_UNSERIALIZE_SHARED_MEMORY - 0x0d00:  /* 1.0 */
	  ERROR (DPMI_ERROR_UNSUPPORTED);
	default: goto bad_function;
	}
    /* -------------------------------------------------------------------- */
    case 0x0e: /* FPU group.  */
      switch (AL)
	{
	case DPMI_GET_FPU_STATUS - 0x0e00:  /* 1.0 */
	case DPMI_SET_FPU_EMULATION - 0x0e00:  /* 1.0 */
	  ERROR (DPMI_ERROR_UNSUPPORTED);
	default: goto bad_function;
	}
    /* -------------------------------------------------------------------- */
    default:
    bad_function:
      ERROR (DPMI_ERROR_UNSUPPORTED);
    error:
      eflags |= (1 << FLAG_CF);
      BARRIER ();
      return;
    }
}
/* ---------------------------------------------------------------------- */
