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

THISDIR=`dirname $0`

#
# Copy application and dependencies into buildroot/
#

OBJDIR=../../$MODE
SRCDIR=../..
(
cd $THISDIR
export PATH=$PATH:.
rm -rf buildroot
mkdir buildroot
doo cp $OBJDIR/Tao.exe buildroot/
SFILES="builtins.xl xl.syntax xl.stylesheet git.stylesheet"
for f in $SFILES ; do
    doo cp $SRCDIR/$f buildroot/
done
for f in `bin/ldd buildroot/Tao.exe | grep -v -i 'windows/system' | grep -v -i 'ntdll.dll' | grep -v -i 'comctl' | sed 's/^.*=> \\(.*\\)(0x.*)$/\\1/' | sed 's@/cygdrive@@'` ; 
do 
    doo cp $f buildroot/
done
)
