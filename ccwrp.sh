#!/usr/bin/env bash

ORIG_ARGS="$*"
if [ $# -lt 2 ]; then
  cc ${ORIG_ARGS}
  exit $?
fi
LAST_ARG=${!#}
set -- "${@:1:$(($#-1))}"
PLAST_ARG=${!#}
if [ "$PLAST_ARG" != "-c" ]; then
  cc ${ORIG_ARGS}
  exit $?
fi
case "${LAST_ARG}" in
  *.S)
       echo "cc -m32 -x assembler-with-cpp $* ${LAST_ARG}"
       cc -m32 -x assembler-with-cpp $* ${LAST_ARG}
       exit $?
       ;;
  *.s)
       echo "cc -m32 -x assembler $* ${LAST_ARG}"
       cc -m32 -x assembler $* ${LAST_ARG}
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
  echo "cc -E -CC -g0 `pkg-config --cflags dj64` ${INCS} ${LAST_ARG} | cc -xc - $*"
  cc -E -CC -g0 `pkg-config --cflags dj64` ${INCS} ${LAST_ARG} | cc -xc - $*
else
  cc ${LAST_ARG}
fi
