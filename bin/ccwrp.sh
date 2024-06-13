#!/usr/bin/env bash

set -o pipefail

[ -n "$CC" ] || CC=cc
[ -n "$CPP" ] || CPP=cpp

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
case "${LAST_ARG}" in
  *.S)
       echo "$CC -m32 -x assembler-with-cpp $* ${LAST_ARG}"
       $CC -m32 -x assembler-with-cpp $* ${LAST_ARG}
       exit $?
       ;;
  *.s)
       echo "$CC -m32 -x assembler $* ${LAST_ARG}"
       $CC -m32 -x assembler $* ${LAST_ARG}
       exit $?
       ;;
esac
for arg in "$@"
do
    case "$arg" in
    -I*) INCS="$INCS $arg"
         shift
         ;;
    esac
done
if [ -n "$*" ]; then
  $CPP -CC -g0 `pkg-config --variable=cppflags dj64` \
    ${INCS} ${LAST_ARG} | \
    sed -E -e 's/^\*\/#/\*\/\n#/' -e 's/^\*\/ *;/\*\/\n;\n/' | \
    $CC -xc `pkg-config --cflags dj64` $* -
else
  $CC ${LAST_ARG}
fi
