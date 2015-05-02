/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>
#include <glob.h>

void globfree(glob_t *_pglob)
{
  size_t i;
  if (!_pglob->gl_pathv)
    return;
  for (i=0; i<_pglob->gl_pathc; i++)
    if (_pglob->gl_pathv[i])
      free(_pglob->gl_pathv[i]);
  free(_pglob->gl_pathv);
}
