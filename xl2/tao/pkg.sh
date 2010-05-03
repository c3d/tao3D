#!/bin/bash
# Usage: pkg.sh [debug | release]

DIR=debug
if test "$1" ; then
  DIR="$1"
fi
#DIR=release
doo() {
  echo $*
  eval $*
}

(
cd $DIR
rm -rf pkg
mkdir pkg
cp tao.exe pkg/
cp ../builtins.xl pkg/
cp ../xl.syntax pkg/
for f in `ldd tao.exe | grep -v 'WINDOWS\\\\system32' | grep -v 'ntdll.dll' | sed 's/^.*=> \\(.*\\)(0x.*)$/\\1/' | sed 's@/cygdrive@@'` ; 
do 
    doo cp $f pkg/
done
)
