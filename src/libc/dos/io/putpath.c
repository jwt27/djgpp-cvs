/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <libc/dosio.h>
#include <libc/farptrgs.h>
#include <go32.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <io.h>
#include <dpmi.h>

static const char env_delim = '~';

/* Can't use stackavail, since it pollutes the namespace...  */
static int __inline__
enough_stack_p(void)
{
  extern unsigned __djgpp_stack_limit;
  unsigned sp;
  __asm__ __volatile__ ("movl %%esp,%k0\n" : "=r" (sp) : );
  return (int) (sp - __djgpp_stack_limit) > 4*1024;
}

int
_put_path(const char *path)
{
  return _put_path2(path, 0);
}

int
_put_path2(const char *path, int offset)
{
  size_t o = __tb+offset;
  int space = __tb_size - offset;
  const char *p = path;

  if (path == 0)
  {
    errno = EFAULT;
    abort();
  }

  _farsetsel(_dos_ds);

  if (p[0] && p[1] == ':')
    p += 2;
  if (strncmp(p, "/dev/", 5) == 0)
  {
    if (strcmp(p+5, "null") == 0)
      path = "nul";
    else if (strcmp(p+5, "tty") == 0)
      path = "con";
    else if (strcmp(p+5, "env") == 0)
      /* Keep it as is to avoid referencing an 'env' directory in the
	 current directory */
      ;
    else if (((p[5] >= 'a' && p[5] <= 'z')
	      || (p[5] >= 'A' && p[5] <= 'Z'))
	     && (p[6] == '/' || p[6] == '\\' || p[6] == '\0'))
    {
      /* map /dev/a/ to a:/ */
      _farnspokeb(o++, p[5]);
      _farnspokeb(o++, ':');
      path = p + 6;
      space -= 2;
    }
    else if (strncmp(p+5, "env", 3) == 0
             && (p[8] == '/' || p[8] == '\\') && p[9])
    {
      /* /dev/env/FOO/bar: expand env var FOO and generate %FOO%/bar */
      char *var_name, *d;
      char *var_value;
      int new_offset;
      int use_default = 1;
      int c;

      p += 9;           /* point to the beginning of the variable name */
      var_name = alloca(strlen (p) + 1);
      for (d = var_name; *p && *p != '/' && *p != '\\'; *d++ = *p++)
        if (*p == env_delim)
        {
          if (p[1] == env_delim)      /* two ~ in a row mean a literal ~ */
            p++;
          else
            break;
        }
      *d = '\0';
      var_value = getenv(d = var_name);
      if (var_value && *var_value)
      {
        /* The value of the env var can include special constructs
           like /dev/x/foo or even a reference to another env var, so
           we need to recursively invoke ourselves.  */
	if (!enough_stack_p())
	{
	  /* This is probably a case of infinite recursion caused by
	     a self-referencing /dev/env/foo value, in which case
	     ENAMETOOLONG is probably right.  But it could also happen
	     if they were short on stack to begin with, in which case
	     we would lie if we use ENAMETOOLONG.  So:  */
	  errno = ENOMEM;
	  return offset;
	}
        new_offset = _put_path2(var_value, offset);
        space -= new_offset - offset;
        o += new_offset - offset;
        use_default = 0;
      }
      if (*p == env_delim)    /* use or skip the default value if present */
      {
        for (++p; *p; p++)
        {
          if (*p == env_delim)
          {
            if (p[1] == env_delim)
              p++;
            else
              break;
          }
          if (use_default)
            *d++ = *p;
        }
        if (use_default)
        {
          *d = '\0';
          /* The default value may use special constructs as well.  */
	  if (!enough_stack_p())	/* infinite recursion? */
	  {
	    errno = ENOMEM;
	    return offset;
	  }
          new_offset = _put_path2(var_name, offset);
          space -= new_offset - offset;
          o += new_offset - offset;
        }
        if (*p == env_delim)	/* a luser could forget the trailing '~' */
          p++;
      }

      /* if the rest of path begins with a slash, remove the trailing
         slash in the transfer buffer */
      if ((*p == '/' || *p == '\\') && o-1 >= __tb+offset
          && ((c = _farnspeekb(o-1)) == '/' || c == '\\'))
        o--;
      path = p;
    }
    else	/* Don't recognize item after /dev, remove if no directory */
    {
      __dpmi_regs r;
      const long *q;

      q = (const long *)path;
      _farnspokel(o, *q);			/* First 8 characters */
      _farnspokel(o+4, *(q+1));
      if (path[4] == '/')
        _farnspokeb(o+4, 0);			/* /dev */
      else
        _farnspokeb(o+6, 0);			/* d:/dev */

      r.x.ax = 0x4300;
      r.x.dx = __tb_offset;
      r.x.ds = __tb_segment;
      __dpmi_int(0x21, &r);
      if ((r.x.flags & 1) || !(r.x.cx & 0x10) )	/* Exist?  Directory? */
        if (p[5])
          path = p + 5;
    }
  }

  /* collapse multiple slashes to a single slash */
  for (; *path; path++)
  {
    if (path[0] != '/' || path[1] != '/')
    {
      _farnspokeb(o, *path);
      o++;
      if (--space < 2) /* safety check */
	break;
    }
  }

  /* remove trailing slash if it doesn't
     represent the root directory */
  if (o-2 >= __tb+offset
      && _farnspeekb(o-1) == '/'
      && _farnspeekb(o-2) != ':')
    o--;

  /* null terminate it */
  _farnspokeb(o, 0);
  return o - __tb;
}
