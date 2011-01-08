#include <libc/stubs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dpmi.h>
#include <go32.h>
#include <signal.h>
#include <setjmp.h>
#include <stubinfo.h>
#include <debug/v2load.h>
#include <libc/farptrgs.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
  int i;
  char cmdline[128];
  char env_str[32];
  jmp_buf load_state;

  if(argc < 2) {
    printf("Usage: v2load imagename [args]\n");
    exit(1);
  }

  strcpy(env_str, "FOO=from-v2load");
  putenv(env_str);
  
  cmdline[1] = 0;
  for(i=2; argv[i]; i++) {
  	strcat(cmdline+1, " ");
  	strcat(cmdline+1, argv[i]);
  }
  i = strlen(cmdline+1);
  cmdline[0] = i;
  cmdline[i+1] = 13;
  if(v2loadimage(argv[1],cmdline,load_state)) {
    printf("Load failed for %s\n",argv[1]);
    exit(2);
  }
  printf("Program text ................: %08x - %08x\n",
	     areas[A_text].first_addr, areas[A_text].last_addr);
  printf("Program data ................: %08x - %08x\n",
	     areas[A_data].first_addr, areas[A_data].last_addr);
  printf("Program bss .................: %08x - %08x\n",
	     areas[A_bss].first_addr, areas[A_bss].last_addr);
  printf("Program stack ...............: %08x - %08x\n",
	     areas[A_stack].first_addr, areas[A_stack].last_addr);
  printf("Program arena ...............: %08x - %08x\n",
	     areas[A_arena].first_addr, areas[A_arena].last_addr);
  printf("Jumping to image...\n");
  longjmp(load_state, 0);
}
