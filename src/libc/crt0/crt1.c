/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <crt0.h>
#include <string.h>
#include <libc/internal.h>
#include <stdlib.h>
#include <go32.h>
#include <stubinfo.h>
#include <dpmi.h>
#include <libc/farptrgs.h>
#include <pc.h>
#include <libc/bss.h>
#include <fcntl.h>
#include <libc/environ.h>

/* Global variables */

#define ds _my_ds()

/* This gets incremented each time the program is started.
   Programs (such as Emacs) which dump their code to create
   a new executable, cause this to be larger than 2.  Library
   functions that cache info in static variables should check
   the value of `__bss_count' if they need to reinitialize
   the static storage.  */
int __bss_count = 1;

char **environ;
int _crt0_startup_flags;	/* default to zero unless app overrides them */

int __crt0_argc;
char **__crt0_argv;

/* Local variables */

static void
setup_core_selector(void)
{
  int c = __dpmi_allocate_ldt_descriptors(1);
  if (c == -1)
  {
    _dos_ds = 0;
    return;
  }
  _dos_ds = c;

  __dpmi_set_segment_limit(_dos_ds, -1);
}

static void
setup_screens(void)
{
  if(_farpeekw(_dos_ds, 0xffff3) == 0xfd80)	/* NEC PC98 ? */
  {
    ScreenPrimary = ScreenSecondary = 0xa0000;
  }
  else if (_farpeekb(_dos_ds, 0x449) == 7)
  {
    ScreenPrimary = 0xb0000;
    ScreenSecondary = 0xb8000;
  }
  else
  {
    ScreenPrimary = 0xb8000;
    ScreenSecondary = 0xb0000;
  }
}

static void
setup_go32_info_block(void)
{
  __dpmi_version_ret ver;

  __dpmi_get_version(&ver);

  _go32_info_block.size_of_this_structure_in_bytes = sizeof(_go32_info_block);
  __tb = _stubinfo->ds_segment * 16;
  _go32_info_block.size_of_transfer_buffer = _stubinfo->minkeep;
  _go32_info_block.pid = _stubinfo->psp_selector;
  _go32_info_block.master_interrupt_controller_base = ver.master_pic;
  _go32_info_block.slave_interrupt_controller_base = ver.slave_pic;
  _go32_info_block.linear_address_of_stub_info_structure = 0xffffffffU; /* force error */
  __dpmi_get_segment_base_address(_stubinfo->psp_selector,
    &_go32_info_block.linear_address_of_original_psp);
  _go32_info_block.run_mode = _GO32_RUN_MODE_DPMI;
  _go32_info_block.run_mode_info = (ver.major << 8) | (ver.minor);
}

char *__dos_argv0;

static void
setup_environment(void)
{
  char *dos_environ = alloca(_stubinfo->env_size), *cp;
  short env_selector;
  int env_count=0;
  movedata(_stubinfo->psp_selector, 0x2c, ds, (int)&env_selector, 2);
  movedata(env_selector, 0, ds, (int)dos_environ, _stubinfo->env_size);
  cp = dos_environ;
  do {
    env_count++;
    while (*cp) cp++; /* skip to NUL */
    cp++; /* skip to next character */
  } while (*cp); /* repeat until two NULs */
  environ = (char **)malloc((env_count+1) * sizeof(char *));
  if (environ == 0)
    return;

  cp = dos_environ;
  env_count = 0;
  do {
    /* putenv assumes each string is malloc'd */
    environ[env_count] = (char *)malloc(strlen(cp)+1);
    strcpy(environ[env_count], cp);
    env_count++;
    while (*cp) cp++; /* skip to NUL */
    cp++; /* skip to next character */
  } while (*cp); /* repeat until two NULs */
  environ[env_count] = 0;
  
  __dos_argv0 = (char *)malloc(strlen(cp + 3)+1);
  if (__dos_argv0 == 0)
    abort();
  strcpy(__dos_argv0, cp+3);
}

extern void __main(void);
extern int  main(int, char **, char **);
extern void _crt0_init_mcount(void);	/* For profiling */

char __PROXY[] = " !proxy";
size_t __PROXY_LEN = sizeof(__PROXY)-1;

void
__crt1_startup(void)
{
  char *pn;
  __bss_count ++;
  __crt0_argv = 0;
  setup_core_selector();
  setup_screens();
  setup_go32_info_block();
  __djgpp_exception_setup();
  setup_environment();
  __environ_changed++;
  /* Make so rest of startup could use LFN.  */
  (void)_USE_LFN;
  __crt0_setup_arguments();
  pn = __crt0_argv ? __crt0_argv[0] : __dos_argv0;
  __crt0_load_environment_file(pn);
  (void)_USE_LFN;	/* DJGPP.ENV might have set $LFN */
  _npxsetup(pn);
  _crt0_init_mcount();
  __main();
  exit(main(__crt0_argc, __crt0_argv, environ));
}

