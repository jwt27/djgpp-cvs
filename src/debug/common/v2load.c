/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* Routine to load V2.0 format image.  Control is returned to the caller so
   additional setup can be done before execution.
   C. Sandmann Jan-94
 */
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
#define SIMAGIC "go32stub"

AREAS areas[MAX_AREA];

extern char **environ;

static int read_section(int pf, unsigned ds, unsigned foffset, unsigned soffset, unsigned size)
{
  char buffer[32768];
  int read_size;
  int nr;
  unsigned my_ss;

  size += (foffset & 0x1ff);	/* sector alignment */
  foffset &= 0xfffffe00U;
  soffset &= 0xfffffe00U;
  lseek(pf, foffset, 0);
  asm("mov %%ss,%w0" : "=g" (my_ss) );
  while(size) {
    read_size = (size > sizeof(buffer)) ? sizeof(buffer) : size;
    nr = read(pf, buffer, read_size);
    if(nr != read_size)
      return -1;
    movedata(my_ss, (unsigned)&buffer, ds, soffset, read_size);
    size -= read_size;
    soffset += read_size;
  }
  return 0;
}

int v2loadimage(const char *program, const char *cmdline, jmp_buf load_state)
{
  unsigned short header[3];
  int pf,i, envp;
  unsigned coff_offset;
  _GO32_StubInfo si;
  unsigned long coffhdr[42];
  unsigned start_eip, text_foffset, text_soffset, text_size, data_foffset;
  unsigned data_soffset, data_size, bss_soffset, bss_size;
  unsigned client_cs, client_ds, my_ds;
  __dpmi_meminfo memblock;
  unsigned new_env_selector;
  char true_name[FILENAME_MAX];

  _truename(program, true_name);

  pf = open(program, O_RDONLY|O_BINARY);
  if(pf < 0)
    return -1;

  read(pf, header, sizeof(header));
  if (header[0] == 0x5a4d) {		/* MZ exe signature, stubbed? */
    coff_offset = (long)header[2]*512L;
    if (header[1])
      coff_offset += (long)header[1] - 512L;
    lseek(pf, 512, 0);			/* Position of V2 stubinfo */
    read(pf, si.magic, 8);
    if (memcmp(SIMAGIC, si.magic, 8) != 0) {
      close(pf);
      return -1;				/* Not V2 image, show failure */
    }
    read(pf, &si.magic[8], sizeof(si)-8);	/* Rest of stubinfo */
    /* Check for real image, (for now assume imbedded) */
    lseek(pf, coff_offset, 0);
    read(pf, header, sizeof(header));
  } else {
    coff_offset = 0;
    strcpy(si.magic, "go32stub, V 2.00");
    si.size = 0x44;
    si.minstack = 0x40000;
    si.minkeep = 16384;			/* transfer buffer size */
    memset(&si.basename, 0, 24);	/* Asciiz strings */
  }
  if (header[0] != 0x014c) {		/* COFF? */
    close(pf);
    return -1;				/* Not V2 image, show failure */
  }
  lseek(pf, coff_offset, 0);
  i = read(pf, coffhdr, 0x0a8);    
  if (i != 0x0a8) {
    close(pf);
    return -1;				/* Not V2 image, show failure */
  }

  start_eip = coffhdr[5 + 4];
  text_foffset = coff_offset + coffhdr[12 + 5];	/* scnptr */
  text_soffset = coffhdr[12 + 3];		/* vaddr */
  text_size = coffhdr[12 + 4];			/* size */
  areas[A_text].first_addr = text_soffset;
  areas[A_text].last_addr = text_soffset + text_size - 1;
  data_foffset = coff_offset + coffhdr[22 + 5];
  data_soffset = coffhdr[22 + 3];
  data_size = coffhdr[22 + 4];
  areas[A_data].first_addr = data_soffset;
  areas[A_data].last_addr = data_soffset + data_size - 1;
  bss_soffset = coffhdr[32 + 3];
  bss_size = coffhdr[32 + 4];
  areas[A_bss].first_addr = bss_soffset;
  areas[A_bss].last_addr = bss_soffset + bss_size - 1;
  areas[A_stack].first_addr = bss_soffset + bss_size;
  areas[A_stack].last_addr = areas[A_stack].first_addr + si.minstack - 1;
  areas[A_arena].first_addr = areas[A_stack].last_addr + 1;
  areas[A_arena].last_addr = (areas[A_arena].first_addr & ~0xffff) + 0xffff;

  si.initial_size = bss_soffset + bss_size;
  if (si.initial_size <= 0x10001)
    si.initial_size = 0x10001;
  si.initial_size += 0xffff;
  si.initial_size &= 0xffff0000U;

  si.env_size = 0;
  for (i=0; environ[i]; i++)
    si.env_size += strlen(environ[i])+1;
  si.env_size += 4 + strlen(true_name);

  /* Allocate the dos memory for the environment and command line. */
  i = __dpmi_allocate_dos_memory((si.env_size + 256) / 16, (int *)&new_env_selector);
  if(i == -1)
    return -1;

  envp = 0;
  for (i=0; environ[i]; i++)
  {
    int len = strlen(environ[i])+1;
    movedata(_my_ds(), (unsigned)environ[i], new_env_selector, envp, len);
    envp += len;
  }
  _farpokeb(new_env_selector, envp++, 0);
  _farpokeb(new_env_selector, envp++, 1);
  _farpokeb(new_env_selector, envp++, 0);
  movedata(_my_ds(), (unsigned)true_name, new_env_selector, envp, strlen(true_name)+1);

  /* Allocate the dos memory for the transfer buffer.  This nukes si.cs_selector,
   but that's OK since we set it next. */
  i = __dpmi_allocate_dos_memory((si.minkeep + 256) / 16, (int *)&si.psp_selector);
  if(i == -1)
    return -1;
  si.ds_segment = i + (256/16);

  if((si.cs_selector = __dpmi_allocate_ldt_descriptors(1)) == 0xffff)
    return -1;

  if(__dpmi_set_segment_base_address(si.cs_selector, si.ds_segment * 16) == -1)
    return -1;

  /* Set the type as 16 bit, small */
  if(__dpmi_set_descriptor_access_rights(si.cs_selector, ((si.cs_selector & 3) << 5) | 0x009b) == -1)
    return -1;

  if(__dpmi_set_segment_limit(si.cs_selector, si.minkeep-1) == -1)
    return -1;

  if((si.ds_selector = __dpmi_create_alias_descriptor(si.cs_selector)) == 0xffff)
    return -1;

  if(__dpmi_set_descriptor_access_rights(si.ds_selector, ((si.cs_selector & 3) << 5) | 0x4093) == -1)
    return -1;

#ifdef DEBUG
  show_selector("ds16", si.ds_selector);
  show_selector("cs16", si.cs_selector);
  show_selector("psp16", si.psp_selector);
  printf("mine:\n");
  show_selector("ds16", _stubinfo->ds_selector);
  show_selector("cs16", _stubinfo->cs_selector);
  show_selector("psp16", _stubinfo->psp_selector);
  printf("Initial size: 0x%lx\n",si.initial_size);
#endif

/* Finished with 16 bit config, now for 32 bit setup */

  if((client_cs = __dpmi_allocate_ldt_descriptors(1)) == 0xffff)
    return -1;

  memblock.size = si.initial_size;
  if(__dpmi_allocate_memory(&memblock) == -1)
    return -1;
  si.memory_handle = memblock.handle;
  
  if(__dpmi_set_segment_base_address(client_cs, memblock.address) == -1)
    return -1;

  /* Set the type as 32 bit, big */
  if(__dpmi_set_descriptor_access_rights(client_cs, ((client_cs & 3) << 5) | 0xc09b) == -1)
    return -1;

  if(__dpmi_set_segment_limit(client_cs, si.initial_size-1) == -1)
    return -1;

  if((client_ds = __dpmi_create_alias_descriptor(client_cs)) == 0xffff)
    return -1;

/* This shouldn't be necessary, but QDPMI does not clone the 32 bit properly
   so we have to reset this stuff or it doesn't work when it is used as the
   client's SS  :-( */
  if(__dpmi_set_descriptor_access_rights(client_ds, ((client_cs & 3) << 5) | 0xc093) == -1)
    return -1;

  if(__dpmi_set_segment_limit(client_ds, si.initial_size-1) == -1)
    return -1;
/* End should be unnecessary code */

/* Load image */
  my_ds = 0;
  asm("mov %%ds,%w0" : "=g" (my_ds) );
#ifdef DEBUG
  show_selector("my_ds", my_ds);
  show_selector("client_ds",client_ds);
  asm("mov %%cs,%w0" : "=g" (i) );
  show_selector("my_cs",i);
  show_selector("client_cs",client_cs);
  printf("Load image...\n");
#endif

  if(read_section(pf, client_ds, text_foffset, text_soffset, text_size) )
    return -1;
  if(read_section(pf, client_ds, data_foffset, data_soffset, data_size) )
    return -1;
  for (i=0; i+3<bss_size; i+= 4)
    _farpokel(client_ds, bss_soffset+i, 0);
  for (; i<bss_size; i++)
    _farpokeb(client_ds, bss_soffset+i, 0);

  close(pf);
  /* copy my psp into debug process */
  movedata(_stubinfo->psp_selector, 0, si.psp_selector, 0, 128);

  _farpokew(si.psp_selector, 0x2c, new_env_selector);

  /* copy command arguments into debug process */
  movedata(my_ds, (unsigned)cmdline, si.psp_selector, 128, 128);
  /* copy si we built into debug process */
  movedata(my_ds, (unsigned)&si, si.ds_selector, 0, sizeof(si));
  load_state->__ds = client_ds;
  load_state->__cs = client_cs;
  load_state->__eip = start_eip;
  load_state->__esp = si.minkeep;
  load_state->__ss = si.ds_selector;
  load_state->__fs = si.ds_selector;
  load_state->__es = load_state->__gs = client_ds;
  load_state->__eflags = 0x3202;
  return 0;
}

#ifdef TEST
void main(int argc, char **argv)
{
  int i;
  char cmdline[128];
  jmp_buf load_state;

  if(argc < 2) {
    printf("Usage: v2load imagename [args]\n");
    exit(1);
  }

  putenv("FOO=from-v2load");
  
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
#endif
