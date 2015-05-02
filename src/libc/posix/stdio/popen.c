/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2004 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
   This is popen() and pclose() for MSDOS.  They were developed using
   Microsoft C, but should port easily to DOS C any compiler.
   
   Original author: pacetti@fl-ngnet.army.mil

   These routines are hacks, that is they SIMULATE their UNIX
   counterparts.  Since MSDOS and won't allow concurrent process spawning,
   you can't really pipe.  I came up with this nearly stupid way around
   piping because I wanted to have some portability between UNIX and MSDOS.
   I'm probably not the first person to have this idea or implement it, but
   I think I'm the first to give it away for free (no points?!).

   The functions simulate popen() and pclose() by redirecting stdin or
   stdout, then spawning a child process via system().

   If you popen() for read, the stdout is redirected to a temporary
   file, and the child is spawned.  The stdout is reopened via dup2(), the
   temporary file is opened for read and a file pointer to it is returned.

   If you popen() for write, a temporary file is opened for write, and
   a file pointer to it is returned.  When you pclose(), the stdin is
   redirected to the temporary file and the child is spawned.

   In both cases, pclose() closes and unlinks the temporary file.

   A static linked list of C structures is built to store necessary
   info for all popen()ed files so you can popen() more than one file at
   a time.

   The popen() function will return NULL on an error, or a valid FILE
   *pointer on a successful open.  The pclose() function will return
   negative one (-1) on an error or zero (0) on a successful close.

   The function prototypes are:

   FILE *popen(command, mode)
        char *command, char *mode;

   int pclose(pp)
       FILE *pp;

   Where command is a character string equivilant to a MSDOS command
   line, mode is "r" for read or "w" for write, and pp is a pointer to a
   file opened through popen().
 */

#include <libc/stubs.h>
#include <io.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libc/file.h>

/* hold file pointer, command, mode, and the status of the command */
struct pipe_list {
  FILE *fp;
  int exit_status;
  char *command, mode[10];
  struct pipe_list *next;
};

/* static, global list pointer */
static struct pipe_list *pl = NULL;

FILE *popen(const char *cm, const char *md) /* program name, pipe mode */
{
  struct pipe_list *l1;
  char *temp_name;

  /* make new node */
  if ((l1 = malloc(sizeof(*l1))) == NULL)
    return NULL;

  /* if empty list - just grab new node */
  l1->next = pl;
  pl = l1;

  /* stick in elements we know already */
  l1->command = NULL;

  if ((temp_name = malloc(L_tmpnam)) == NULL)
    goto error;

  if (tmpnam(temp_name) == NULL)
    goto error;

  strcpy(l1->mode, md);

  if (l1->mode[0] == 'r')
  /* caller wants to read */
  {
    int fd;

    /* dup stdout */
    if ((fd = dup(fileno(stdout))) == -1)
      goto error;

    /* redirect stdout */
    if (!freopen(temp_name, "wb", stdout))
      goto error;

    /* make sure file is removed on abnormal exit */
    stdout->_flag |= _IORMONCL;
    stdout->_name_to_remove = temp_name;

    /* execute command */
    l1->exit_status = system(cm);

    /* don't remove file */
    stdout->_flag &= ~_IORMONCL;
    stdout->_name_to_remove = NULL;

    /* flush file just in case */
    fflush(stdout);

    /* reopen real stdout */
    if (dup2(fd, fileno(stdout)) == -1)
      goto error;

    /* close duplicate stdout */
    close(fd);

    /* if cmd couldn't be run, make sure we return NULL */
    if (l1->exit_status == -1)
      goto error;
  }
  else if (l1->mode[0] == 'w')
  /* caller wants to write */
  {
    /* if can save the program name, build temp file */
    if (!(l1->command = malloc(strlen(cm) + 1)))
      goto error;

    strcpy(l1->command, cm);
  }
  else
    /* unknown mode */
    goto error;

  /* open file for caller */
  l1->fp = fopen(temp_name, l1->mode);

  /* make sure file is removed on abnormal exit */
  l1->fp->_flag |= _IORMONCL;
  l1->fp->_name_to_remove = temp_name;

  return l1->fp;

 error:

  if (temp_name)
    free(temp_name);

  pl = l1->next;
  free(l1);

  return NULL;
}

int pclose(FILE *pp)
{
  struct pipe_list *l1, **l2;	/* list pointers */
  char *temp_name;		/* file name */
  int retval = -1;		/* function return value */

  for (l2 = &pl; *l2; l2 = &(*l2)->next)
    if ((*l2)->fp == pp)
      break;

  if (!*l2)
    return -1;

  l1 = *l2;
  *l2 = l1->next;

  if (!(l1->fp->_flag & _IORMONCL))
    /* file wasn't popen()ed */
    return -1;
  else
    temp_name = l1->fp->_name_to_remove;

  /* if pipe was opened to write */
  if (l1->mode[0] == 'w')
  {
    int fd;

    /* don't remove file while closing */
    l1->fp->_flag &= ~_IORMONCL;
    l1->fp->_name_to_remove = NULL;

    /* close the (hopefully) popen()ed file */
    fclose(l1->fp);

    /* dup stdin */
    if ((fd = dup(fileno(stdin))) == -1)
      goto exit;

    /* redirect stdin */
    if (!freopen(temp_name, "rb", stdin))
      goto exit;

    /* make sure file is removed on abnormal exit */
    stdin->_flag |= _IORMONCL;
    stdin->_name_to_remove = temp_name;

    /* execute command */
    retval = system(l1->command);

    /* don't remove file */
    stdin->_flag &= ~_IORMONCL;
    stdin->_name_to_remove = NULL;

    /* close and remove file */
    close(fileno(stdin));
    remove(temp_name);

    /* reopen real stdin */
    if (dup2(fd, fileno(stdin)) == -1)
    {
      retval = -1;
      goto exit;
    }

    /* close duplicate stdin */
    close(fd);

  exit:

    free(l1->command);
  }
  /* if pipe was opened to read, return the exit status we saved */
  else if (l1->mode[0] == 'r')
  {
    retval = l1->exit_status;

    /* close and remove file */
    fclose(l1->fp);
  }
  else
    /* invalid mode */
    retval = -1;

  free(l1);

  return retval;
}
