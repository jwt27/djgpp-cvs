/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <process.h>
#include <go32.h>
#include <dpmi.h>
#include <ctype.h>
#include <libc/dosexec.h>
#include <libc/unconst.h>
#include <libc/dosio.h>

extern char **environ;

int __dosexec_in_system = 0;

typedef struct {
  unsigned short eseg;
  unsigned short argoff;
  unsigned short argseg;
  unsigned short fcb1_off;
  unsigned short fcb1_seg;
  unsigned short fcb2_off;
  unsigned short fcb2_seg;
} Execp;

static Execp parm;

static unsigned long tbuf;

static unsigned long talloc(size_t amt)
{
  unsigned long rv = tbuf;
  tbuf += amt;
  return rv;
}

static int direct_exec_tail(const char *program, const char *args, char * const envp[])
{
  __dpmi_regs r;
  unsigned long program_la;
  unsigned long arg_la;
  unsigned long parm_la;
  unsigned long env_la, env_e_la;
  char arg_header[3];
  char short_name[FILENAME_MAX];
  const char *progname;
  unsigned proglen;
  int i;
  
  sync();

  proglen = strlen(program)+1;
  if(_USE_LFN) {
    dosmemput(program, proglen, tbuf);
    r.x.ax = 0x7160;			/* Truename */
    r.x.cx = 1;				/* Get short name */
    r.x.ds = r.x.es = tbuf / 16;
    r.x.si = r.x.di = tbuf & 15;
    __dpmi_int(0x21, &r);
    if (r.x.flags & 1)
    {
      errno = __doserr_to_errno(r.x.ax);
      return -1;
    }
    dosmemget(tbuf, FILENAME_MAX, short_name);
    progname = short_name;
    proglen = strlen(short_name)+1;
  } else
    progname = program;
  
  program_la = talloc(proglen);
  arg_la = talloc(strlen(args)+3);
  parm_la = talloc(sizeof(Execp));

  dosmemput(progname, proglen, program_la);
  
  arg_header[0] = strlen(args);
  arg_header[1] = '\r';
  dosmemput(arg_header, 1, arg_la);
  dosmemput(args, strlen(args), arg_la+1);
  dosmemput(arg_header+1, 1, arg_la+1+strlen(args));

  do {
    env_la = talloc(1);
  } while (env_la & 15);
  talloc(-1);
  for (i=0; envp[i]; i++)
  {
    env_e_la = talloc(strlen(envp[i])+1);
    dosmemput(envp[i], strlen(envp[i])+1, env_e_la);
  }
  arg_header[0] = 0;
  arg_header[1] = 1;
  arg_header[2] = 0;
  dosmemput(arg_header, 3, talloc(3));
  env_e_la = talloc(proglen);
  dosmemput(progname, proglen, env_e_la);

  parm.eseg = env_la / 16;
  parm.argseg = arg_la / 16;
  parm.argoff = arg_la & 15;
  dosmemput(&parm, sizeof(parm), parm_la);

  r.x.ax = 0x4b00;
  r.x.ds = program_la / 16;
  r.x.dx = program_la & 15;
  r.x.es = parm_la / 16;
  r.x.bx = parm_la & 15;
  __dpmi_int(0x21, &r);
  if (r.x.flags & 1)
  {
    errno = __doserr_to_errno(r.x.ax);
    return -1;
  }
  
  r.h.ah = 0x4d;
  __dpmi_int(0x21, &r);
  
  if (r.x.flags & 1)
  {
    errno = __doserr_to_errno(r.x.ax);
    return -1;
  }
  return r.x.ax;
}

int
_dos_exec(const char *program, const char *args, char * const envp[])
{
  tbuf = __tb;
  return direct_exec_tail(program, args, envp);
}

static char GO32_STRING[] = "go32.exe";

static int direct_exec(const char *program, char **argv, char **envp)
{
  int i, arglen;
  char *args, *argp;

  tbuf = __tb;

  arglen = 0;
  for (i=1; argv[i]; i++)
    arglen += strlen(argv[i]) + 1;
  args = (char *)alloca(arglen+1);
  argp = args;
  for (i=1; argv[i]; i++)
  {
    const char *p = argv[i];
    if (argp - args > 125)
      break;
    *argp++ = ' ';
    while (*p)
    {
      if (argp - args > 125)
        break;
      *argp++ = *p++;
    }
  }
  *argp = 0;
  
  return direct_exec_tail(program, args, envp);
}

typedef struct {
  char magic[16];
  int struct_length;
  char go32[16];
} StubInfo;
#define STUB_INFO_MAGIC "StubInfoMagic!!"

static int go32_exec(const char *program, char **argv, char **envp)
{
  char *save_argv0;
  int is_stubbed = 0;
  int found_si = 0;
  StubInfo si;
  unsigned short header[3];
  int pf, i;
  char *go32, *sip=0;
  char rpath[80];
  int stub_offset, argc;

  int v2_0 = 0;
  int si_la=0, rm_la, rm_seg;
  short *rm_argv;
  char cmdline[34];

  if (__dosexec_in_system)
    return direct_exec(program, argv, envp);

  pf = open(program, O_RDONLY|O_BINARY);

  read(pf, header, sizeof(header));
  if (header[0] == 0x010b || header[0] == 0x014c)
  {
    is_stubbed = 1;
  }
  else if (header[0] == 0x5a4d)
  {
    int header_offset = (long)header[2]*512L;
    if (header[1])
      header_offset += (long)header[1] - 512L;
    lseek(pf, 512, 0);
    read(pf, cmdline, 8);
    cmdline[8] = 0;
    if (strcmp(cmdline, "go32stub") == 0)
      v2_0 = 1;
    else
    {
      lseek(pf, header_offset - 4, 0);
      read(pf, &stub_offset, 4);
      header[0] = 0;
      read(pf, header, sizeof(header));
      if (header[0] == 0x010b)
	is_stubbed = 1;
      if (header[0] == 0x014c)
	is_stubbed = 1;
      lseek(pf, stub_offset, 0);
      read(pf, &si, sizeof(si));
      if (memcmp(STUB_INFO_MAGIC, si.magic, 16) == 0)
	found_si = 1;
    }
  }
  if (!is_stubbed && !v2_0)
  {
    close(pf);
    return direct_exec(program, argv, envp);
  }

  if (found_si)
    go32 = si.go32;
  else
    go32 = GO32_STRING;

  if (v2_0)
  {
    strcpy(rpath, program);
  }
  else
  {
    if (!__dosexec_find_on_path(go32, envp, rpath))
    {
      close(pf);
      return direct_exec(program, argv, envp); /* give up and just run it */
    }

    if (found_si)
    {  
      lseek(pf, stub_offset, 0);
      sip = (char *)alloca(si.struct_length);
      read(pf, sip, si.struct_length);
    }
  }
  close(pf);

  save_argv0 = argv[0];
  argv[0] = unconst(program, char *); /* since that's where we really found it */

  tbuf = __tb;

  if (found_si)
  {
    si_la = talloc(si.struct_length);
    dosmemput(sip, si.struct_length, si_la);
  }
  
  for (argc=0; argv[argc]; argc++);
  rm_la = talloc(2*(argc+1));
  rm_seg = (__tb >> 4) & 0xffff;
  rm_argv = (short *)alloca((argc+1) * sizeof(short));
  for (i=0; i<argc; i++)
  {
    int sl = strlen(argv[i]) + 1;
    int q = talloc(sl);
    dosmemput(argv[i], sl, q);
    rm_argv[i] = (q - (rm_seg<<4)) & 0xffff;
  }
  rm_argv[i] = 0;
  dosmemput(rm_argv, 2*(argc+1), rm_la);
  
  sprintf(cmdline, " !proxy %04x %04x %04x %04x %04x",
    argc, rm_seg, (rm_la - (rm_seg<<4))&0xffff,
    rm_seg, (si_la - (rm_seg<<4))&0xffff);
  if (!found_si)
    cmdline[22] = 0; /* remove stub information */

  argv[0] = save_argv0;

  return direct_exec_tail(rpath, cmdline, envp);
}

int
__dosexec_command_exec(const char *program, char **argv, char **envp)
{
  const char *comspec=0;
  char *cmdline;
  char *newargs[3];
  int cmdlen;
  int i;
  
  cmdlen = strlen(program) + 4;
  for (i=0; argv[i]; i++)
    cmdlen += strlen(argv[i]) + 1;
  cmdline = (char *)alloca(cmdlen);
  
  strcpy(cmdline, "/c ");
  for (i=0; program[i] > ' '; i++)
  {
    if (program[i] == '/')
      cmdline[i+3] = '\\';
    else
      cmdline[i+3] = program[i];
  }
  for (; program[i]; i++)
    cmdline[i+3] = program[i];
  cmdline[i+3] = 0;
  for (i=1; argv[i]; i++)
  {
    strcat(cmdline, " ");
    strcat(cmdline, argv[i]);
  }
  for (i=0; envp[i]; i++)
    if (strncmp(envp[i], "COMSPEC=", 8) == 0)
      comspec = envp[i]+8;
  if (!comspec)
    for (i=0; environ[i]; i++)
      if (strncmp(environ[i], "COMSPEC=", 8) == 0)
        comspec = environ[i]+8;
  if (!comspec)
    comspec = "c:\\command.com";
  newargs[0] = unconst(comspec, char *);
  newargs[1] = cmdline;
  newargs[2] = 0;
  i = direct_exec(comspec, newargs, envp);
  return i;
}

static int script_exec(const char *program, char **argv, char **envp)
{
  char line[130], interp[80], iargs[130];
  FILE *f;
  char **newargs;
  int i, hasargs=0;

  f = fopen(program, "rt");
  if (!f)
  {
    errno = ENOENT;
    return -1;
  }
  fgets(line, 130, f);
  fclose(f);
  iargs[0] = 0;
  interp[0] = 0;
  sscanf(line, "#! %s %[^\n]", interp, iargs);
  if (interp[0] == 0)
    return __dosexec_command_exec(program, argv, envp); /* it couldn't be .exe or .com if here */
  if (iargs[0])
    hasargs=1;

  for (i=0; argv[i]; i++);
  newargs = (char **)alloca((i+2+hasargs)*sizeof(char *));
  for (i=0; argv[i]; i++)
    newargs[i+1+hasargs] = unconst(argv[i], char *);
  newargs[i+1+hasargs] = 0;
  newargs[0] = unconst(argv[0], char *); /* it might work right, if not in system() */
  if (hasargs)
    newargs[1] = iargs;

  i = spawnvpe(P_WAIT, interp, newargs, envp);
  return i;
}

static struct {
  const char *extension;
  int (*interp)(const char *, char **, char **);
} interpreters[] = {
  { ".com", direct_exec },
  { ".exe", go32_exec },
  { ".bat", __dosexec_command_exec },
  { "",     go32_exec },
  { 0,      script_exec },
  { 0,      0 },
};
#define INTERP_NO_EXT 3

/*-------------------------------------------------*/

char *
__dosexec_find_on_path(const char *program, char *envp[], char *buf)
{
  char *pp, *rp, *pe;
  const char *ptr;
  int i, hasdot=0, haspath=0;
  int tried_dot = 0;

  strcpy(buf, program);
  rp = buf + strlen(buf);

  for (ptr=program; *ptr; ptr++)
  {
    if (*ptr == '.')
      hasdot = 1;
    if (*ptr == '/' || *ptr == '\\' || *ptr == ':')
    {
      haspath = 1;
      hasdot = 0;
    }
  }

  if (hasdot)
  {
    if (access(buf, 0) == 0 && access(buf, D_OK))
      return buf;
  }
  else
    for (i=0; interpreters[i].extension; i++)
    {
      strcpy(rp, interpreters[i].extension);
      if (access(buf, 0) == 0 && access(buf, D_OK))
	return buf;
    }

  if (haspath)
    return 0;
  *rp = 0;

  pp = 0;
  for (i=0; envp[i]; i++)
    if (strncmp(envp[i], "PATH=", 5) == 0)
      pp = envp[i] + 5;
  if (pp == 0)
    return 0;

  while (1)
  {
    if (!tried_dot)
    {
      rp = buf;
      pe = pp;
      tried_dot = 1;
    }
    else
    {
      rp = buf;
      for (pe = pp; *pe && *pe != ';'; pe++)
        *rp++ = *pe;
      pp = pe+1;
      if (rp > buf && rp[-1] != '/' && rp[-1] != '\\' && rp[-1] != ':')
        *rp++ = '/';
    }
    for (ptr = program; *ptr; ptr++)
      *rp++ = *ptr;
    *rp = 0;
    
    if (hasdot)
    {
      if (access(buf, 0) == 0 && access(buf, D_OK))
	return buf;
    }
    else
    {
      for (i=0; interpreters[i].extension; i++)
      {
        strcpy(rp, interpreters[i].extension);
        if (access(buf, 0) == 0 && access(buf, D_OK))
          return buf;
      }
    }
    if (*pe == 0)
      return 0;
  }
}

int __spawnve(int mode, const char *path, char *const argv[], char *const envp[])
{
  /* This is the one that does the work! */
  union { char *const *x; char **p; } u;
  int i = -1;
  char **argvp;
  char **envpp;
  char rpath[80], *rp, *rd=0;

  u.x = argv; argvp = u.p;
  u.x = envp; envpp = u.p;

  fflush(stdout); /* just in case */
  for (rp=rpath; *path; *rp++ = *path++)
  {
    if (*path == '.')
      rd = rp;
    if (*path == '\\' || *path == '/')
      rd = 0;
  }
  *rp = 0;
  if (rd)
  {
    for (i=0; interpreters[i].extension; i++)
      if (stricmp(rd, interpreters[i].extension) == 0)
        break;
  }
  while (access(rpath, 0) && access(rpath, D_OK))
  {
    i++;
    if (interpreters[i].extension == 0 || rd)
    {
      errno = ENOENT;
      return -1;
    }
    strcpy(rp, interpreters[i].extension);
  }
  if (i == -1)
    i = INTERP_NO_EXT;
  i = interpreters[i].interp(rpath, argvp, envpp);
  if (mode == P_OVERLAY)
    exit(i);
  return i;
}
