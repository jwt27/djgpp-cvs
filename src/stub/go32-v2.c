/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* GO32V2 - A quick hack for loading non-stubbed images for V2.0
   Charles Sandmann 6/95 NO WARRANTY - build with -ldbg
   Bugs: can't handle V1 images (do this someday)
         doesn't scan manifest for versions installed yet */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <process.h>
#include <go32.h>
#include <errno.h>
#include <dpmi.h>
#include <debug/v2load.h>

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

/* Read the given file and determine what kind of program it is */

static int
check_image_version(char *image)
{
  int rv = 0;
  unsigned short header[3];
  unsigned char firstbytes[1];
  unsigned long coffhdr[42];
  int pf, i;
  int coff_offset, text_foffset;

  pf = open(image, O_RDONLY|O_BINARY);
  if(pf < 0)
  {
    return IMG_ERROR;
  }

  /* See if it has an EXE header */
  read(pf, header, sizeof(header));
  if (header[0] == 0x5a4d)	/* MZ exe signature, stubbed? */
  {
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
    close(pf);
    return rv | IMG_UNKNOWN;
  }
  rv |= IMG_COFF;

  /* Read the COFF header */
  lseek(pf, coff_offset, 0);
  i = read(pf, coffhdr, 0x0a8);    
  if (i != 0x0a8)
  {
    close(pf);
    return rv | IMG_UNKNOWN;
  }

  /* See what the first opcode is */
  text_foffset = coff_offset + coffhdr[12 + 5];	/* scnptr */
  lseek(pf, text_foffset, 0);
  read(pf, firstbytes, 1);
  if (firstbytes[0] == 0xa3)	/* Opcode for movl %eax, 0x12345678 (V1) */
  {
    close(pf);
    return rv;
  }
  return rv | IMG_V2;
}

/*======================================================================*/

extern char **environ;

/* We were called when the V1 go32 should have been called.  Call it
   with the same arguments we were passed */

void
run_v1_coff(int argc, char **argv)
{
  char *path = getenv("PATH");
  char *tokbuf = alloca(strlen(path)+1);
  char *dir;

  strcpy(tokbuf, path);

  /* we don't check "." because if v1's go32 was in "." we
     would never get a chance to run. */
  for (dir=strtok(tokbuf, ";"); dir; dir=strtok(0, ";"))
  {
    char *cp;
    char tmp[500];
    strcpy(tmp, dir);
    cp = tmp + strlen(tmp) - 1;
    if (*cp != ':' && *cp != '/' && *cp != '\\')
      *++cp = '/';
    strcpy(cp+1, "go32.exe");
    if (check_image_version(tmp) == IMG_PLAIN_EXE)
    {
      char oldcmdline[128];
      dosmemget(_go32_info_block.linear_address_of_original_psp+128, 128, oldcmdline);
      oldcmdline[1+oldcmdline[0]] = 0;
      exit(_dos_exec(tmp, oldcmdline+2, environ));
    }
  }
  fprintf(stderr, "go32/v1: cannot find v1's go32.exe\n");
  exit(1);
}

/*======================================================================*/

/* It was an unstubbed V2 COFF file.  Use v2load to run it */

run_v2_coff(int argc, char **argv)
{
  int i;
  char cmdline[128];
  jmp_buf start_state;

  cmdline[1] = 0;
  for(i=2; argv[i]; i++)
  {
    strcat(cmdline+1, " ");
    strcat(cmdline+1, argv[i]);
  }
  i = strlen(cmdline+1);
  cmdline[0] = i;
  cmdline[i+1] = 13;
  if(v2loadimage(argv[1],cmdline,start_state))
  {
    fprintf(stderr, "Load failed for image %s\n",argv[1]);
    exit(1);
  }

  longjmp(start_state, 0);
  return 0;
}

/*======================================================================*/

/* Save on space, don't mess with command line */
char **__crt0_glob_function(char *argument) { return 0; }
void __crt0_load_environment_file(char *app) {}

int
main(int argc, char **argv)
{
  int i;

  __djgpp_set_ctrl_c(0);

  /* called from stub, our argv[0] is toast from the !proxy */
  if (check_image_version(argv[0]) != IMG_EXE_V2
      || (argc > 1 &&
	  (strcmp(argv[1], "!proxy") == 0 || strcmp(argv[1], "-d") == 0)))
  {
    /* meant to run other go32 */
    run_v1_coff(argc, argv);
  }

  if (argc < 2)
  {
    printf("go32/v2 version %s built %s %s\n","2.0",__DATE__,__TIME__);
    printf("Usage: go32 coff-image [args]\n");
    printf("Rename this to go32.exe only if you need a go32 that can run v2 binaries as\n"
	   " well as v1 binaries (old makefiles).  Put ahead of the old go32 in your PATH.\n");
    i = _go32_dpmi_remaining_physical_memory()/1024;
    printf("DPMI memory available: %d Kb\n",i);
    i = _go32_dpmi_remaining_virtual_memory()/1024-i;
    if(i < 0)
      i = 0;
    printf("DPMI swap space available: %d Kb\n", i);
    exit(1);
  }

  switch (check_image_version(argv[1]))
  {
  case IMG_UNKNOWN:
    fprintf(stderr, "go32/v2: Unknown file type: %s\n", argv[1]);
    exit(1);
  case IMG_ERROR:
    fprintf(stderr, "go32/v2: Error: %s: \n", argv[1], strerror(errno));
    exit(1);
  case IMG_COFF_V2:
    run_v2_coff(argc, argv);
  case IMG_COFF_V1:
  case IMG_EXE_V1:
    run_v1_coff(argc, argv);
  case IMG_PLAIN_EXE:
  case IMG_EXE_V2:
    return spawnv(P_WAIT, argv[1], argv+1);
  }
  exit(1);
}
