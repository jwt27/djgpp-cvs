/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

#include <libc/stubs.h>
#include <sys/stat.h>
#include "xstat.h"

/* Some fields of struct stat are expensive to compute under DOS,
   because they require multiple disk accesses.  Fortunately, many
   DOS programs don't care about these.  To leave both pedants (like
   me) and performance-oriented guys happy, a variable is provided
   which controls which expensive fields should be computed.  To get
   the fastest stat() for your program, clear the bits for only those
   features you need and set the others.

   This improvement was suggested by Charles Sandmann
   <sandmann@clio.rice.edu> and DJ Delorie <dj@delorie.com>.  */

/* Please see the header file sys/stat.h for the definitions of _STAT_*. */

/* Should we bother about executables at all? */
#define _STAT_EXECBIT       (_STAT_EXEC_EXT | _STAT_EXEC_MAGIC)

/* By default, all the bits are reset (including as yet unused ones), so
   people who don't care will transparently have the full version.  */
unsigned short _djstat_flags;

/* As we depend on undocumented DOS features, we could fail in some
   incompatible environment or future DOS versions.  If we do, the
   following variable will have some of its bits set.  Each bit
   describes a single feature which we tried to use and failed.
   The function _djstat_describe_lossage() may be called to print a
   human-readable description of the bits which were set by the last
   call to f?stat().  This should make debugging f?stat() failures
   in an unanticipated environment a lot easier.

   This improvement was suggested by Charles Sandmann
   <sandmann@clio.rice.edu>.  */

unsigned short _djstat_fail_bits;

