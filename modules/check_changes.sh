#!/bin/bash
#

if [ "$1" = "" ] ; then
  echo "Usage: $0 <SINCE>"
  echo "   SINCE : Tag or commit id"
  echo
  echo "For each module, show what has changesd since 'SINCE': (change log and .xl diff)"
  echo "Convenient to detect if the version of a module has to be incremented."
  exit 1
fi

SINCE="$1" 
ALL_XL=$(find . -name '*.xl')
MODS=""
for i in $ALL_XL ; do
  BN=$(basename $i)
  DN=$(dirname $i)
  NOEXT=$(echo $BN | sed 's/\.xl$//')
  echo $i | grep -q ${NOEXT}/${NOEXT}.xl && MODS="$MODS $DN"
done

for i in $MODS ; do
  XL=$(basename $i).xl
  [ -d "$i" ] && (echo ; echo "/// $i ///" ; echo ; git lol $SINCE..HEAD $i ; git diff $SINCE HEAD $i/$XL)
done
