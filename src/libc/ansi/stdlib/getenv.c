/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>
#include <string.h>

extern char **environ;

char *
getenv(const char *name)
{
  int i;

  if (environ == 0)
    return 0;

  for (i=0; environ[i]; i++)
  {
    char *ep = environ[i];
    const char *np = name;
    while (*ep && *np && *ep == *np && *np != '=')
      ep++, np++;
    if (*ep == '=' && *np == 0)
      return ep+1;
  }
  return 0;
}
