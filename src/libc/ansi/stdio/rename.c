/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* ------------------------- rename() for DJGPP -------------------------- */

/*
 *  An implementation of rename() which can move both files AND
 *  directories on the same filesystem (in the DOS world this means
 *  the same logical drive).  Most cases are simply passed to the
 *  DOS Int 21h/AH=56h function.  Special treatment is required for
 *  renaming (moving) directories which don't share their parent
 *  directory, because DOS won't allow this.  This is called ``Prune
 *  and graft'' operation.  Most of the code below handles this
 *  special case.  It recursively creates subdirectories at the
 *  target path, moves regular files there, then deletes the (empty)
 *  directories at the source.
 *
 *  An alternative (and much faster) implementation would be to access
 *  the parent directories of the source and the target at the disk
 *  sector level and rewrite them with BIOS calls.  However, this won't
 *  work for networked drives and will leave the file system in an
 *  inconsistent state, should the machine crash before the operation
 *  is completed.  (A hybrid approach which uses the fast method when
 *  possible and the slow one otherwise, is left as an exercise for the
 *  ambitious readers. ;-)
 */

#include <libc/stubs.h>
#include <libc/symlink.h>
#include <libc/bss.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <io.h>
#include <sys/stat.h>
#include <dir.h>

/* -------------------------------------------------------
       Internal variables and static helper functions.
   ------------------------------------------------------- */

/* A stack of directories queued for deletion as soon as they are
   emptied.  Implemented as an array of structures; each element
   contains a ptr into a char array (the pool) where the names of
   the directories are stored, and the length of the pathname of
   that directory.  */

typedef struct _stacked_dir {
  char * dirname;
  int    dirlen;
} Stacked_Dir;

static char        * dirnames_pool;    /* the pool of names */
static int           pool_size;        /* size of the pool */
static Stacked_Dir * dirstack;         /* the stack itself */
static Stacked_Dir * stack_top;        /* top of stack */
static int           stack_size;       /* current stack size */

static char          target[FILENAME_MAX];
static char          source[FILENAME_MAX];
static int           last;

static int           rename_count = -1;

/*
** Initialize the stack.  Make sure the storage is allocated.
*/
static int
init_stack(void)
{
  /* Init static variables at program start, or if we have been
     restarted (emacs).  */
  if (__bss_count != rename_count)
    {
      rename_count = __bss_count;
      dirstack = (Stacked_Dir *)0;
      pool_size = 1024;
      stack_size = 32;
    }

  if (dirstack == 0)
    {
      /* Initialize storage.  */
      dirnames_pool = (char *)malloc(pool_size);
      if (dirnames_pool == 0)
        {
          errno = ENOMEM;
          return 0;
        }

      dirstack = (Stacked_Dir *)malloc(stack_size * sizeof(Stacked_Dir));
      if (dirstack == 0)
        {
          errno = ENOMEM;
          return 0;
        }
    }

  /* Forget the previous contents of the stack.  */
  stack_top = dirstack;
  stack_top->dirname = dirnames_pool;
  stack_top->dirlen  = 0;

  return 1;
}

/*
** Push a directory onto the stack, return non-zero in
** case of success.
*/

static int
push_dir(const char *dir)
{
  int dspace = strlen(dir) + 1;
  char * pool_end;      /* where unised space in pool begins */

  pool_end = stack_top->dirname + stack_top->dirlen;

  /* Ensure we have enough space in the name pool for this directory.  */
  if (pool_end + dspace >= dirnames_pool + pool_size)
    {
      char * temp;

      /* Make its size doubled, plus a space for this directory.  */
      pool_size += dspace + pool_size;
      temp = (char *)realloc(dirnames_pool, pool_size);
      if (temp == 0)
        {
          errno = ENOMEM;
          return 0;
        }
      pool_end += temp - dirnames_pool;
      dirnames_pool = temp;
    }

  /* Bump stack top and check for stack overflow.  */
  if (++stack_top - dirstack >= stack_size)
    {
      /* Not enough storage--reallocate.  */
      Stacked_Dir * temp;

      stack_size *= 2;
      temp = (Stacked_Dir *)realloc(dirstack,
                                    stack_size * sizeof(Stacked_Dir));
      if (temp == 0)
        {
          errno = ENOMEM;
          return 0;
        }
      stack_top += temp - dirstack;
      dirstack = temp;
    }

  /* Now push the directory onto the stack.  */
  stack_top->dirname = strcpy(pool_end, dir);
  stack_top->dirlen = dspace;

  return 1;
}

/*
** Pop a directory off the stack, return its name.
*/

static char *
pop_dir(void)
{
  char * retval;

  if (stack_top < dirstack || stack_top->dirlen == 0) /* empty stack */
    return (char *)0;

  retval = stack_top->dirname;
  stack_top--;
  return retval;
}

/*
** See if DIR1 is a parent of DIR2, return 1 if it is.
** Note that this is in no way a general solution: it only
** works in the context of mover() operation which always
** gets filenames rooted at the same directory.  Otherwise,
** you must pass a fully-qualified pathnames for this to
** work.
*/

static int
is_parent(const char *dir1, const char *dir2)
{
  if (dir1 == 0 || dir2 == 0 || *dir1 == 0)
    return 0;
  while (*dir1 && *dir2 && tolower((unsigned char)*dir1) == tolower((unsigned char)*dir2))
    {
      dir1++;
      dir2++;
    }

  return *dir1 == '\0' && (*dir2 == '/' || *dir2 == '\\');
}

/*
** Main workhorse.  This will be passed to __file_tree_walk()
** to do the actual job of moving a subtree to another branch
** of the filesystem hierarchy.
*/

static int
mover(const char *from, const struct ffblk *ff)
{
  char  to[FILENAME_MAX];

  /* Did we just finish to empty a directory?  */
  if (!is_parent(stack_top->dirname, from))
    {
      /* Remove an empty directory and pop it from stack.  */
      if (remove(pop_dir()))
        return -1;
    }

  strcpy(to, target);
  strcat(to, from + last);

  if (ff->ff_attrib & 0x10)
    {
      /* A directory -- create its namesake and push it onto stack.   */
      if (mkdir(to, ff->ff_attrib & 0xffe7) == -1 ||
          push_dir(from) == 0)
        return -1;
    }
  else  /* a file -- move it */
    {
      if (_rename(from, to) == -1)
        return -1;
    }

  return 0;
}


/* -------------------------------------------------------
       Main entry point and the only exported function
   ------------------------------------------------------- */

int
rename(const char *old, const char *new)
{
  int status;
  char *p;
  int source_attr, target_attr;
  int old_dev, new_dev;
  int e = errno;
  int simple_should_do = 0; /* _rename() enough? */
  int target_exists = 0;
  char real_old[FILENAME_MAX];
  char real_new[FILENAME_MAX];

  /* Much of the following quite tedious administrivia is necessary
     to return a meaningful code in errno.  Otherwise, for most of
     the calamities DOS will feed us the ubiquitous EACCES which
     doesn't tell us much.  */

  /* There are several conditions which we must check upfront,
     to ensure NEW isn't deleted unless rename() is to succeed.  */

  /* Fail with EFAULT if one of OLD or NEW is a NULL ptr.  */
  if (old == 0 || new == 0)
    {
      errno = EFAULT;
      return -1;
    }

  /* Handle symlinks */
  if (!__solve_dir_symlinks(old, real_old) || 
      !__solve_dir_symlinks(new, real_new))
     return -1;

  /* Fail with ENAMETOOLONG if either OLD or NEW are too long.  This is
     explicitly checked so that DOS filename truncation won't fool us.  */
  if (strlen(real_old) > FILENAME_MAX || strlen(real_new) > FILENAME_MAX)
    {
      errno = ENAMETOOLONG;
      return -1;
    }

  /* Fail with ENOENT if OLD doesn't exist or is otherwise
     inaccessible.  */
  if ((source_attr = _chmod(real_old, 0)) == -1)
    return -1;      /* errno set by _chmod() */

  /* Fail with EXDEV, if old and new aren't on the same device.  */
  if (real_old[1] == ':')
    old_dev = toupper((unsigned char)real_old[0]) - 'A';
  else
    old_dev = getdisk();
  if (real_new[1] == ':')
    new_dev = toupper((unsigned char)real_new[0]) - 'A';
  else
    new_dev = getdisk();
  if (old_dev != new_dev)
    {
      errno = EXDEV;
      return -1;
    }

  /* Refuse to rename `.' or `..' or `d:.' or `d:..'  */
  if ((real_old[0] == '.' && (real_old[1] == '\0' || (real_old[1] == '.' && 
       real_old[2] == '\0'))) || (real_old[1] == ':' && real_old[2] == '.' && 
      (real_old[3] == '\0' || (real_old[3] == '.' && real_old[4] == '\0'))))
    {
      errno = EINVAL;
      return -1;
    }

  /* Some problems can only happen if NEW already exists.  */
  errno = 0;
  target_attr = _chmod(real_new, 0);
  if (errno != ENOENT)
    {
      int old_is_dir = (source_attr & 0x10) == 0x10;
      int new_is_dir = (target_attr & 0x10) == 0x10;

      target_exists = 1;

      /* Fail if OLD is a directory while NEW isn't, or vice versa.  */
      if (old_is_dir && !new_is_dir)
        {
          errno = ENOTDIR;
          return -1;
        }
      else if (new_is_dir && !old_is_dir)
        {
          errno = EISDIR;
          return -1;
        }
      else if (old_is_dir && new_is_dir)
        {
          char new_true[FILENAME_MAX], old_true[FILENAME_MAX];

          /* Fail if both OLD and NEW are directories and
             OLD is parent of NEW.  */
          errno = 0;
          if (is_parent(_truename_sfn(real_old, old_true), 
                        _truename_sfn(real_new, new_true)))
            {
              errno = EINVAL;
              return -1;
            }
          else if (errno)
            return -1;
          else
            {
              /* See if these two directories share a common parent.  If
                 they do, then _rename() will do the job for us.  */

              char *s = strrchr(old_true, '\\'), *t = strrchr(new_true, '\\');

              *s = '\0'; /* truncate at end of parent directory name */
              *t = '\0';
              if (strcmp(old_true, new_true) == 0)
                simple_should_do = 1;
            }
        }
      else
        {
          /* They both are files, _rename() must be enough.  */
          simple_should_do = 1;
        }
    }
  else if ((source_attr & 0x10) == 0x10)
    {
      /* Fail to rename OLD -> OLD/whatever.  */
      char new_true[FILENAME_MAX], old_true[FILENAME_MAX];

      errno = 0;
      if (is_parent(_truename_sfn(real_old, old_true), 
                    _truename_sfn(real_new, new_true)))
	{
	  errno = EINVAL;
	  return -1;
	}
      else if (errno)
	return -1;
    }
  else if (target_attr == -1)
    {
      /* Target doesn't exist, and source is not a directory:
	 _rename() must be good enough.  */
      simple_should_do = 1;
    }

  /* On to some REAL work for a change.  Let DOS do the simple job:
     moving a regular file, or renaming a directory.  Note that on
     Windows 95 this will also move a directory to a subtree of
     another parent directory.  */
  if ((status = _rename(real_old, real_new)) == 0)
    {
      errno = e;    /* restore errno we inherited */
      return 0;
    }
  else if (simple_should_do)
    /* If _rename() fails and we KNOW it shouldn't, give up
       and return -1 with whatever errno we have.  */
    return -1;
  else if (errno == EACCES && target_exists && (target_attr & 0x10) &&
           _chmod(real_new, 0) != -1)
    {
      /* If target still exists (i.e., it wasn't removed inside
         _rename()) and is a directory, assume it's not empty.
         (We could verify that with findfirst()/findnext(), but that
         would be too expensive.  Besides, what other reason could DOS
         possibly have for not letting us remove a directory?? ;-)  */

      errno = ENOTEMPTY;
      return -1;
    }
  else
    {
      /* We are in for some dirty work...  Sigh...
         Recursively traverse the directory tree rooted at OLD,
         moving regular files and creating subdirectories at NEW.  */

      int retval;

      strcpy(source, real_old);
      last = strlen(source) - 1;

      strcpy(target, real_new);
      if (strchr(target, '\\'))
        for (p = target; *p; p++)
          {
            if (*p == '\\')
              *p = '/';
          }
      if (source[last] == '/' || source[last] == '\\')
        source[last] = '\0';
      else
        ++last;

      /* Create NEW and push it onto the stack.  */
      if (mkdir(real_new, source_attr & 0xffe7) == -1 ||
          init_stack() == 0 || push_dir(old) == 0)
        return -1;

      /* Process all of its siblings.  */
      retval = __file_tree_walk(source, mover);

      if (retval)
        {

          /* If anything went wrong, remove NEW in a desperate attempt
             to leave everything as we found it.  */
          int savederr = errno;

          remove(target);
          errno = savederr;
        }
      else
        {
          char *p2;

          /* Remove any (empty) directories that remain on the stack.
             This is required because we only detect that a subdirectory
             can be removed when we see the next entry in its parent dir.
             Subdirectories which are the LAST (or the only) entries in
             their parent directories, won't be removed.  */
          while ((p2 = pop_dir()) != 0)
            {
              errno = e;
              if (remove(p2) != 0)
                {
                  int se = errno;

                  remove(target);
                  errno = se;
                  return -1;
                }
            }
        }

      return retval;
    }
}

#ifdef  TEST

#include <stdlib.h>

int
main(int argc, char *argv[])
{
  if (argc > 2)
    {
      char msg[80];

      sprintf(msg, "movdir: %d", rename(argv[1], argv[2]));
      if (errno)
        perror(msg);
      else
        puts(msg);
    }
  else
    printf("Usage: %s from to\n", argv[0]);

  return 0;
}

#endif
