/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>
#include <unistd.h>
#include <io.h>

static char msg[] = "Abort!\r\n";

void
abort()
{
  _write(STDERR_FILENO, msg, sizeof(msg)-1);
  _exit(1);
}
