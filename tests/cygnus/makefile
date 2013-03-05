# Makefile for tests/cygnus.

# Originally, Makefile for newlib/libm/test.
# Copyright (c) 1994 Cygnus Support.
# All rights reserved.
#
# Redistribution and use in source and binary forms are permitted
# provided that the above copyright notice and this paragraph are
# duplicated in all such forms and that any documentation,
# advertising materials, and other materials related to such
# distribution and use acknowledge that the software was developed
# at Cygnus Support, Inc.  Cygnus Support, Inc. may not be used to
# endorse or promote products derived from this software without
# specific prior written permission.
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.


# HERE is computed as an absolute file name by using $(shell), so that
# sub-Make's could use CFLAGS even though they run in different directories.
# If you don't have `pwd', replace it with `cd' and make sure your shell is
# NOT a Unix-style one.
HERE := $(shell pwd)
TOP   = $(HERE)/..
CC    = gcc

CRT0     = $(TOP)/../lib/crt0.o
LIBGCCA := $(shell $(CC) -print-file-name=libgcc.a)

# FIXME: this should probably use misc.exe
RM = rm -f

DEFS     = -D_USE_LIBM_MATH_H -D_HAVE_STDC
CPPFLAGS = -nostdinc -I$(HERE) -I$(TOP)/../include
CFLAGS   = $(DEFS) -O2 -g -Wall
LIBS     = $(CRT0) -lm -lc $(LIBGCCA)
LDFLAGS  = -nostdlib -L$(TOP)/../lib

OFILES = test.o string.o convert.o conv_vec.o iconv_vec.o test_is.o \
         dvec.o sprint_vec.o sprint_ivec.o math.o math2.o test_ieee.o

acosVEC_FILES   = acos_vec.c acosf_vec.c
acoshVEC_FILES  = acosh_vec.c acoshf_vec.c
asinVEC_FILES   = asin_vec.c asinf_vec.c
asinhVEC_FILES  = asinh_vec.c asinhf_vec.c
atanVEC_FILES   = atan_vec.c atanf_vec.c
atanhVEC_FILES  = atanh_vec.c atanhf_vec.c
atan2VEC_FILES  = atan2_vec.c atan2f_vec.c
cosVEC_FILES    = cos_vec.c cosf_vec.c
coshVEC_FILES   = cosh_vec.c coshf_vec.c
erfVEC_FILES    = erf_vec.c erfc_vec.c erfcf_vec.c erff_vec.c
expVEC_FILES    = exp_vec.c expf_vec.c
miscVEC_FILES   = ceil_vec.c ceilf_vec.c fabs_vec.c fabsf_vec.c \
                  floor_vec.c floorf_vec.c
fmodVEC_FILES   = fmod_vec.c fmodf_vec.c
frexpVEC_FILES  = frexp_vec.c frexpf_vec.c
gammaVEC_FILES  = gamma_vec.c gammaf_vec.c
hypotVEC_FILES  = hypot_vec.c hypotf_vec.c
jy01VEC_FILES   = j0_vec.c j0f_vec.c j1_vec.c j1f_vec.c y0_vec.c y0f_vec.c \
                  y1_vec.c y1f_vec.c
jynVEC_FILES    = jn_vec.c jnf_vec.c yn_vec.c ynf_vec.c
ldexpVEC_FILES  = ldexp_vec.c ldexpf_vec.c
log10VEC_FILES  = log10_vec.c log10f_vec.c
log1pVEC_FILES  = log1p_vec.c log1pf_vec.c
log2VEC_FILES   = log2_vec.c log2f_vec.c
logVEC_FILES    = log_vec.c logf_vec.c
modfVEC_FILES   = modf_vec.c modff_vec.c
powVEC_FILES    = pow_vec.c powf_vec.c
sinVEC_FILES    = sin_vec.c sinf_vec.c
sinhVEC_FILES   = sinh_vec.c sinhf_vec.c
sqrtVEC_FILES   = sqrt_vec.c sqrtf_vec.c
tanVEC_FILES    = tan_vec.c tanf_vec.c
tanhVEC_FILES   = tanh_vec.c tanhf_vec.c

GEN_VEC_NAMES   = acos  acosh asin  \
                  asinh atan  atanh \
                  atan2 cos   cosh  \
                  erf   exp   misc  \
                  fmod  frexp gamma \
                  hypot jy01  jyn   \
                  ldexp log10 log1p \
                  log2  log   modf  \
                  pow   sin   sinh  \
                  sqrt  tan   tanh

GEN_PROGS = $(addsuffix vec.exe,$(addprefix tgen/g, $(GEN_VEC_NAMES)))

GEN_VEC_FILES := $(foreach name,$(GEN_VEC_NAMES),$($(name)VEC_FILES))

VEC_OFILES = $(GEN_VEC_FILES:.c=.o)

$(OFILES): CFLAGS = $(DEFS) -fno-builtin -O2 -g -Wall

all: check

check: mtest.exe t-trunc.exe t-truncf.exe t-truncl.exe
	./mtest.exe > mtest.results
	./t-trunc.exe
	./t-truncf.exe
	./t-truncl.exe

# Pattern rules to generate test vectors.  (The funky vec.c=%.c replacement
# is meant to create a pattern rule where actually a normal rule will
# do, since only pattern rules can tell Make that several targets are
# generated all at once.  Without this, Make will invoke the vector-
# generating program several times.)
$(miscVEC_FILES:vec.c=%.c): tgen/gmisc%.exe
	$<

$(jy01VEC_FILES:vec.c=%.c): tgen/gjy01%.exe
	$<

$(jynVEC_FILES:vec.c=%.c): tgen/gjyn%.exe
	$<

$(erfVEC_FILES:vec.c=%.c): tgen/gerf%.exe
	$<

%_vec.c %f_vec.c: tgen/g%vec.exe
	$<

tgen/g%vec.exe: tgen/g%vec.cpp tgen/genmathv.h
	$(MAKE) -C $(@D) $(@F) TOP='$(TOP)' \
	 CFLAGS='$(CFLAGS)' CPPFLAGS='$(CPPFLAGS) -I./qfloat -I..' \
	 QFLOAT=$(HERE)/tgen/qfloat TEST=$(HERE) MATH=$(TOP)/../lib

mtest.exe: $(OFILES)  $(VEC_OFILES)
	$(CC) -o $@ $(LDFLAGS) $(OFILES) $(VEC_OFILES) $(LIBS)

t-trunc.exe: t-trunc.o
	$(CC) -o $@ $(LDFLAGS) t-trunc.o $(LIBS)

t-truncf.exe: t-truncf.o
	$(CC) -o $@ $(LDFLAGS) t-truncf.o $(LIBS)

t-truncl.exe: t-truncl.o
	$(CC) -o $@ $(LDFLAGS) t-truncl.o $(LIBS)

$(OFILES) $(VEC_OFILES) : test.h

clean mostlyclean:
	-cd tgen; $(MAKE) $@
	cd $(HERE); $(RM) $(OFILES) $(VEC_OFILES) *~ *.exe mtest.results t-trunc*.o

.SECONDARY: $(GEN_PROGS) $(GEN_VEC_FILES)
.PHONY: all check clean mostlyclean
