/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* GO32V2 - A quick hack for loading non-stubbed images for V2.0
   Charles Sandmann 6/95 NO WARRANTY - build with -ldbg
   Eli Zaretskii    6/96 fix code which runs V1 EXE and COFF.
   Eli Zaretskii    7/96 fix code which runs V2 COFF.
   Eli Zaretskii    8/96 *really* fix code which runs V2 COFF;
			 support for V2.1 long command lines.
   Bugs:  doesn't scan manifest for versions installed yet */

/* If you want to change this, remember to test it with all the
   various ways `go32' can be called.  The following is the
   list of different cases I know about:

     go32 is called from DOS prompt without any arguments
     go32 is called from DOS prompt to run unstubbed COFF image of a v2 program
     go32 is called from DOS prompt to debug a v1 COFF image (go32 -d)
     v2's Make calls go32 to run unstubbed COFF image of a v2 program
     v1's Make calls go32 to run unstubbed COFF image of a v2 program
     16-bit Make calls go32 to run unstubbed COFF image of a v2 program
     v2's Make calls go32 to run unstubbed COFF image of a v1 program
     v1's Make calls go32 to run unstubbed COFF image of a v1 program
     16-bit Make calls go32 to run unstubbed COFF image of a v1 program
     v1 .EXE program is called from the DOS prompt
     v1 symlink is called from the DOS prompt
     v2's Make calls a v1 .EXE program
     v1's Make calls a v1 .EXE program
     16-bit Make calls a v1 .EXE program
     when v2's Make calls go32, you need to test command lines which
       are both shorter and longer than the DOS 126-character limit

   Note that there is nothing special about Make, it just serves as an
   example of one program that calls another.  It is convenient to use
   Make to test the above cases, because Make is available as both v2
   and v1 executable and as a 16-bit program, and because it can be
   easily used to call different programs with different command lines
   by tweaking a Makefile.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <process.h>
#include <go32.h>
#include <errno.h>
#include <dpmi.h>
#include <debug/v2load.h>
#include <sys/exceptn.h>
#include <sys/farptr.h>

/*======================================================================*/

#define IMG_ERROR	0
#define IMG_EXE		1
#define IMG_COFF	2
#define IMG_V2		4
#define IMG_UNKNOWN	8

#define IMG_PLAIN_EXE	(IMG_EXE | IMG_UNKNOWN)
#define IMG_EXE_V1	(IMG_EXE | IMG_COFF)
#define IMG_EXE_V2	(IMG_EXE | IMG_COFF | IMG_V2)
#define IMG_COFF_V1	(IMG_COFF)
#define IMG_COFF_V2	(IMG_COFF | IMG_V2)

/* Non-zero if debugging printout is required.  */
static int verbose = 0;

/* Non-zero if we were called by the !proxy method.  */
static int proxy_call = 0;

/* If we were called as `go32', holds the length of go32 full pathname.  */
static int go32_len = 0;

/* Read the given file and determine what kind of program it is.
   If IS_GO32 is non-zero, we *know* that IMAGE should be v1's go32.exe
   (therefore it cannot be a v1 symlink).  */

static int
check_image_version(char *image, int is_go32)
{
  int rv = 0;
  unsigned short header[3];
  unsigned char firstbytes[1];
  unsigned long coffhdr[42];
  int pf, i;
  int coff_offset, text_foffset;

  if (verbose)
    fprintf (stderr, "Hmmm... `%s\': ", image);

  errno = 0;
  pf = open(image, O_RDONLY|O_BINARY);
  if(pf < 0)
  {
    if (verbose)
      fprintf (stderr, "(error: %s)\n", strerror(errno));
    return IMG_ERROR;
  }

  /* See if it has an EXE header */
  read(pf, header, sizeof(header));
  if (header[0] == 0x5a4d)	/* MZ exe signature, stubbed? */
  {
    if (verbose)
      fprintf (stderr, "MZ, ");
    coff_offset = (long)header[2]*512L;
    if (header[1])
      coff_offset += (long)header[1] - 512L;
    lseek(pf, coff_offset, 0);
    read(pf, header, sizeof(header));
    rv |= IMG_EXE;
  }
  else
    coff_offset = 0;

  /* See if it has a COFF header (maybe after exe) */
  if (header[0] != 0x014c)	/* COFF? */
  {
    /* We need to pretend that v1 symlinks are v1 executables, to avoid
       endless looping, unless we were called as "go32 !proxy...", in
       which case we *know* it cannot be a symlink.  */
    int symlink = proxy_call && !is_go32;

    close(pf);
    if (verbose)
      fprintf (stderr, "without COFF signature%s\n",
	       symlink ? ", V1.x symlink" : "");
    if (symlink)
      return rv | IMG_COFF;
    else
      return rv | IMG_UNKNOWN;
  }
  if (verbose)
    fprintf (stderr, "COFF, ");
  rv |= IMG_COFF;

  /* Read the COFF header */
  errno = 0;
  lseek(pf, coff_offset, 0);
  i = read(pf, coffhdr, 0x0a8);    
  if (i != 0x0a8)
  {
    close(pf);
    if (verbose)
      fprintf (stderr, "(error reading COFF header: %s)\n", strerror(errno));
    return rv | IMG_UNKNOWN;
  }

  /* See what the first opcode is */
  text_foffset = coff_offset + coffhdr[12 + 5];	/* scnptr */
  errno = 0;
  lseek(pf, text_foffset, 0);
  read(pf, firstbytes, 1);
  if (errno)
  {
    if (verbose)
      fprintf (stderr, "(error reading 1st opcode: %s)\n", strerror(errno));
    return rv | IMG_UNKNOWN;
  }
  if (firstbytes[0] == 0xa3)	/* Opcode for movl %eax, 0x12345678 (V1) */
  {
    close(pf);
    if (verbose)
      fprintf (stderr, "V1.x\n");
    return rv;
  }
  if (verbose)
    fprintf (stderr, "V2\n");
  return rv | IMG_V2;
}

/*======================================================================*/

static int
far_strlen(int selector, int linear_addr)
{
  int save=linear_addr;
  _farsetsel(selector);
  while (_farnspeekb(linear_addr))
    linear_addr++;
  return linear_addr - save;
}

extern char **environ;

/* The original DOS command-line tail.  */
char DosCmdLine[128];

/* Points to the first non-blank character of the DOS command-line tail. */
char *argv1_start = &DosCmdLine[1];

char PROXY_STRING[] = "!proxy ";

/* We were called when the V1 go32 should have been called.  Call it
   with the same arguments we were passed */

int
run_v1_coff(int argc, char **argv)
{
  char *path = getenv("PATH");
  char *tokbuf = alloca(strlen(path)+1);
  char *dir;
  char *cmdbuf = alloca(strlen(argv1_start)+sizeof(PROXY_STRING));

  strcpy(tokbuf, path);
  if (strncmp(argv1_start, PROXY_STRING, sizeof(PROXY_STRING)-1) == 0)
    strcpy(cmdbuf, argv1_start);
  else
  {
    /* We should never get here, because v1 programs aren't run
       with !proxy in the environment.  But let's be defensive...  */
    strcpy(cmdbuf, PROXY_STRING);
    strcpy(cmdbuf + sizeof(PROXY_STRING) - 1, argv1_start);
  }

  /* we don't check "." because if v1's go32 was in "." we
     would never get a chance to run. */
  for (dir=strtok(tokbuf, ";"); dir; dir=strtok(0, ";"))
  {
    char *cp;
    char tmp[300];
    strcpy(tmp, dir);
    cp = tmp + strlen(tmp) - 1;
    if (*cp != ':' && *cp != '/' && *cp != '\\')
      *++cp = '/';
    strcpy(cp+1, "go32.exe");
    if (check_image_version(tmp, 1) == IMG_PLAIN_EXE)
    {
      /* We will try to pass the v1's go32 the original command line
	 as we got it (before our startup code built the argv[] array).  */
      if (!go32_len || !proxy_call)
      {
	if (verbose)
	  fprintf (stderr, "Exec: `%s %s\'\n", tmp, argv1_start);
	return _dos_exec(tmp, argv1_start, environ, NULL);
      }
      else
      {
	int proxy_argc, proxy_seg, proxy_ofs, i;
	unsigned short *rm_argv;
	char **arglist;

	/* Now, for the tricky part.

	   When a v2 program calls us, it sees that we are a v2 image,
	   and constructs the `!proxy' arguments accordingly.  This
	   means the arguments pointed to by `!proxy' include the
	   pathname of v2's go32.  We need to bump the SEG:OFF pointer
	   into the transfer buffer so that the v2's go32 pathname is
	   not seen by v1's go32.  How? see below.  */
	if (sscanf (cmdbuf + 7, "%x %x %x",
		    &proxy_argc, &proxy_seg, &proxy_ofs) != 3)
	{
	  fprintf (stderr, "go32/v2: malformed !proxy command line\n");
	  return -1;
	}

	/* Pull in the full command line from the transfer buffer
	   (actually, only needed for verbose operation).  */
	rm_argv=(unsigned short *)alloca(proxy_argc*sizeof(unsigned short));
	arglist = (char **)alloca (proxy_argc * sizeof (char *));
	movedata(_dos_ds, proxy_seg * 16 + proxy_ofs,
		 _my_ds(), (int)rm_argv, proxy_argc*sizeof(unsigned short));
    
	for (i = 0; i < proxy_argc; i++)
	{
	  int al = far_strlen(_dos_ds, proxy_seg*16 + rm_argv[i]);
	  char *arg = (char *)alloca(al+1);
	  movedata(_dos_ds, proxy_seg*16 + rm_argv[i],
		   _my_ds(), (int)(arg), al+1);
	  arglist[i] = arg;
	  if (verbose)
	    fprintf (stderr, "%s ", arg);
	}
	if (verbose)
	  fprintf (stderr, "\n");

	/* `rm_argv[i]' is the offset into the transfer buffer of the
	   i'th argument from the command line.  We need to bump
	   `proxy_ofs' so that it points to `rm_argv[1]' instead of
	   `rm_argv[0]'.  This way, the first argument that will be
	   seen by v1's go32 will be the image to run, not the
	   pathname of v2's go32.  */
	proxy_ofs += sizeof (rm_argv[0]);
	sprintf (cmdbuf + 7, "%04x %04x %04x",
		 proxy_argc - 1, proxy_seg, proxy_ofs);
	if (verbose)
	  fprintf (stderr, "Exec: `%s %s\'\n", tmp, cmdbuf);

	return _dos_exec(tmp, cmdbuf, environ, NULL);
      }
    }
  }
  fprintf(stderr, "go32/v2: cannot find v1's go32.exe\n");
  return -1;
}

/*======================================================================*/

/* It was an unstubbed V2 COFF file.  Use v2load to run it */

int
run_v2_coff(int argc, char **argv)
{
  jmp_buf start_state;
  int rm_la;
  short *rm_argv;
  char newcmdline[24];
  char fullpath[FILENAME_MAX];
  int i, sl, q, cmdlen;
  char *argv0 = argv[0];
  int tbuf, max_dos_mem;

  /* The actual command line might be longer than 126 characters, so
     we cannot call `v2loadimage' with the full argv[] array as the
     command line.  We also cannot call it with the original DOS
     command line, because after expansion by the startup code of
     the image we load it will have the image name as both argv[0] and
     argv[1] and all other arguments after it.  Response file cannot
     be used either, because the thread never returns to us after we
     longjmp, so we don't get to delete the response file.

     The only way I know out of this mess is to pass the argv[] array
     (sans argv[1]) via the !proxy method.  Sigh..  (The code was
     shamelessly stolen from src/libc/dis/process/dosexec.c).  */

  /* Make argv[0] explicit (why not?).  */
  _fixpath (argv[0], fullpath);
  argv[0] = fullpath;

  /* Can't use the usual transfer buffer, because `v2loadimage' will
     overwrite it.  Allocate our own buffer, for as much bytes as we need.  */
  for (cmdlen = sizeof (short), i = 0; i < argc; i++)
    cmdlen += strlen (argv[i]) + 1 + sizeof (short);

  tbuf = __dpmi_allocate_dos_memory ((cmdlen + 15) >> 4, &max_dos_mem);
  if (tbuf == -1)
  {
    fprintf (stderr, "Not enough DOS memory to pass args to %s\n", argv[0]);
    if (verbose)
      fprintf (stderr, "(Need %d bytes, only %d available)\n",
	       (cmdlen + 15) & 0xfffffff0U, max_dos_mem << 4);
    argv[0] = argv0;
    return -1;
  }

  rm_la   = (unsigned long)(unsigned short)tbuf << 4;
  q	  = rm_la + (argc + 1) * sizeof (short);
  rm_argv = (short *)alloca ((argc + 1) * sizeof (short));
  for (i = 0; i < argc; i++)
  {
    sl = strlen (argv[i]) + 1;
    dosmemput (argv[i], sl, q);
    rm_argv[i] = (q - rm_la) & 0xffff;
    q += sl;
  }
  rm_argv[i] = 0;
  dosmemput (rm_argv, (argc + 1) * sizeof (short), rm_la);
  newcmdline[0] = 22;
  sprintf (newcmdline + 1, " %s%04x %04x 0000", PROXY_STRING, argc, tbuf);
  if (verbose)
    fprintf (stderr, "V2Load %s%s\n", argv[0], newcmdline + 1);

  newcmdline[23] = '\n';
  if(v2loadimage(argv0, newcmdline, start_state))
  {
    fprintf(stderr, "Load failed for image %s\n",argv[0]);
    argv[0] = argv0;
    return -1;
  }
  argv[0] = argv0;

#if 1
  {
    extern __dpmi_paddr __djgpp_old_kbd;
    __dpmi_paddr except;

    /* restore old handlers */
    __dpmi_get_protected_mode_interrupt_vector(9, &except);
    if (__djgpp_old_kbd.offset32 != except.offset32
	|| __djgpp_old_kbd.selector != except.selector)
      __djgpp_exception_toggle ();
  }
#endif
  longjmp(start_state, 0);
#if 1
  /* reset new handlers */
  __djgpp_exception_toggle ();
#endif
  return 0;
}

/*======================================================================*/

/* Save on space, don't expand command-line wildcards.  */
char **__crt0_glob_function(char *argument) { return 0; }
void __crt0_load_environment_file(char *app) {}

int
main(int argc, char **argv)
{
  int i;
  char *tail;
  char *proxy_ev = getenv(" !proxy");

  __djgpp_set_ctrl_c(0);

  /* Debugging printout, anyone?  */
  if (getenv ("GO32_V2_DEBUG"))
    verbose = 1;

  /* Get the original DOS command-line tail.  */
  dosmemget(_go32_info_block.linear_address_of_original_psp+128, 128,
	    DosCmdLine);
  DosCmdLine[1+DosCmdLine[0]] = 0;

  /* Get past any whitespace in DOS command line.  */
  argv1_start = &DosCmdLine[1];
  while (*argv1_start && isspace((unsigned char)*argv1_start))
    argv1_start++;

  if (verbose)
  {
    fprintf (stderr, "Called as `%s\'\n", argv[0]);
    fprintf (stderr, "DOS CmdTail: `%s\'\n", argv1_start);
  }

  /* !proxy in the environment overrides the command line.  */
  if (proxy_ev)
  {
    argv1_start = proxy_ev;
    proxy_call  = 1;
    if (verbose)
    {
      fprintf (stderr, "Environ CmdTail: `%s\'\n", argv1_start);
    }
    while (*argv1_start && isspace((unsigned char)*argv1_start))
      argv1_start++;
  }
  else if (strncmp(argv1_start, PROXY_STRING, sizeof(PROXY_STRING)-1) == 0)
  {
    proxy_call = 1;
  }

  /* Are we called as GO32?
     If we are, then the *real* image is in `argv[1]'.  */
  for (tail = argv[0] + strlen (argv[0]); tail > argv[0]; tail--)
    if (*tail == '/' || *tail == '\\')
    {
      ++tail;
      break;
    }

  /* I don't want to rely too much on the way argv[0] looks like.
     `go32', `go32.exe', `Go32.EXE'--I think all of these should be OK.  */
  if (strlen(tail) >= 4 && strnicmp (tail, "go32", 4) == 0
      && (tail[4] == '.' || tail[4] == '-' || tail[4] == '\0'))
  {
    go32_len = strlen (argv[0]);
    ++argv;
    --argc;
  }

  /* `go32 -d' means the *real* image (the debugger) is after `-d'.  */
  if (go32_len && argc > 0 && strcmp (argv[0], "-d") == 0)
  {
    ++argv;
    --argc;
  }

  if (argc < 1)
  {
    printf("go32/v2 version %s built %s %s\n","2.0",__DATE__,__TIME__);
    printf("Usage: go32 coff-image [args]\n");
    printf("Rename this to go32.exe only if you need a go32 that can run v2 binaries as\n"
	   " well as v1 binaries (old makefiles).  Put ahead of the old go32 in your PATH\n"
	   " but do not delete your old go32 - leave it in the PATH after this one.\n");
    
    printf("Set GO32_V2_DEBUG=y in the environment to get verbose output.\n\n");
    /* Add the memory that we use for ourselves to the free amount.  */
    i = (_go32_dpmi_remaining_physical_memory() + (int)sbrk(0))/1024;
    printf("DPMI memory available: %d Kb\n",i);
    i = _go32_dpmi_remaining_virtual_memory()/1024-i;
    if(i < 0)
      i = 0;
    printf("DPMI swap space available: %d Kb\n", i);
    return 1;
  }

  switch (check_image_version(argv[0], 0))
  {
  case IMG_UNKNOWN:
    fprintf(stderr, "go32/v2: Unknown file type: %s\n", argv[0]);
    return -1;
  case IMG_ERROR:
    fprintf(stderr, "go32/v2: Error: %s: %s\n", argv[0], strerror(errno));
    return -1;
  case IMG_COFF_V2:
    return run_v2_coff(argc, argv);
  case IMG_COFF_V1:
  case IMG_EXE_V1:
    return run_v1_coff(argc, argv);
  case IMG_PLAIN_EXE:
  case IMG_EXE_V2:
    return spawnv(P_WAIT, argv[0], argv);
  }
  return 1;
}

