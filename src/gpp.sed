# Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details
#
# For building the GPP support option file code strip out 
# C options that are not allowed for C++ source
#
s:-Wbad-function-cast::g
s:-Wmissing-declarations::g
s:-Wmissing-prototypes::g
s:-Wstrict-prototypes::g
