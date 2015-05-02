/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <errno.h>
#include <dpmi.h>
#include <go32.h>
#include <dos.h>

int uname(struct utsname *u)
{
  __dpmi_regs r;
  unsigned short dos_version;
  unsigned is_486_or_better;
  unsigned cpuid_support;
  unsigned cpuid_info;

  if (!u)
  {
    errno = EFAULT;
    return -1;
  }

  dos_version = _get_dos_version(1);
  strncpy(u->sysname, _os_flavor, sizeof(u->sysname) - 1);
  u->sysname[sizeof(u->sysname) - 1] = '\0';
  sprintf(u->release, "%d", dos_version >> 8);
  sprintf(u->version, "%02d", dos_version & 0xff);

  /* CPU detection code by Laurynas Biveinis            */
  /* Uses Phil Frisbie, Jr 386 and CPUID detection code */

  /* Let's check for 386. Intel says that 386 is unable to set or clear */
  /* value of 18 bit in EFLAGS (AC). So we toggle this bit and see if   */
  /* we succeed */
  asm volatile (
       "pushf;"
       "popl   %%eax;"
       "movl   %%eax, %%ebx;"
       "xorl   $0x40000, %%eax;"
       "pushl  %%eax;"
       "popf;"
       "pushf;"
       "popl   %%eax;"
       "cmpl   %%ebx, %%eax;"
       "jz     0f;"
       "movl   $1, %0;"               /* 80486+ present */
       "jmp    1f;"
       "0:"
       "movl   $0, %0;"               /* 80386 present */
       "1:"
       "pushl  %%ebx;"                /* get original EFLAGS */
       "popf;"                        /* restore EFLAGS */
       : "=g" (is_486_or_better)
       :
       : "eax", "ebx");
  if (is_486_or_better)
  {
      /* In the same way we checked for 386, we will check for CPUID now, */
      /* using 21 bit in EFLAGS (ID bit)      */
      asm volatile (
         "pushf;"                      /* get extended flags */
         "popl     %%eax;"
         "movl     %%eax, %%ebx;"       /* save current flags */
         "xorl     $0x200000, %%eax;"   /* toggle bit 21 */
         "pushl    %%eax;"              /* put new flags on stack */
         "popfl;"                       /* flags updated now in flags */
         "pushfl;"                      /* get extended flags */
         "popl     %%eax;"
         "xorl     %%ebx, %%eax;"       /* if bit 21 r/w then supports cpuid */
         "jz       0f;"
         "movl     $1, %0;"
         "jmp      1f;"
         "0:"
         "movl     $0, %0;"
         "1:"
         "pushl    %%ebx;"              /* Restore */
         "popfl;"                       /* original EFLAGS */
         : "=g" (cpuid_support)
         :
         : "%eax", "%ebx");
      if (cpuid_support)
      {
         /* Now we can use CPUID */
         asm volatile (
            "movl   $1, %%eax;"
            "cpuid;"
            : "=a" (cpuid_info)
            :
            : "%ebx", "%ecx", "%edx");
            /* What we need is instruction family info in 8-11 bits */
            switch ((cpuid_info & 0x780) >> 8)
            {
	       case 0x7: strcpy(u->machine, "i786"); break;
               case 0x6: strcpy(u->machine, "i686"); break;
               case 0x5: strcpy(u->machine, "i586"); break;
               case 0x4: strcpy(u->machine, "i486"); break;
            }
      }
      else
         strcpy(u->machine, "i486"); // i486 not supporting CPUID
  }
  else
     strcpy(u->machine, "i386");

  r.x.ax = 0x5e00;
  r.x.ds = __tb >> 4;
  r.x.dx = __tb & 15;
  __dpmi_int(0x21, &r);
  if ((r.x.flags & 1) || (r.h.ch == 0))
    strcpy(u->nodename, "pc");
  else
  {
    int i = 8;
    dosmemget(__tb, 8, u->nodename);
    do {
      u->nodename[i--] = 0;
    } while (i && u->nodename[i] <= ' ');
  }
  return 0;
}
