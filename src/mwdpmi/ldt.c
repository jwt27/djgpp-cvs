/* ---------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                 */
/* Distributed under the GPL, see COPYING for details.                    */
/* ---------------------------------------------------------------------- */
#include "code32.h"
#include "clients.h"
#include "ldt.h"
/* ---------------------------------------------------------------------- */
/* Allocate a number of contiguos descriptors in the LDT returning the
   index (not the selector) of the first, or -1 for failure.  */

int
ldt_allocate (int count)
{
  int first;

  /* The 16 first are reserved for other purposed per specs.  */
  for (first = 0x10; first + count <= ldt_selector_count; )
    {
      int test = first + count - 1;

      while (LDT_FREE_P (test))
	{
	  if (test > first)
	    test--;
	  else
	    {
	      for (test = first; test < first + count; test++)
		{
		  /* RW data, byte granular, present, base=limit=0 */
		  current_client->ldt_types[test] = LDT_TYPE_USER;
		  LDT_SET_BASE (test, 0x00000000);
		  LDT_SET_LIMIT (test, 0x00000, 0);
		  current_client->ldt[test].limit0 = 0;
		  if (current_client->is_32bit)
		    current_client->ldt[test].limit1 |= 0x40;
		  current_client->ldt[test].stype = (RING << 5) | 0x92;
		}
	      return first;
	    }
	}
      first = test + 1;
    }
  return -1;
}
/* ---------------------------------------------------------------------- */
#ifdef DEBUG
void
ldt_print (void)
{
  int i, typ;

  for (i = 0; i < ldt_selector_count; i++)
    {
      switch (current_client->ldt_types[i])
	{
	case LDT_TYPE_FREE:
	  /* Nothing.  */
	  break;
	case LDT_TYPE_USER:
	  eprintf ("%04x: user, ", LDT_SEL (i));
	  typ = current_client->ldt[i].stype & 0x1f;
	  if (typ & 0x10)
	    {
	      if (typ & 0x08)
		eprintf ("%s-bit code, %s, ",
			 (current_client->ldt[i].limit1 & 0x40) ? "32" : "16",
			 (typ & 2) ? "readable" : "no-read");
	      else
		eprintf ("%s-bit data, %s, %s, ",
			 (current_client->ldt[i].limit1 & 0x40) ? "32" : "16",
			 (typ & 2) ? "r/w" : "ro",
			 (typ & 4) ? "e-down" : "e-up");
	      eprintf ("base=%08x, limit=%08x\r\n",
		       LDT_GET_BASE (i),
		       LDT_GET_LIMIT (i));
	    }
	  else
	    eprintf ("bogus\r\n");
	  break;
	case LDT_TYPE_SEGMENT:
	  eprintf ("%04x: real-mode segment %04x\r\n",
		   LDT_SEL (i),
		   LDT_GET_BASE (i) >> 4);
	  break;
	case LDT_TYPE_DOS:
	  eprintf ("%04x: allocated dos memory at %04x:0000, length=0x%x\r\n",
		   LDT_SEL (i),
		   LDT_GET_BASE (i) >> 4,
		   LDT_GET_LIMIT (i) + 1);
	  break;
	case LDT_TYPE_DOS_CONT:
	  eprintf ("%04x: allocated dos memory (continuation)\r\n",
		   LDT_SEL (i));
	  break;
	case LDT_TYPE_SYSTEM:
	  eprintf ("%04x: system provided (environment, etc.)\r\n",
		   LDT_SEL (i));
	  break;
	default:
	  eprintf ("%04x: bogus type.\r\n",
		   LDT_SEL (i));
	}
    }
}
#endif
/* ---------------------------------------------------------------------- */
