#!/bin/ksh

cd ${DJDIR}/include

if [ x"$1" = x"" ]
then
  echo 'Usage: whatin [-f|-a|-p|-d] <file>.h [cpp-options]'
  echo ; ls -CF
  echo ; ls -CF sys
  exit 0
fi

with=
without=

case $1 in
  -f)
    with=-D__dj_ENFORCE_ANSI_FREESTANDING
    shift
    ;;
  -a)
    without=-D__dj_ENFORCE_ANSI_FREESTANDING
    with=-D__dj_ENFORCE_ANSI_HOSTED
    shift
    ;;
  -p)
    without=-D__dj_ENFORCE_ANSI_HOSTED
    with=-D_POSIX_SOURCE
    shift
    ;;
  -d)
    without=-D_POSIX_SOURCE
    with=
    shift
    ;;
esac

file=$1
shift
flags="-P -lang-c -I- -I. -D__dj_ENFORCE_FUNCTION_CALLS $*"

do_one()
{
  cpp $with $flags $1 | lessblank > with.wht
  if [ "$without" ]
  then
    cpp $without $flags $1 | lessblank > without.wht
  else
    echo > without.wht
  fi
  diff without.wht with.wht | sed -e '/^[0-9]/ d' -e 's/^. //' > just.wht
  if [ -s just.wht ]
  then
    echo -------------------- $1 --------------------
    tabspc < just.wht | sed -f d:/posix/utils/whatin.sed
  fi
  rm without.wht with.wht just.wht
}

if [ "$file" = "-s" ]
then
  for i in *.h sys/*.h
  do
    do_one $i
  done
else
  do_one $file
fi
