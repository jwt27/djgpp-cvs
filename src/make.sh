#!/bin/ksh

HERE=$PWD

if [ "$1" = "-q" ]
then
  shift
else
  case " $* " in
    *\ -k\ *) kflag=y ;;
    *) kflag= ;;
  esac
  error=
  for mf in `find . -name makefile -print | sed -n -e 's@./@@' -e '/\// p' | sort`
  do
    echo --------------------------------------- Making $* in ${mf%/*}
    cd $HERE/${mf%/*}
    if [ "$kflag" ]
    then
      make $* || error=y
    else
      make $* || exit 1
    fi
  done
  test "$error" && exit 1
fi

cd $HERE
echo --------------------------------------- Making \"$*\" for library

echo > makefile.oi
echo > makefile.rf2
for oh in `find . -name makefile.oh -print | sed 's/^.\///' | sort`
do
  prefix=${oh%/*}
  sed -e "s/^/OBJS += /" -e "s?&/?$prefix/?g" $oh >> makefile.oi
  sed -e "s?&/?$prefix/?g" $oh >> makefile.rf2
done

update makefile.rf2 makefile.rf
rm makefile.rf2

make.exe $*
