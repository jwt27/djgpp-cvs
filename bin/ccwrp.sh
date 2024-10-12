#!/usr/bin/env bash

set -o pipefail

[ -n "$CC" ] || CC=cc
[ -n "$CPP" ] || CPP=cpp
ASCPPFLAGS=`pkg-config --variable=cppflags dj64`
XCPPFLAGS=`pkg-config --variable=xcppflags dj64`
[ -n "$DJ64AS" ] || DJ64AS="$CC -x assembler"
[ -n "$DJ64ASFLAGS" ] || DJ64ASFLAGS="-m32 -Wa,-defsym,_DJ64=1 -c"

ORIG_ARGS="$*"
if [ $# -lt 2 ]; then
  $CC ${ORIG_ARGS}
  exit $?
fi
LAST_ARG=${!#}
set -- "${@:1:$(($#-1))}"
PLAST_ARG=${!#}
if [ "$PLAST_ARG" != "-c" ]; then
  $CC ${ORIG_ARGS}
  exit $?
fi
for arg in "$@"
do
    case "$arg" in
    -I*) INCS="$INCS $arg"
         shift
         ;;
    esac
done
case "${LAST_ARG}" in
  *.S|-)
        set -- "${@:1:$(($#-1))}"
        echo "$CPP -x assembler-with-cpp $ASCPPFLAGS ${INCS} ${LAST_ARG} | " \
             "$DJ64AS $DJ64ASFLAGS - $*"
        $CPP -x assembler-with-cpp $ASCPPFLAGS ${INCS} ${LAST_ARG} | \
            $DJ64AS $DJ64ASFLAGS - $*
        exit $?
        ;;
  *.s)
        set -- "${@:1:$(($#-1))}"
        echo "$DJ64AS $DJ64ASFLAGS $* ${LAST_ARG}"
        $DJ64AS $DJ64ASFLAGS $* ${LAST_ARG}
        exit $?
        ;;
esac
if [ -n "$*" ]; then
  $CPP -CC -g0 $XCPPFLAGS $CPPFLAGS ${INCS} ${LAST_ARG} | \
    sed -E -e 's/^\*\/#/\*\/\n#/' -e 's/^\*\/ *;/\*\/\n;\n/' | \
    eval $CC -xc `pkg-config --cflags dj64` $CFLAGS ${*//\"/\\\"} -
else
  $CC ${LAST_ARG}
fi
