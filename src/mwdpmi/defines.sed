# ---------------------------------------------------------------------------
# Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
# Distributed under the GPL, see COPYING for details.
# ---------------------------------------------------------------------------

# This file changes the assembler file "defines.inc" into a suitable
# C include file equivalent called "defines.h".

1i\
/* Generated automatically from defines.inc -- DO NOT EDIT! */\
\
#ifndef defines_h_included\
#define defines_h_included

/^;;/d
s!; ----!; !
s!;[ 	]*\(.*\)$!/* \1 */!
s!^\([A-Za-z0-9_]*[ 	]*\)=[ 	]*\(.*\)$!#define \1 \2!

# The following line makes Emacs this that we have no local variables
# in this script file.  This must be pretty close to the end of the file,
# or else Emacs might grab the section below which is intended for the
# C file.
# Local Variables: #
# End: #

$a\
#endif /* defines_h_included */\
\
/* Like I said above, you are not supposed to edit this file.  */\
/* Local Variables: */\
/* buffer-read-only: t */\
/* End: */

# This is probably not used by Emacs.  It should though.
# Local Variables: #
# End: #
