/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "ed.h"
#include "debug.h"
#include <debug/syms.h>
#include <libc/file.h>	/* Workaround for stderr bug below */

static char ansi_mode;	/* if set, OK to emit ansi control codes */

void ansi(int fg)
{
  if(ansi_mode)
    printf("\033[%d;%dm", (fg & A_bold) ? 1 : 0, 30+(fg&7));
}

/* for debugging a qdpmi bug */
#if 0
#define x printf("%s:%d\n", __FILE__, __LINE__);
#else
#define x
#endif

void ansidetect(void)	/* Idea from DJ's install program */
{
  word16 oldp, newp;
  asm volatile ("movb $3,%%ah;movb $0,%%bh;int $0x10;movw %%dx,%0"
		: "=g" (oldp) :
		: "ax", "bx", "cx", "dx");
  printf("\033[0m");
  fflush(stdout);
  asm volatile ("movb $3,%%ah;movb $0,%%bh;int $0x10;movw %%dx,%0"
		: "=g" (newp) :
		: "ax", "bx", "cx", "dx");
  if (newp == oldp)
    ansi_mode = 1;
  else {
    printf("\r    \r");
    fflush(stdout);
    ansi_mode = 0;
  }
}

int main(int argc, char **argv)
{
  int i;
  char cmdline[128];
  jmp_buf start_state;

  if(argc < 2) {
    printf("Usage: %s debug-image [args]\n", argv[0]);
    exit(1);
  }
x
  syms_init(argv[1]);
x
  cmdline[1] = 0;
  for(i=2; argv[i]; i++) {
  	strcat(cmdline+1, " ");
  	strcat(cmdline+1, argv[i]);
  }
  i = strlen(cmdline+1);
  cmdline[0] = i;
  cmdline[i+1] = 13;
x  if(v2loadimage(argv[1],cmdline,start_state)) {
x    printf("Load failed for image %s\n",argv[1]);
    exit(1);
  }
x
  edi_init(start_state);
x  ansidetect();
x  stdout->_file = dup(fileno(stdout));
x
x  debugger();

  return 0;
}
