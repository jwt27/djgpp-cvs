/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <libc/system.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <process.h>
#include <libc/dosexec.h>
#include <libc/unconst.h>

extern char **environ;
#define alloca __builtin_alloca

static int
plainsystem(const char *command)
{
  char *newargs[3];

  /* If redirection is handled then it is OK to use "<>|" since they must
     be inside double quotes which newer MS-DOS versions allow.  */
  if ((__system_flags & __system_redirect) || strpbrk(command, "><|") == 0)
  {
    char *endcmd;
    char cmd[256];
    char found_at[256];
    int i, j;
    for (i=0; command[i]<=' ' && i<255; i++);
    for (j=0; command[i]>' ' && i<255; i++, j++)
      cmd[j] = command[i];
    cmd[j] = 0;
    newargs[0] = cmd;
    newargs[1] = command[i] ? unconst(command+i+1,char *) : 0;
    newargs[2] = 0;
    __dosexec_in_system = 1;
    if (__dosexec_find_on_path(cmd, environ, found_at))
    {
      int rv = __spawnve(P_WAIT, found_at, newargs, (char * const *)environ);
      __dosexec_in_system = 0;
      return rv;
    }
    __dosexec_in_system = 0;
    endcmd = 0;
    for (i=0; cmd[i]; i++)
    {
      if (cmd[i] == '.')
	endcmd = cmd+i+1;
      if (cmd[i] == '\\' || cmd[i] == '/')
	endcmd = 0;
    }
    if (endcmd)
    {
      if (stricmp(endcmd, "exe") == 0 || stricmp(endcmd, "com") == 0)
      {
	errno = ENOENT;
	return -1;
      }
    }
  }

  newargs[0] = 0;
  newargs[1] = 0;
  newargs[2] = 0;
  return __dosexec_command_exec(command, newargs, environ);
}

/* ------------------------------------------------------------------------ */
/* Now follows the overlay that handles redirection.  There should be no    */
/* fixed limits in this code.                                               */

int __system_flags = __system_redirect | __system_handle_null_commands;

static inline int
emiterror (const char *s)
{
  write (2, s, strlen (s));
  return 1;
}

static int CHECK_FOR(const char *s, const char *cmd, int cmdl);
static int
CHECK_FOR(const char *s, const char *cmd, int cmdl)
{
  while (cmdl)
  {
    if (*s++ != *cmd++)
      return 0;
    cmdl--;
  }
  if (isgraph(*s))
    return 0;
  return 1;
}

/* This function handles certain dummy commands and passes the rest to
   plainsystem.  */
static int
system1 (const char *s)
{
  if ((__system_flags & __system_handle_null_commands)
      && ((CHECK_FOR (s, "cd", 2))
	  || CHECK_FOR (s, "rem", 3)
	  || (CHECK_FOR (s, "set", 3))
	  || CHECK_FOR (s, "exit", 4)
	  || CHECK_FOR (s, "goto", 4)
	  || (CHECK_FOR (s, "path", 4))
	  || (CHECK_FOR (s, "chdir", 5))
	  || CHECK_FOR (s, "shift", 5)
	  || (CHECK_FOR (s, "prompt", 6))))
    return 0;
#if 0
  else if ((__system_flags & __system_handle_echo) && CHECK_FOR ("echo", 4))
    return do_echo (s + 4);
#endif
  else if (*s == 0)
    return emiterror ("Invalid null command.\n");
  else
    return plainsystem (s);
}

/* This function handles redirection and passes the rest to system1.  */
int
system (const char *cmdline)
{
  /* Special case: NULL means just exec the command interpreter.  */
  if (cmdline == 0)
    cmdline = "";

  /* Strip initial spaces.  */
  while (isspace(*cmdline))
    cmdline++;

  /* Special case: empty string means just exec the command interpreter.  */
  if (*cmdline == 0)
    return plainsystem (getenv("COMSPEC"));

  if (__system_flags & __system_redirect)
  {
    char *f_in = 0, *f_out = 0;
    int rm_in = 0, rm_out = 0;
    /* Assigned to silence -Wall */
    int h_in = 0, h_inbak = 0, h_out = 0, h_outbak = 0;
    char *s, *t, *u, *v, *tmp;
    int needcmd = 1, result = 0, again;

    tmp = alloca(L_tmpnam);

    s = strcpy (alloca (strlen (cmdline) + 1), cmdline);
    while ((*s || needcmd) && result >= 0)
    {
      if (rm_in)
	remove (f_in);
      f_in = f_out;		/* Piping.  */
      rm_in = rm_out;
      f_out = 0;
      rm_out = 0;
      needcmd = 0;
      while (isspace (*s)) s++;
      t = s;
      do {
	again = 0;
	switch (*t)
	{
	case '<':
	  if (f_in)
	  {
	    result = emiterror ("Ambiguous input redirect.\n");
	    goto leave;
	  }
	  u = t + 1;
	  while (isspace (*u)) u++;
	  v = u;
	  while (!isspace (*v) && *v) v++;
	  f_in = strncpy (alloca (v - u + 1), u, v - u);
	  f_in[v - u] = 0;
	  strcpy (t, v);
	  again = 1;
	  break;
	case '>':
	  if (f_out)
	  {
	    result = emiterror ("Ambiguous output redirect.\n");
	    goto leave;
	  }
	  u = t + 1;
	  while (isspace (*u)) u++;
	  v = u;
	  while (!isspace (*v) && *v) v++;
	  f_out = strncpy (alloca (v - u + 1), u, v - u);
	  f_out[v - u] = 0;
	  strcpy (t, v);
	  again = 1;
	  break;
	case '|':
	  if (f_out)
	  {
	    result = emiterror ("Ambiguous output redirect.\n");
	    goto leave;
	  }
	  needcmd = 1;

	  /* tmpnam guarantees unique names */
	  tmpnam(tmp);
	  f_out = strcpy (alloca (L_tmpnam), tmp);
	  rm_out = 1;
	  /* Fall through.  */
	case 0:
	  u = t + (*t != 0);
	  while (t > s && isspace (t[-1])) t--;
	  *t = 0;
#ifdef TEST
	  fprintf (stderr, "Input from: %s\nOutput to:  %s\n",
		   f_in ? f_in : "<stdin>",
		   f_out ? f_out : "<stdout>");
	  fflush (stderr);
#endif
	  if (f_in)
	  {
	    h_in = open (f_in, O_RDONLY | O_BINARY);
	    h_inbak = dup (0);
	    dup2 (h_in, 0);
	  }
	  if (f_out)
	  {
	    h_out = open (f_out,
			  O_WRONLY | O_BINARY | O_CREAT | O_TRUNC,
			  S_IREAD | S_IWRITE);
	    h_outbak = dup (1);
	    dup2 (h_out, 1);
	  }
	  if (f_in && h_in < 0)
	    result = emiterror ("File not found.\n");
	  else if (f_in && h_inbak < 0)
	    result = emiterror ("Out of file handles.\n");
	  else if (f_out && h_out < 0)
	    result = emiterror ("File creation error.\n");
	  else if (f_out && h_outbak < 0)
	    result = emiterror ("Out of file handles.\n");

	  if (!result)
	  {
#ifdef TEST
	    fprintf (stderr, "system (\"%s\");\n", s);
	    fflush (stderr);
#endif
	    result = system1 (s);
	  }
	  if (f_in)
	  {
	    dup2 (h_inbak, 0);
	    close (h_in);
	  }
	  if (f_out)
	  {
	    dup2 (h_outbak, 1);
	    close (h_out);
	  }
	  s = u;
	  break;
	case '"':
	  /* Newer MS-DOS versions allow the use of <|> inside double
	     quotes.  */
	  t++;
	  while (*t && *t != '"') t++;
	  if (*t) t++;
	  again = 1;
	  break;
	default:
	  t++;
	  again = 1;
	  break;
	}
      } while (again);
    }
  leave:
    if (rm_in)
      remove (f_in);
    if (rm_out)
      remove (f_out);

    return result;
  }
  else
    return system1 (cmdline);
}
