#!/bin/ksh

find . -name "*.[cs]" -print \
| while read fn
do
  if [ ! -f ${fn%.*}.txh ]
  then
    echo $fn
  fi
done
