/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */

/* Redirection of inferior's standard handles.

   These functions are needed when a debugger wants to redirect
   standard handles of the debuggee, or if the debuggee redirects
   some of its standard handles.  If you don't see the problem right
   away, read on.

   The debuggee is not a separate process, we just pretend it is by
   jumping between two threads of execution.  But, as far as DOS is
   concerned, the debugger and the debuggee are a single process, and
   they share the same Job File Table (JFT).  The JFT is a table
   maintained by DOS in the program's PSP where, for each open handle,
   DOS stores the index into the SFT, the System File Table.  (The SFT
   is an internal data structure where DOS maintains everything it
   knows about a certain open file/device.)  A handle that is returned
   by `open', `_open' and other similar functions is simply an index
   into the JFT where DOS stored the SFT entry index for the file or
   device that the program opened.

   When a program starts, the first 5 entries in the JFT are
   preconnected to the standard devices.  Any handles opened by either
   the debugger or the debuggee use handles beyond the first 5 (unless
   one of the preconnected handles is deliberately closed).  Here we
   mostly deal with handles 0, 1 and 2, the standard input, standard
   output, and standard error; they all start connected to the console
   device (unless somebody redirects the debugger's I/O from the
   command line).

   Since both the debugger and the debuggee share the same JFT, their
   handles 0, 1 and 2 point to the same JFT entries and thus are
   connected to the same files/devices.  Therefore, if the debugger
   redirects its standard output, the standard output of the debuggee
   is also automagically redirected to the same file/device!  Similarly,
   if the debuggee redirects its stdout to a file, you won't be able to
   see debugger's output (it will go to the same file where the debuggee
   has its output); and if the debuggee closes its standard input, you
   will lose the ability to talk to debugger!
   
   The code below attempts to solve all these problems by creating an
   illusion of two separate sets of standard handles.  Each time the
   debuggee is about to be run or resumed, the functions provided below
   redirect debugger's own standard handles to the file specified in the
   command-line (as given by e.g. the "run" command of GDB) before
   running the debuggee, then redirects them back to the debugger's
   original input/output when the control is returned from the debuggee
   (e.g. after a breakpoint is hit).  Although the debugger and the
   debuggee have two separate copies of the file-associated data
   structures, the debugger still can redirect standard handles of the
   debuggee because they use the same JFT entries as debugger's own
   standard handles.

   Bugs: We only watch the first 3 handles, so if the debuggee redirects
   or closes handles 3 and 4, or any other handle that is inherited by
   the debugger from its parent, the debugger won't notice.  (If you want
   to fix that, you should know that the exit code of a DJGPP program
   forcibly closes all handles beyond the first 3 ones, so the debugger
   will lose all such handles when the debuggee exits normally.  For
   example, currently the debugger loses stdaux and stdprn after the
   debuggee exits.)

   Written by Eli Zaretskii.  */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <debug/redir.h>

/* Since the debuggee might close handles 3 and 4 as part of its
   exit code, we must make sure handle 4 and below are NEVER used
   for our redirected handles.  Hence the need for the following
   three functions.  */

/* Move a file descriptor FD such that it is at least MIN_FD.
   If the file descriptor is changed (meaning it was origially
   *below* MIN_FD), the old one will be closed.
   If the operation failed (no more handles available?), -1 will
   be returned, in which case the original descriptor is still
   valid.

   This jewel is due to Morten Welinder <terra@diku.dk>.  */
static int
move_fd (int fd, int min_fd)
{
  int new_fd, tmp_fd;

  if (fd == -1 || fd >= min_fd)
    return fd;

  tmp_fd = dup (fd);
  if (tmp_fd == -1)
    return tmp_fd;

  new_fd = move_fd (tmp_fd, min_fd);
  if (new_fd != -1)
    close (fd);		/* success--get rid of the original descriptor */
  else
    close (tmp_fd);	/* failure--get rid of the temporary descriptor */
  return new_fd;
}

/* Open a file like `open' does, but make sure the returned handle
   is never less than 5, to avoid messing with the preconnected handles.  */
static int
safe_open (const char *path, int oflags, int shflags)
{
  int fd = open (path, oflags, shflags);
  int safe_fd;

  if (fd != -1 && fd < 5)
    {
      safe_fd = move_fd (fd, 5);
      if (safe_fd != -1)
	fd = safe_fd;
    }
  return fd;
}


/* Duplicate a handle like `dup' does, but make sure the returned handle
   is never less than 5, to avoid messing with the preconnected handles.  */
static int
safe_dup (int fd)
{
  int new_fd = dup (fd);
  int safe_fd;

  if (new_fd != -1 && new_fd < 5)
    {
      safe_fd = move_fd (new_fd, 5);
      if (safe_fd != -1)
	new_fd = safe_fd;
    }
  return new_fd;
}

/* Given a string like ">> foo", create a new redirection object.  */
static struct dbg_redirect *
redir_new (const char *redir_string)
{
  const char *fn = redir_string + 1;
  char fullpath[FILENAME_MAX];
  int mode, handle;
  struct dbg_redirect *result;

  switch (*redir_string) {
    default:
      return NULL;
    case '<':
      mode = O_RDONLY;
      handle = 0;
      break;
    case '>':
      mode = O_WRONLY | O_CREAT;
      handle = 1;
      if (*fn == '>')
	{
	  fn++;
	  mode |= O_APPEND;
	}
      else
	mode |= O_TRUNC;
      break;
  }

  fn++;	/* skip the delimiting space */
  result = (struct dbg_redirect *) malloc (sizeof (struct dbg_redirect));
  if (result != NULL)
    {
      /* Store the full path name, in case we chdir.  */
      _fixpath (fn, fullpath);
      result->file_name = strdup (fullpath);
      if (result->file_name)
	{
	  result->inf_handle = result->our_handle = handle;
	  result->mode = mode;
	  result->filepos = (off_t)-1;	/* unknown */
	}
      else
	{
	  free (result);
	  result = NULL;
	}
    }

  return result;
}


/* Close redirected handles and free the redirection object.  */

static int
redir_delete (struct dbg_redirect *redir)
{
  int result = 0;

  if (redir)
    {
      if (redir->inf_handle > DBG_HANDLES-1) /* leave standard handles alone */
	result = close (redir->inf_handle);
      if (redir->our_handle > DBG_HANDLES-1)
	result |= close (redir->our_handle);
      if (redir->file_name)
	free (redir->file_name);
      free (redir);
    }
  return result;
}

/* Open a file and plug the info into a struct dbg_redirect.  */
static int
redir_open (int fd, struct dbg_redirect *redir)
{
  int desc = safe_open (redir->file_name, redir->mode, 0644);

  if (desc != -1)
    {
      redir->inf_handle = desc;
      redir->our_handle = fd;
      redir->filepos = lseek (desc, 0L, SEEK_CUR);
    }

  return desc;
}

/* Make the descriptor FD reference the file that inferior opened on
   that descriptor, and remember the debugger's device opened on FD.  */
static int
redir_handle_to_child (int fd, struct dbg_redirect *redir)
{
  int save_fd, status = 0;

  save_fd = safe_dup (fd);
  if (save_fd == -1 && errno != EBADF)	/* EBADF probably means FD is closed */
    status = -1;

  redir->our_handle = save_fd;
  if (redir->inf_handle == -1)	/* it was closed in the inferior */
    {
      if (save_fd != -1)
	status = close (fd);
    }
  else
    {
      if (dup2 (redir->inf_handle, fd) == -1
	  || (fd != redir->inf_handle && close (redir->inf_handle) == -1))
	status = -1;
      else
	redir->inf_handle = -1;	/* we've just closed it */
    }

  return status == -1 ? -1 : fd;
}

/* Make the descriptor FD reference the file which the debugger
   had originally open on that descriptor.  */
static int
redir_handle_to_debugger (int fd, struct dbg_redirect *redir)
{
  int save_fd, status = 0;

  save_fd = safe_dup (fd);
  if (save_fd == -1 && errno != EBADF)	/* EBADF probably means FD is closed */
    status = -1;

  redir->inf_handle = save_fd;
  if (redir->our_handle == -1)	/* it was closed in the debugger */
    {
      if (save_fd != -1)
	status = close (fd);
    }
  else
    {
      if (dup2 (redir->our_handle, fd) == -1
	  || (fd != redir->our_handle && close (redir->our_handle) == -1))
	status = -1;
      else
	redir->our_handle = -1;	/* we've just closed it */
    }

  return status == -1 ? -1 : fd;
}

/* Given a pointer to a redirection symbol (presumably followed by a
   target file name), extract the target file name, removing any
   quoting characters as we go, create a redirection object and return
   the number of characters occupied by the symbol and the file name
   in the original command line.  In case of failure return zero.  */

static size_t
get_redirection (const char *s, cmdline_t *cmd)
{
  char buf[FILENAME_MAX + 5];	/* some extra for the redirection symbol */
  char *d = buf;
  const char *start = s;
  int quote = 0;
  struct dbg_redirect *redir;

  *d++ = *s++;			/* copy the redirection symbol */
  if (*s == '>' && *start == '>') /* >> */
    *d++ = *s++;
  while (isspace ((unsigned char)*s))	/* skip whitespace before file name */
    s++;
  *d++ = ' ';			/* separate redirection from file name */

  while (*s)
    {
      if (*s == '\'' || *s == '"')
	{
	  if (!quote)
	    quote = *s++;
	  else if (*s == quote)
	    {
	      quote = 0;
	      s++;
	    }
	  else
	    *d++ = *s++;
	}
      else if (*s == '\\')
	{
	  if (s[1] == '\'' || s[1] == '"') /* escaped quote char */
	    s++;
	  *d++ = *s++;
	}
      else if (isspace ((unsigned char )*s) && !quote)
	break;
      else
	*d++ = *s++;
    }
  *d = '\0';

  /* DOS/Windows don't allow redirection characters in file names,
     so we can bail out early if they use them, or if there's no
     target file name after the redirection symbol.  */
  if (d[-1] == '>' || d[-1] == '<')
    {
      errno = ENOENT;
      return 0;
    }
  else if ((redir = redir_new (buf)) == NULL)
    {
      errno = ENOMEM;
      return 0;
    }

  /* Plug the redirection object into the proper slot, as its standard
     handle tells, and open the file.  We currently support only the
     standard handles.  */
  if (redir->our_handle >= 0 && redir->our_handle < DBG_HANDLES)
    {
      cmd->redirection[redir->our_handle] = redir;
      if (redir_open (redir->our_handle, redir) == -1)
	{
	  cmd->redirection[redir->our_handle] = NULL;
	  redir_delete (redir);
	  return 0;
	}
      return s - start;
    }
  else
    return 0;
}

/* A destructor for a cmdline_t object.  */

void
redir_cmdline_delete (cmdline_t *cmd)
{
  if (cmd)
    {
      if (cmd->command)
	{
	  free (cmd->command);
	  cmd->command = NULL;
	}
      if (cmd->redirection)
	{
	  int i;

	  for (i = 0; i < DBG_HANDLES; i++)
	    {
	      redir_delete (cmd->redirection[i]);
	      cmd->redirection[i] = NULL;
	    }
	}
      cmd->redirected = 0;
    }
}

/* Process the inferior's command line.  Find any redirection symbols,
   create redirection objects and remove the redirection symbols and
   their target file names from the command line.  Create a cmdline_t
   object, and return zero if all's well, non-zero otherwise.  */

int
redir_cmdline_parse (const char *args, cmdline_t *cmd)
{
  const char *s = args;
  size_t cmd_len = 0, cmd_space = strlen (args) + 1;
  char *d = cmd->command = (char *) malloc (cmd_space);
  int quote = 0;

  if (!d || !cmd->redirection)
    return -1;

  while (isspace ((unsigned char)*s))
    s++;

  while (*s)
    {
      if (*s == '\'' || *s == '"')
	{
	  if (!quote)
	    quote = *s;
	  else if (*s == quote)
	    quote = 0;
	}
      else if (*s == '\\')
	{
	  if (s[1] == '\'' || s[1] == '"') /* escaped quote char */
	    s++;
	}
      else if (!quote)
	{
	  if (*s == '<' || *s == '>')
	    {
	      size_t copied = get_redirection (s, cmd);

	      if (copied <= 0)
		return -1;	/* funky command */
	      s += copied;
	    }
	  else if (isspace ((unsigned char)*s)
		   && isspace ((unsigned char)d[-1]))
	    {
	      /* Collapse multiple whitespace characters.  */
	      d--;
	      cmd_len--;
	    }
	  }
      if (*s)
	{
	  *d++ = *s++;
	  cmd_len++;
	}
    }
  *d = '\0';
  /* Free unused space.  */
  cmd->command = (char *) realloc (cmd->command, cmd_len + 1);
  return 0;
}

/* Switch all standard handles to inferior's handles.  */

int
redir_to_child (cmdline_t *cmd)
{
  int i;

  if (!cmd->redirection)
    return -1;
  if (cmd->redirected)
    return 0;

  cmd->redirected = 1;
  for (i = 0; i < DBG_HANDLES; i++)
    if (redir_handle_to_child (i, cmd->redirection[i]) == -1)
      return -1;

  return 0;
}

/* Switch all standard handles to our handles.  */

int
redir_to_debugger (cmdline_t *cmd)
{
  int i;

  if (!cmd->redirection)
    return -1;
  if (!cmd->redirected)
    return 0;

  cmd->redirected = 0;
  for (i = DBG_HANDLES-1; i >= 0; i--)
    if (redir_handle_to_debugger (i, cmd->redirection[i]) == -1)
      return -1;

  return 0;
}

int
redir_debug_init (cmdline_t *cmd)
{
  int i;

  redir_cmdline_delete (cmd);

  if (!cmd->redirection)
    {
      cmd->redirection = (struct dbg_redirect **)
	calloc (DBG_HANDLES, sizeof (struct dbg_redirect *));
      if (cmd->redirection == NULL)
	return -1;
    }

  for (i = 0; i < DBG_HANDLES; i++)
    {
      cmd->redirection[i] =
	(struct dbg_redirect *) calloc (1, sizeof (struct dbg_redirect));
      if (!cmd->redirection[i])
	return -1;
      cmd->redirection[i]->inf_handle = cmd->redirection[i]->our_handle = i;
    }

  return 0;
}
