/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int
setenv(const char *var, const char *val, int replace)
{
  char *prev;

  errno = 0;

  if (var == (char *)0 || val == (char *)0)
  {
    errno = EINVAL;
    return -1;
  }

  if ((prev  = getenv (var)) && !replace)
    return 0;
  else
    {
      size_t l_var = strlen (var);
      char *envstr = (char *)alloca (l_var + strlen (val) + 2);
      char *peq    = strchr (var, '=');

      if (*val == '=')
        ++val;
      if (peq)
        l_var = peq - var;

      strncpy (envstr, var, l_var);
      envstr[l_var++] = '=';
      strcpy (envstr + l_var, val);

      return putenv (envstr);
    }
}

#ifdef  TEST

#include <stdio.h>

int
main(void)
{
  char p[] = "ENVTEST=set-with-setenv-in-a-single-string";

  printf("ENVTEST=%s\n", getenv("ENVTEST"));
  putenv("ENVTEST=set-with-putenv");
  printf("ENVTEST=%s\n", getenv("ENVTEST"));
  setenv("ENVTEST", "set-with-setenv-safely", 0);
  printf("ENVTEST=%s\n", getenv("ENVTEST"));
  setenv("ENVTEST", "set-with-setenv-forcibly", 1);
  printf("ENVTEST=%s\n", getenv("ENVTEST"));
  setenv("ENVTEST", "=set-with-setenv-with-equals", 1);
  printf("ENVTEST=%s\n", getenv("ENVTEST"));
  setenv(p, p + 8, 1);
  printf("ENVTEST=%s\n", getenv("ENVTEST"));
  setenv("ENVTEST", "", 1);
  printf("ENVTEST=%s\n", getenv("ENVTEST"));

  return 0;
}

#endif
