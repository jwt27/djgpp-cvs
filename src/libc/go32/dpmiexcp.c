/* Copyright (C) 1994, 1995 Charles Sandmann (sandmann@clio.rice.edu)
   Exception handling and basis for signal support for DJGPP V2.0
   This software may be freely distributed, no warranty. */

#include <libc/stubs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <io.h>
#include <libc/farptrgs.h>
#include <dpmi.h>
#include <go32.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>
#include <crt0.h>
#include <pc.h>
#include <sys/exceptn.h>
#include <sys/nearptr.h>		/* For DS base/limit info */
#include <libc/internal.h>

#define err(x) _write(STDERR_FILENO, x, sizeof(x)-1)

extern unsigned end __asm__ ("end");
static unsigned char old_video_mode = 3;
static int cbrk_vect = 0x1b;		/* May be 0x06 for PC98 */

/* These are all defined in exceptn.S and only used here */
extern int __djgpp_exception_table;
extern int __djgpp_npx_hdlr;
extern int __djgpp_kbd_hdlr;
extern int __djgpp_kbd_hdlr_pc98;
extern int __djgpp_iret, __djgpp_i24;
extern void __djgpp_cbrk_hdlr(void);
extern int __djgpp_hw_lock_start, __djgpp_hw_lock_end;
extern __dpmi_paddr __djgpp_old_kbd;

static void
itox(int v, int len)
{
  static char buf[9]; /* startup default to all NULLs */
  char *bp = buf+7;
  while (len)
  {
    int d = v & 15;
    v >>= 4;
    *bp = d + '0';
    if (d > 9)
      *bp += 'a' - '9' - 1;
    bp--;
    len--;
  }
  _write(STDERR_FILENO, bp+1, 7-(bp-buf));
}

static int
except_to_sig(int excep)
{
  switch(excep)
  {
  case 5:
  case 8:
  case 9:
  case 11:
  case 12:
  case 13:
  case 14:
    return SIGSEGV;
  case 0:
  case 4:
  case 16:
    return SIGFPE;
  case 1:
  case 3:
    return SIGTRAP;
  case 7:
    return SIGNOFP;
  default:
    if(excep == 0x75)		/* HW int to fake exception values hardcoded in exceptn.S */
      return SIGFPE;
    else if(excep == 0x78)
      return SIGTIMR;
    else if(excep == 0x79 || excep == 0x1b)
      return SIGINT;
    else
      return SIGILL;
  }
}

static void
show_call_frame(void)
{
  unsigned *vbp, *vbp_new, *tos;
  unsigned veip;
  int max=0;

  tos = (unsigned *)__djgpp_selector_limit;
  vbp = (unsigned *)__djgpp_exception_state->__ebp;
  err("Call frame traceback EIPs:\r\n  0x");
  itox(__djgpp_exception_state->__eip, 8);
  while (((unsigned)vbp >= __djgpp_exception_state->__esp) && (vbp >= &end) && (vbp < tos))
  {
    vbp_new = (unsigned *)*vbp;
    if (vbp_new == 0)
      break;
    veip = *(vbp + 1);
    err("\r\n  0x");
    itox(veip, 8);
    vbp = vbp_new;
    if (++max == 10)
      break;
  } 
  err("\r\n");
}

static const char *exception_names[] = {
  "Division by Zero",
  "Debug",
  "NMI",
  "Breakpoint",
  "Overflow",
  "Bounds Check",
  "Invalid Opcode",
  "Coprocessor not available",
  "Double Fault",
  "Coprocessor overrun",
  "Invalid TSS",
  "Segment Not Present",
  "Stack Fault",
  "General Protection Fault",
  "Page fault",
  0,
  "Coprocessor Error",
  "Alignment Check"
};

static char has_error[] = {0,0,0,0,0,0,0,0 ,1,0,1,1,1,1,1,0 ,0,1 };

#define EXCEPTION_COUNT (sizeof(exception_names)/sizeof(exception_names[0]))

static void
dump_selector(const char *name, int sel)
{
  unsigned long base;
  unsigned limit;
  write(STDERR_FILENO, name, 2);
  err(": sel="); itox(sel, 4);
  if (sel) {
    if (__dpmi_get_segment_base_address(sel, &base))
    {
      err("  invalid");
    }
    else
    {
      err("  base="); itox(base, 8);
      limit = __dpmi_get_segment_limit(sel);
      err("  limit="); itox(limit, 8);
    }
  }
  err("\r\n");
}

static void
do_faulting_finish_message(void)
{
  const char *en;
  unsigned long signum = __djgpp_exception_state->__signum;
  int i;
  
  /* check video mode for original here and reset (not if PC98) */
  if(ScreenPrimary != 0xa0000 && _farpeekb(_dos_ds, 0x449) != old_video_mode) {
    asm("pusha;movzbl _old_video_mode,%eax; int $0x10;popa;nop");
  }
  en = (signum >= EXCEPTION_COUNT) ? 0 : 
  exception_names[signum];
  if (signum == 0x75)
    en = "Floating Point exception";
  if (signum == 0x1b)
    en = "Control-Break Pressed";
  if (signum == 0x79)
    en = "Control-C Pressed";
  if (en == 0)
  {
    err("Exception ");
    itox(signum, 2);
    err(" at eip=");
    itox(__djgpp_exception_state->__eip, 8);
  }
  else
  {
    _write(STDERR_FILENO, en, strlen(en));
    err(" at eip=");
    itox(__djgpp_exception_state->__eip, 8);
  }
  if (signum == 0x79)
  {
    err("\r\n");
    exit(-1);
  }
  if (signum <= EXCEPTION_COUNT && has_error[signum])
  {
    unsigned int errorcode = __djgpp_exception_state->__sigmask & 0xffff;
    if(errorcode)
    {
      err(", error="); itox(errorcode, 4);
    }
  }
  err("\r\neax="); itox(__djgpp_exception_state->__eax, 8);
  err(" ebx="); itox(__djgpp_exception_state->__ebx, 8);
  err(" ecx="); itox(__djgpp_exception_state->__ecx, 8);
  err(" edx="); itox(__djgpp_exception_state->__edx, 8);
  err(" esi="); itox(__djgpp_exception_state->__esi, 8);
  err(" edi="); itox(__djgpp_exception_state->__edi, 8);
  err("\r\nebp="); itox(__djgpp_exception_state->__ebp, 8);
  err(" esp="); itox(__djgpp_exception_state->__esp, 8);
  err(" program=");
  for (i=0; __dos_argv0[i]; i++);
  write(STDERR_FILENO, __dos_argv0, i);
  err("\r\n");
  dump_selector("cs", __djgpp_exception_state->__cs);
  dump_selector("ds", __djgpp_exception_state->__ds);
  dump_selector("es", __djgpp_exception_state->__es);
  dump_selector("fs", __djgpp_exception_state->__fs);
  dump_selector("gs", __djgpp_exception_state->__gs);
  dump_selector("ss", __djgpp_exception_state->__ss);
  err("\r\n");
  if (__djgpp_exception_state->__cs == _my_cs())
    show_call_frame();
  exit(-1);
}

typedef void (*SignalHandler) (int);

static SignalHandler signal_list[SIGMAX];	/* SIG_DFL = 0 */

SignalHandler
signal(int sig, SignalHandler func)
{
  SignalHandler temp;
  if(sig <= 0 || sig > SIGMAX || sig == SIGKILL)
  {
    errno = EINVAL;
    return SIG_ERR;
  }
  temp = signal_list[sig - 1];
  signal_list[sig - 1] = func;
  return temp;
}

static const char signames[] = "ABRTFPE ILL SEGVTERMALRMHUP INT KILLPIPEQUITUSR1USR2NOFPTRAP";

int
raise(int sig)
{
  SignalHandler temp;
  if(sig <= 0)
    return -1;
  if(sig > SIGMAX)
    return -1;
  temp = signal_list[sig - 1];
  if(temp == (SignalHandler)SIG_IGN)
    return 0;			/* Ignore it */
  if(temp == (SignalHandler)SIG_DFL)
  {
  traceback_exit:
    if (sig >= SIGABRT && sig <= SIGTRAP)
    {
      err("Exiting due to signal SIG");
      _write(STDERR_FILENO, signames+(sig-SIGABRT)*4, 4);
    }
    else
    {
      err("Exiting due to signal 0x");
      itox(sig, 4);
    }
    err("\r\n");
    if(__djgpp_exception_state_ptr)
      do_faulting_finish_message();	/* Exits, does not return */
    exit(-1);
  }
  if((unsigned)temp < 4096 || temp > (SignalHandler)&end)
  {
    err("Bad signal handler, ");
    goto traceback_exit;
  }
  temp(sig);
  return 0;
}

/* This routine must call exit() or jump changing stacks.  This routine is
   the basis for traceback generation, core creation, signal handling. */
void
__djgpp_exception_processor(void)
{
  int sig;
  
  sig = except_to_sig(__djgpp_exception_state->__signum);
  raise(sig);
  if(__djgpp_exception_state->__signum >= EXCEPTION_COUNT) /* Not exception so continue OK */
    longjmp(__djgpp_exception_state, __djgpp_exception_state->__eax);
  /* User handler did not exit or longjmp, we must exit */
  err("Cannot continue from exception, exiting due to signal ");
  itox(sig, 4);
  err("\r\n");
  do_faulting_finish_message();
}

static __dpmi_paddr except_ori[EXCEPTION_COUNT];
static __dpmi_paddr kbd_ori;
static __dpmi_paddr npx_ori;
static __dpmi_raddr cbrk_ori,cbrk_rmcb;
static char cbrk_hooked = 0;
static __dpmi_regs cbrk_regs;

/* Routine toggles ALL the exceptions.  Used around system calls, at exit. */

void
__djgpp_exception_toggle(void)
{
  __dpmi_paddr except;
  int i;
  
  for(i=0; i < EXCEPTION_COUNT; i++)
  {
    __dpmi_get_processor_exception_handler_vector(i, &except);
    if(i != 2 || (_crt0_startup_flags & _CRT0_FLAG_NMI_SIGNAL))
      __dpmi_set_processor_exception_handler_vector(i, &except_ori[i]);
    except_ori[i] = except;
  }
  __dpmi_get_protected_mode_interrupt_vector(0x75, &except);
  __dpmi_set_protected_mode_interrupt_vector(0x75, &npx_ori);
  npx_ori = except;
  __dpmi_get_protected_mode_interrupt_vector(9, &except);
  __dpmi_set_protected_mode_interrupt_vector(9, &kbd_ori);
  kbd_ori = except;
  if(cbrk_hooked) {
    __dpmi_set_real_mode_interrupt_vector(cbrk_vect, &cbrk_ori);
    __dpmi_free_real_mode_callback(&cbrk_rmcb);
    cbrk_hooked = 0;
  } else {
    __dpmi_get_real_mode_interrupt_vector(cbrk_vect, &cbrk_ori);
    __dpmi_allocate_real_mode_callback(__djgpp_cbrk_hdlr, &cbrk_regs, &cbrk_rmcb);
    __dpmi_set_real_mode_interrupt_vector(cbrk_vect, &cbrk_rmcb);
    cbrk_hooked = 1;
  }
}

void
__djgpp_exception_setup(void)
{
  __dpmi_paddr except;
  __dpmi_meminfo lockmem;
  int i;

  for (i = 0; i < SIGMAX; i++)
    signal_list[i] = (SignalHandler)SIG_DFL;

  /* app_DS only used when converting HW interrupts to exceptions */
  asm("mov %ds,___djgpp_app_DS");
  asm("mov %ds,___djgpp_our_DS");
  __djgpp_dos_sel = _dos_ds;

  /* lock addresses which may see HW interrupts */
  lockmem.address = __djgpp_base_address + (unsigned) &__djgpp_hw_lock_start;
  lockmem.size = ((unsigned) &__djgpp_hw_lock_end
		  - (unsigned) &__djgpp_hw_lock_start);
  __dpmi_lock_linear_region(&lockmem);
  
  asm("mov %%cs,%0" : "=g" (except.selector) );
  except.offset32 = (unsigned) &__djgpp_exception_table;
  for(i=0; i < EXCEPTION_COUNT; i++)
  {
    except_ori[i] = except;	/* New value to set */
    except.offset32 += 4;	/* This is the size of push n, jmp */
  }
  kbd_ori.selector = npx_ori.selector = except.selector;
  npx_ori.offset32 = (unsigned) &__djgpp_npx_hdlr;
  if(ScreenPrimary != 0xa0000)
    kbd_ori.offset32 = (unsigned) &__djgpp_kbd_hdlr;
  else
  {
    kbd_ori.offset32 = (unsigned) &__djgpp_kbd_hdlr_pc98;
    cbrk_vect = 0x06;
    except.offset32 = (unsigned) &__djgpp_iret;		/* TDPMI98 bug */
    __dpmi_set_protected_mode_interrupt_vector(0x23, &except);
  }
  except.offset32 = (unsigned) &__djgpp_i24;
  __dpmi_set_protected_mode_interrupt_vector(0x24, &except);

  __dpmi_get_protected_mode_interrupt_vector(9, &__djgpp_old_kbd);
  __djgpp_exception_toggle();	/* Set new values & save old values */

  /* get original video mode and save */
  old_video_mode = _farpeekb(_dos_ds, 0x449);
}

int
__djgpp_set_ctrl_c(int enable)
{
  int oldenable = !(__djgpp_hwint_flags & 1);
  if(enable)
    __djgpp_hwint_flags &= ~1;
  else
    __djgpp_hwint_flags |= 1;
  return oldenable;
}

void __attribute__((noreturn))
_exit(int status)
{
  /* We need to restore hardware interrupt handlers even if somebody calls
     `_exit' directly, or else we crash the machine in nested programs.
     We only toggle the handlers if the original keyboard handler is intact
     (otherwise, they might have already toggled them).  */
  if (__djgpp_old_kbd.offset32 == kbd_ori.offset32
      && __djgpp_old_kbd.selector == kbd_ori.selector)
    __djgpp_exception_toggle ();
  __exit (status);
}
