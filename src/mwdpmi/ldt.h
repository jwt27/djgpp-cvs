/* ---------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                 */
/* Distributed under the GPL, see COPYING for details.                    */
/* ---------------------------------------------------------------------- */

#define LDT_TYPE_FREE 0
#define LDT_TYPE_USER 1
#define LDT_TYPE_SEGMENT 2
#define LDT_TYPE_DOS 3
#define LDT_TYPE_DOS_CONT 4
#define LDT_TYPE_SYSTEM 5

#define LDT_FREE_P(i) (current_client->ldt_types[i] == LDT_TYPE_FREE)
#define LDT_SEL(i) (((i) << 3) | 4 | RING)

#define SEL_LDT_MODIFYABLE_P(sel) \
  (((sel) & 7) == (4 | RING) && \
   (sel) < 8 * ldt_selector_count && \
   current_client->ldt_types[(sel) >> 3] == LDT_TYPE_USER)

#define SEL_LDT_INSPECTABLE_P(sel) \
  (((sel) & 7) == (4 | RING) && \
   (sel) < 8 * ldt_selector_count && \
   current_client->ldt_types[(sel) >> 3] != LDT_TYPE_FREE)

#define SEL_LDT_FREE_P(sel) \
  (((sel) & 7) == (4 | RING) && \
   (sel) < 8 * ldt_selector_count && \
   current_client->ldt_types[(sel) >> 3] == LDT_TYPE_FREE)


#define LDT_SET_BASE(i,b) \
  (current_client->ldt[i].base0 = (word16)(b), \
   current_client->ldt[i].base1 = (word8)((b) >> 16), \
   current_client->ldt[i].base2 = (word8)((b) >> 24))

#define LDT_GET_BASE(i) \
  (current_client->ldt[i].base0 | \
   (current_client->ldt[i].base1 << 16) | \
   (current_client->ldt[i].base2 << 24))

#define LDT_SET_LIMIT(i,l,g) \
  (current_client->ldt[i].limit0 = (word16)(l), \
   current_client->ldt[i].limit1 = \
     (current_client->ldt[i].limit1 & 0x50) | ((g) << 7) | (((l) >> 16) & 0xf))

#define LDT_GET_LIMIT(i) \
  ((current_client->ldt[i].limit0 | \
    ((0x0f & current_client->ldt[i].limit1) << 16)) << \
   ((current_client->ldt[i].limit1 & 0x80) ? PAGE_SIZE_LOG : 0))

#define LDT_SET_BIT(i,b) \
  do { \
    if ((b) == 16) \
      current_client->ldt[i].limit1 &= ~0x40; \
    else \
      current_client->ldt[i].limit1 |= 0x40; \
     } while (0)

#define LDT_SET_TYPE(i,t,b) \
  do { \
       current_client->ldt[i].stype = 0x90 | (RING << 5) | (t); \
       LDT_SET_BIT ((i),(b)); \
     } while (0)

#define LDT_PRINT(i) \
  eprintf ("%04x: %02x %02x %02x %02x %02x %02x %02x %02x\r\n", \
	   LDT_SEL (i), \
	   (int)(((unsigned char *)&(current_client->ldt[i]))[0]), \
	   (int)(((unsigned char *)&(current_client->ldt[i]))[1]), \
	   (int)(((unsigned char *)&(current_client->ldt[i]))[2]), \
	   (int)(((unsigned char *)&(current_client->ldt[i]))[3]), \
	   (int)(((unsigned char *)&(current_client->ldt[i]))[4]), \
	   (int)(((unsigned char *)&(current_client->ldt[i]))[5]), \
	   (int)(((unsigned char *)&(current_client->ldt[i]))[6]), \
	   (int)(((unsigned char *)&(current_client->ldt[i]))[7]))
