/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <dir.h>

struct __dj_DIR {
  int num_read;
  char *name;
  int flags;
  struct ffblk ff;
  struct dirent de;
  int need_fake_dot_dotdot; /* 0=no, 1=.., 2=. */
};
