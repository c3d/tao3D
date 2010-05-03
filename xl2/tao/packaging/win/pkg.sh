#!/bin/bash
# Usage: pkg.sh [debug | release]

MODE=debug
if test "$1" ; then
  MODE="$1"
fi
doo() {
  echo $*
  eval $*
}

(
OBJDIR=../../$MODE
SRCDIR=../..
rm -rf buildroot
mkdir buildroot
doo cp $OBJDIR/tao.exe buildroot/
doo cp $SRCDIR/builtins.xl buildroot/
doo cp $SRCDIR/xl.syntax buildroot/
for f in `ldd $OBJDIR/tao.exe | grep -v -i 'windows/system' | grep -v -i 'ntdll.dll' | grep -v -i 'comctl' | sed 's/^.*=> \\(.*\\)(0x.*)$/\\1/' | sed 's@/cygdrive@@'` ; 
do 
    doo cp $f buildroot/
done
)
