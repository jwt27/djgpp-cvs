# Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details
#
# For building the profiling support code (src/libc/crt0/mcount.c),
# strip out profiling options, to avoid a chicken-and-egg problem
# in a profiling version of libc (the profiling support code calling
# profiling support code).
#
s:^-a$::
s:^-ax$::
s:^-finstrument-functions$::
s:^-fprofile-arcs$::
s:^-ftest-coverage$::
s:^-p$::
s:^-pg$::
