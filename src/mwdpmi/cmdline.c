/* ---------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                 */
/* Distributed under the GPL, see COPYING for details.                    */
/* ---------------------------------------------------------------------- */
#include "code32.h"

/* ---------------------------------------------------------------------- */
static void
help ()
{
  printf ("Usage: %s options...\r\n\r\n"
#ifdef DEBUG
	  "\
    --debug=flag     Set debug flag (commulative).\r\n"
#endif
"\
    --help           Display this text.\r\n\
    --info           Show operation modes and other details, then exit.\r\n\
    --permanent, -p  Keep the server in memory when the client exits.\r\n\
    --unload, -u     Remove a resident server from memory.\r\n\
    --version        Show program version, then exit.\r\n",
	  argv0);
}
/* ---------------------------------------------------------------------- */
static void
info (int unload)
{
  printf ("\
Server version ..................: %d.%02d\r\n\
Resident status .................: %s\r\n\
Number of active clients ........: %d\r\n\
DPMI version ....................: %d.%02d\r\n\
Mode switch technique ...........: %s\r\n\
A20 line control ................: %s\r\n\
Memory source ...................: %s\r\n\
Cpu .............................: %d86\r\n\
Dos version .....................: %d.%02d\r\n",
	  DPMI_OEM_VERSION_MAJOR, DPMI_OEM_VERSION_MINOR,
	  (unload == 0)
	  ? "Resident and removeable"
	  : ((unload == 1)
	     ? "Not resident"
	     : "Resident, not currently removeable"),
	  client_count,
	  DPMI_VERSION_MAJOR, DPMI_VERSION_MINOR,
	  vcpi_present ? "VCPI" : "Raw",
	  vcpi_present ? "VCPI" : (xms_present ? "XMS" : "ports"),
	  (memory_source == MEMORY_FROM_VCPI)
	  ? "VCPI"
	  : ((memory_source == MEMORY_FROM_XMS)
	     ? "XMS"
	     : "BIOS"),
	  cpu,
	  dos_major, dos_minor);
}
/* ---------------------------------------------------------------------- */
static void
version ()
{
  eprintf ("DPMI server version %d.%02d implementing DPMI %d.%02d"
#ifdef DEBUG
	   " (debugging)"
#endif
	   "\r\n",
	   DPMI_OEM_VERSION_MAJOR, DPMI_OEM_VERSION_MINOR,
	   DPMI_VERSION_MAJOR, DPMI_VERSION_MINOR);
}
/* ---------------------------------------------------------------------- */
static int
match (const char *opt, const char *name)
{
  int len = strlen (opt);
  return (len != 0) && (strncmp (opt, name, len) == 0);
}
/* ---------------------------------------------------------------------- */
int
parse_command_line (int unload)
{
  unsigned char *p, ch, *cmdline;
  int exit_class = 2;
  word16 seg;
  __dpmi_regs regs;

  one_shot_mode = 1;

  /* Grab the prefix seg of the current process so that extra invocations
     will have their command lines interpreted by the resident server.  */
  regs.h.ah = DOS_GET_PSP;
  server_int (INT_DOS, &regs);
  seg = regs.x.bx;

  cmdline = LINEAR_TO_PTR ((seg << 4) + DOS_CMDLINE_OFFSET);
  /* Make the command line zero terminated.  */
  if (*cmdline > 0x7e)
    cmdline[0x7f] = 0;
  else
    cmdline[*cmdline + 1] = 0;
  cmdline++;

  while (1)
    {
      while (*cmdline == ' ' || *cmdline == '\t') cmdline++;
      if (*cmdline == 0) break;

      p = cmdline;
      while (*cmdline != ' ' && *cmdline != '\t' && *cmdline != 0) cmdline++;
      ch = *cmdline;
      *cmdline = 0;

      if (*p == '-')
	if (p[1] == '-')
	  {
	    if (match (p + 2, "permanent"))
	      {
	      permanent:
		one_shot_mode = 0;
	      }
	    else if (match (p + 2, "unload"))
	      {
	      unload:
		switch (unload)
		  {
		  case 0:
		    exit_class = 3;
		    break;
		  case 1:
		    eprintf ("Error: no server to remove from memory.\r\n");
		    exit_class = 1;
		    break;
		  case 2:
		    eprintf ("Error: cannot remove server at this time.\r\n");
		    exit_class = 1;
		    break;
		  }
		break;
	      }
	    else if (match (p + 2, "info"))
	      {
		info (unload);
		exit_class = 0;
	      }
	    else if (match (p + 2, "help"))
	      {
		help ();
		exit_class = 0;
	      }
	    else if (match (p + 2, "version"))
	      {
		version ();
		exit_class = 0;
	      }
#ifdef DEBUG
	    /* This option can't be abbreviated, sorry.  */
	    else if (strncmp (p + 2, "debug=", 6) == 0)
	      {
		char *q = p + 2 + 6, *end;
		word32 flag;

		flag = strtoul (q, &end, 16);
		if (*end)
		  {
		    eprintf ("Error: malformed debug option: %s\r\n", p);
		    return exit_class = 1;
		  }
		debug_flags |= flag;
	      }
#endif
	    else
	      goto bad_option;
	  }
	else
	  {
	    if (p[1] == 'p' && p[2] == 0) goto permanent;
	    if (p[1] == 'u' && p[2] == 0) goto unload;
	    goto bad_option;
	  }
      else
	{
	bad_option:
	  eprintf ("Error: unknown option: %s\r\n", p);
	  exit_class = 1;
	  break;
	}

      *cmdline = ch;
    }

  return exit_class;
}
/* ---------------------------------------------------------------------- */
