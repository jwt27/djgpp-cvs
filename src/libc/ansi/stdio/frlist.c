/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <io.h>
#include <libc/file.h>
#include <libc/local.h>

static __file_rec __initial_file_rec;
__file_rec *__file_rec_list = &__initial_file_rec;

extern void __setup_file_rec_list(void);

/* This is done dynamically (called from __crt1_startup) rather than
   statically, because otherwise any program which restarts itself
   (Emacs) will reuse stale FILE objects from the time it was dumped.  */
void
__setup_file_rec_list(void)
{
  __initial_file_rec.next = 0;
  __initial_file_rec.count = 5;
  __initial_file_rec.files[0] = stdin;
  __initial_file_rec.files[1] = stdout;
  __initial_file_rec.files[2] = stderr;
  __initial_file_rec.files[3] = stdprn;	/* in reverse order! (history) */
  __initial_file_rec.files[4] = stdaux;

  /* A parent program may have closed one or more of the standard 0..4 DOS 
     handles.  If so, we disable the FILE structure so I/O will fail.
     The exit handler may close this structure, but since the handle is
     impossible it will not cause flushing problems with a new file which
     may have been opened on that handle.  If the FILE structure is
     freopened it will flush OK on exit.
     
     Note: fileno() is a macro, so can be on the LHS of assignment. */

  if(_get_dev_info(fileno(stdin)) == -1)
    fileno(stdin) = -1;

  if(_get_dev_info(fileno(stdout)) == -1)
    fileno(stdout) = -1;

  if(_get_dev_info(fileno(stderr)) == -1)
    fileno(stderr) = -1;

  if(_get_dev_info(fileno(stdprn)) == -1)
    fileno(stdprn) = -1;

  if(_get_dev_info(fileno(stdaux)) == -1)
    fileno(stdaux) = -1;
}
