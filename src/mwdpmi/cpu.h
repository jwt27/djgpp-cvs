/* ---------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                 */
/* Distributed under the GPL, see COPYING for details.                    */
/* ---------------------------------------------------------------------- */
/* Pseudo descriptors as used by lidt and lgdt instructions.  */

typedef struct pseudo_descriptor
{
  word16 limit __attribute__ ((packed));
  word32 base __attribute__ ((packed));
} pseudo_descriptor;
/* ---------------------------------------------------------------------- */
typedef struct ptr3232
{
  word32 offset, sel;
} ptr3232;
/* ---------------------------------------------------------------------- */
/* Descriptor entry describing an LDT.  */

typedef struct ldt_descriptor
{
  word16 limit0;
  word16 base0;
  word8  base1;
  word8  stype;
  word8  limit1;
  word8  base2;
} ldt_descriptor;
/* ---------------------------------------------------------------------- */
/* Layout of a code/data descriptor.  */

typedef struct user_descriptor
{
  word16 limit0;
  word16 base0;
  word8  base1;
  word8  stype;
  word8  limit1;
  word8  base2;
} user_descriptor;
/* ---------------------------------------------------------------------- */
/* Layout of a TSS descriptor.  */

typedef struct tss_descriptor
{
  word16 limit0;
  word16 base0;
  word8  base1;
  word8  stype;			/* PDD010B1 */
  word8  limit1;		/* G00?LLLL */
  word8  base2;
} tss_descriptor;
/* ---------------------------------------------------------------------- */
/* Descriptor entry for IDT.  */

typedef struct idt_entry
{
  word16 offset0;
  word16 selector;
  word16 stype;
  word16 offset1;
} idt_entry;
/* ---------------------------------------------------------------------- */
typedef struct tss_segment
{
  word16 link, res_02;
  word32 esp0;
  word16 ss0, res_0a;
  word32 esp1;
  word16 ss1, res_12;
  word32 esp2;
  word16 ss2, res_1a;
  word32 cr3;
  word32 eip;
  word32 eflags;
  word32 eax;
  word32 ecx;
  word32 edx;
  word32 ebx;
  word32 esp;
  word32 ebp;
  word32 esi;
  word32 edi;
  word16 es, res_4a;
  word16 cs, res_4e;
  word16 ss, res_52;
  word16 ds, res_56;
  word16 fs, res_5a;
  word16 gs, res_5e;
  word16 ldt_seg, res_62;
  word16 tss_trap;
  word16 io_map;
} tss_segment;
/* ---------------------------------------------------------------------- */


/* ------------------------------------------------------------------------ */
/* Get Task Register.  */

static __inline__ unsigned short
__cpu_get_tr (void)
{
  unsigned short tr;
  asm ("str %0" : "=g" (tr));
  return tr;
}
/* ------------------------------------------------------------------------ */
/* Set Task Register.  */

static __inline__ void
__cpu_set_tr (unsigned short tr)
{
  asm volatile ("ltr %0" : /* No output */ : "r" (tr));
}
/* ------------------------------------------------------------------------ */
/* Get Local Descriptor Table Register.  */

static __inline__ unsigned short
__cpu_get_ldtr (void)
{
  unsigned short ldtr;
  asm ("sldt %0" : "=g" (ldtr));
  return ldtr;
}
/* ------------------------------------------------------------------------ */
/* Set Local Descriptor Table Register.  */

static __inline__ void
__cpu_set_ldtr (unsigned short ldtr)
{
  asm volatile ("lldt %0" : /* No output */ : "r" (ldtr));
}
/* ------------------------------------------------------------------------ */
/* Clear New Task flag (unprivileged).  */

static __inline__ void __cpu_clear_NT (void)
{
  asm volatile ("pushfl ; andb $0xbf, 1(%esp) ; popfl");
}
/* ------------------------------------------------------------------------ */
static __inline__ void
__cpu_get_idt (pseudo_descriptor *idt)
{
  asm ("sidt %0" : "=m" (*idt));
}
/* ------------------------------------------------------------------------ */
static __inline__ void
__cpu_set_idt (pseudo_descriptor *idt)
{
  asm volatile ("lidt %0" : /* No output.  */ : "m" (*idt));
}
/* ------------------------------------------------------------------------ */
/* Get Control/Debug/Test registers.  */

#define GET_REG(reg) \
  static __inline__ unsigned __cpu_get_##reg (void) \
  { unsigned res; asm ("movl %%" #reg ",%0" : "=r" (res)); return res; }

GET_REG(cr0) GET_REG(cr1) GET_REG(cr2) GET_REG(cr3)
GET_REG(dr0) GET_REG(dr1) GET_REG(dr2) GET_REG(dr3)
GET_REG(dr4) GET_REG(dr5) GET_REG(dr6) GET_REG(dr7)
GET_REG(tr3) GET_REG(tr4) GET_REG(tr5) GET_REG(tr6) GET_REG(tr7)

#undef GET_REG
/* ------------------------------------------------------------------------ */
/* Set Control/Debug/Test registers.  */

#define SET_REG(reg) \
  static __inline__ void __cpu_set_##reg (unsigned val) \
  { asm volatile ("movl %0,%%" #reg : /* No output */ : "r" (val)); }

SET_REG(cr0) SET_REG(cr1) SET_REG(cr2) SET_REG(cr3)
SET_REG(dr0) SET_REG(dr1) SET_REG(dr2) SET_REG(dr3)
SET_REG(dr4) SET_REG(dr5) SET_REG(dr6) SET_REG(dr7)
SET_REG(tr3) SET_REG(tr4) SET_REG(tr5) SET_REG(tr6) SET_REG(tr7)

#undef SET_REG
/* ------------------------------------------------------------------------ */
static __inline__ int
__cpu_verr (unsigned short sel)
{
  int res = 0;
  asm ("verr	%1
	jnz	1f
	incl	%0
1:"
       : "=g" (res)
       : "g" (sel));
  return res;
}

static __inline__ int
__cpu_verw (unsigned short sel)
{
  int res = 0;
  asm ("verw	%1
	jnz	1f
	incl	%0
1:"
       : "=g" (res)
       : "g" (sel));
  return res;
}
/* ------------------------------------------------------------------------ */
/* Various instructions.  */

static __inline__ void __cpu_clts (void) { asm volatile ("clts"); }
static __inline__ void __cpu_hlt (void) { asm volatile ("hlt"); }
static __inline__ void __cpu_cli (void) { asm volatile ("cli"); }
static __inline__ void __cpu_sti (void) { asm volatile ("sti"); }
/* ------------------------------------------------------------------------ */
#define BARRIER() \
  do { \
    __asm__ volatile (\
	"# eflags,eip        : %0 %1\n\
	# cs,ds,es,fs,gs,ss : %2 %3 %4 %5 %6 %7"\
       : "=m" (eflags), "=m" (eip),\
	 "=m" (cs), "=m" (ds), "=m" (es), "=m" (fs), "=m" (gs), "=m" (ss));\
    __asm__ volatile (\
	"# eax,ebx,ecx,edx   : %0 %1 %2 %3\n\
	# esi,edi,ebp,esp   : %4 %5 %6 %7"\
       : "=m" (eax), "=m" (ebx), "=m" (ecx), "=m" (edx),\
	 "=m" (esi), "=m" (edi), "=m" (ebp), "=m" (esp));\
  } while (0)
/* ------------------------------------------------------------------------ */
