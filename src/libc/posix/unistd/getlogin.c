/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <stdlib.h>

static char default_login[] = "dosuser";

char *
getlogin(void)
{
  char *p;

  p = getenv("USERNAME");
  if (!p)
    p = getenv("LOGNAME");
  if (!p)
    p = getenv("USER");
  if (!p)
    p = default_login;
  return p;
}
