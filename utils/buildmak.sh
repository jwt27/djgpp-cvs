#!/bin/ksh

TOP=${PWD#*v2/src/*/}
TOP=${TOP#*v2/tests/*/}
TOP=`echo $TOP | sed "s?[^/][^/]*?..?g"`

echo TOP=$TOP > makefile
echo >> makefile

# SRC="`make -f $TOP/../../utils/buildmake.mak`"
for src in *.[cs]*
do
  case $src in
   \**)	;;
   *.s)		echo SRC += ${src%s}S ;;
   *.c|*.cc)	echo SRC += $src ;;
  esac
done >> makefile

echo >> makefile
echo include '$(TOP)/../makefile.inc' >> makefile

utod makefile
